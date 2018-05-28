#include "tap.h"
#include "cpu_driver.h"
#include "core_memory_controller.h"
#include "top_controller.h"
#include <cassert>
#include <csignal>
#include <sys/wait.h>
#include <sys/types.h>

Tap::Tap(pid_t lc_pid) : _state(TAPSTATE::DISABLED) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        print_err("[TAP] mutex init failed.");
    }

    db_d = new DatabaseDriver();

    _BE_pid = -1;
    _LC_pid = (lc_pid == -1) ? get_opt<pid_t>("HERACLES_LC_PID", -1) : lc_pid;

    if (_LC_pid <= 0) {
        print_err("[TAP] can't recognize LC task.");
        exit(-1);
    }

    print_log("[TAP] inited.");
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
    if(_state == TAPSTATE::DISABLED) {
        kill_BE();
    }
    pthread_mutex_unlock(&mutex);
}

void Tap::cool_down_little() {
    assert(cpu_d != nullptr);
    cpu_d->BE_cores_dec(1);
}

pid_t Tap::BE_pid() {
    pthread_mutex_lock(&mutex);
    pid_t ret = _BE_pid;
    pthread_mutex_unlock(&mutex);
    return ret;
}

pid_t Tap::LC_pid() {
    pthread_mutex_lock(&mutex);
    pid_t ret = _LC_pid;
    pthread_mutex_unlock(&mutex);
    return ret;
}

TAPSTATE Tap::state() {
    pthread_mutex_lock(&mutex);
    TAPSTATE s = _state;
    pthread_mutex_unlock(&mutex);
    return s;
}

void Tap::BE_end(int *status) {
    // clean other drivers' status
    _BE_pid = -1;
    cm_c->clear();
    if(WIFEXITED(status)) {
        db_d->task_finish(true);
    } else {
        db_d->task_finish(false);
    }
    
}

bool Tap::kill_BE() {
    if(_BE_pid != -1) {
        int ret = kill(_BE_pid, SIGKILL);
        if(ret) {
            print_err("[TAP] BE task kill failed.");
            return false;
        }
    }
    return true;
}

int Tap::run() {

    while (true) {
        usleep(1000000);

        if (_BE_pid == -1 && _state == TAPSTATE::ENABLED) {
            Task task = db_d->next_task();
            if (!task.complete) {
                print_log("[TAP] no more ready BE tasks! Heracles will exit.");
                exit(-1);
                break;
                // there's no more "ready" BE tasks and system shall exit.
            }
            print_log("[TAP] new BE task: %s", task.program);

            pid_t pid = fork();
            // else fork new process and exec a new BE task

            if (pid == -1) {
                print_err("[TAP] fork error.");
                continue;
            } else if (pid > 0) {
                _BE_pid = pid;
                int status;
                if(!cm_c->set_new_BE_task(pid)) {
                    print_err("[TAP] init BE CORE/MEMORY/CACHE failed.");
                }
                print_log("[TAP] waiting for BE(pid=%d) to be finished...", pid);
                waitpid(pid, &status, 0);
                print_log("[TAP] pid %d finished.", pid);
                BE_end(&status);
                // parent process: heracles
            } else if (pid == 0) {
                print_log("[BE] new task executing...");
                execvp(task.program, task.argv.data());
                print_err("[BE] you have input the wrong command!");
                exit(-1);
                // child process: exec shell command
            }
        }
    }
    return -1;
}
