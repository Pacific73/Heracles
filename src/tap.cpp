#include "tap.h"
#include "cpu_driver.h"
#include "core_memory_controller.h"
#include "top_controller.h"
#include <cassert>
#include <csignal>
#include <sys/wait.h>

Tap::Tap(pid_t lc_pid) : _state(TAPSTATE::DISABLED) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        print_err("[TAP] mutex init failed.");
    }

    db_d = new DatabaseDriver();

    _BE_pid = -1;
    _LC_pid = (lc_pid == -1) ? get_opt("HERACLES_LC_PID", -1) : lc_pid;

    if (_LC_pid <= 0) {
        print_err("[TAP] can't recognize LC task.");
        exit(-1);
    }
}

void Tap::BE_end() {
    // clean other drivers' status
    _BE_pid = -1;
    cm_c->clear();
    db_d->task_finish();
}

void Tap::set_t_c(TopController *tc) {
    pthread_mutex_lock(&mutex);
    t_c = tc;
    pthread_mutex_unlock(&mutex);
}

void Tap::set_cm_c(CoreMemoryController *cmc) {
    pthread_mutex_lock(&mutex);
    cm_c = cmc;
    pthread_mutex_unlock(&mutex);
}

void Tap::set_cpu_d(CpuDriver* cpud) {
    pthread_mutex_lock(&mutex);
    cpu_d = cpud;
    pthread_mutex_unlock(&mutex);
}

void Tap::set_state(TAPSTATE t) {
    pthread_mutex_lock(&mutex);
    _state = t;
    pthread_mutex_unlock(&mutex);
}

void Tap::cool_down_little() {
    assert(cpu_d != nullptr);
    cpu_d->BE_cores_dec(2);
}

int Tap::run() {
    while (true) {
        if (_BE_pid == -1) {
            std::string command = db_d->next_command();
            if (command == "") {
                std::cout << "[TAP] no more ready BE tasks! Heracles will exit." << std::endl;
                t_c->sys_exit();
                break;
                // there's no more "ready" BE tasks and system shall exit.
            }

            pid_t pid = fork();
            // else fork new process and exec a new BE task

            if (pid == -1) {
                print_err("[TAP] fork error.");
                continue;
            } else if (pid > 0) {
                _BE_pid = pid;
                int status;
                waitpid(pid, &status, 0);
                BE_end();
                // parent process: heracles
            } else if (pid == 0) {
                int ret = system(command.c_str());
                exit(ret);
                // child process: exec shell command
            }
        } else {
            print_err("[TAP] BE_pid != -1 before heracles assign a new task!");
            break;
        }
    }    
}
