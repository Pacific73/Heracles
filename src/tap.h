#ifndef TAP_H
#define TAP_H

#include "db_driver.h"
#include "helpers.h"
#include <pthread.h>
#include <string>
#include <unistd.h>

class CpuDriver;

enum TAPSTATE {
    DISABLED,
    PAUSED,
    ENABLED
};

class Tap {
  private:
    TAPSTATE _state;

    CpuDriver *cpu_d;

    size_t total_cores;
    size_t BE_cores;

    pid_t _LC_pid;
    pid_t _BE_pid;

    DatabaseDriver *db_d;

    pthread_mutex_t mutex;

    void init_database_driver();

    void BE_end();

    std::string get_next_command();

  public:
    Tap();

    void set_cpu_d(CpuDriver *c) { cpu_d = c; }

    pid_t BE_pid() const { return _BE_pid; }

    pid_t LC_pid() const { return _LC_pid; }

    TAPSTATE state() const { return _state; }

    void set_state(TAPSTATE t);

    void cool_down_little();

    int run();
};

#endif