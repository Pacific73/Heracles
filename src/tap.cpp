#include "tap.h"
#include <csignal>

Tap::Tap() : BE_enabled(false), BE_growth_allowed(false) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        print_err("[TAP] Tap init failed.");
    }

    init_database();
    
    LC_pid = get_opt("HERACLES_LC_PID", -1);
    if(LC_pid == -1) {
        print_err("[TAP] can't recognize LC task.");
        exit(-1);
    }
}

void Tap::init_database_driver() {
    db = new DatabaseDriver("path");
    //...
}

void Tap::BE_end() {
    //wait()...
}

void Tap::run_new_BE() {
    std::string command = ""; // get next BE command...
    pid_t pid = fork();
    if(pid > 0) {
        BE_pid = pid;
        signal(SIGCHLD, BE_end());
    } else {
        //exec...

    }

}

void Tap::set_BE_enabled(bool e) {
    pthread_mutex_lock(&mutex);
    BE_enabled = e;
    pthread_mutex_unlock(&mutex);
}

void Tap::set_BE_growth_enabled(bool e) {
    pthread_mutex_lock(&mutex);
    BE_growth_allowed = e;
    pthread_mutex_unlock(&mutex);
}