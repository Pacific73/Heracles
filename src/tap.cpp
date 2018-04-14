#include "tap.h"
#include "cpu_driver.h"
#include <csignal>
#include <sys/wait.h>
#include <cassert>

Tap::Tap() : _state(TAPSTATE::DISABLED) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        print_err("[TAP] mutex init failed.");
    }

    init_database_driver();

    _LC_pid = get_opt("HERACLES_LC_PID", -1);
    if (_LC_pid == -1) {
        print_err("[TAP] can't recognize LC task.");
        exit(-1);
    }

    _BE_pid = -1;
}

void Tap::init_database_driver() {
    db_d = new DatabaseDriver("path");
    //...
}

std::string Tap::get_next_command() { return ""; }

void Tap::BE_end() {
    // clean other drivers' status
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
            std::string command = get_next_command();
            if (command == "") {
                break;
                // no new tasks
            }
            pid_t pid = fork();
            if (pid == -1) {
                print_err("[TAP] fork error.");
                continue;
            } else if (pid > 0) {
                _BE_pid = pid;
                int status;
                waitpid(pid, &status, 0);
                BE_end();
                _BE_pid = -1;
            } else if (pid == 0) {
                // exec command....
            }
        } else {
            print_err("[TAP] BE_pid == 1 && wait failed.");
            break;
        }
    }
}