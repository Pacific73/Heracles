#ifndef TAP_H
#define TAP_H

#include "db_driver.h"
#include "helpers.h"
#include <pthread.h>
#include <string>
#include <unistd.h>

class CpuDriver;

class TopController;

class CoreMemoryController;

enum TAPSTATE { DISABLED, PAUSED, ENABLED };

class Tap {
  private:
    TAPSTATE _state;

    CpuDriver *cpu_d;
    DatabaseDriver *db_d;

    TopController *t_c;
    CoreMemoryController *cm_c;

    size_t total_cores;
    size_t BE_cores;

    pid_t _LC_pid;
    pid_t _BE_pid;

    pthread_mutex_t mutex;

    void BE_end();

  public:
    Tap(pid_t lc_pid);

    void set_t_c(TopController *tc);

    void set_cm_c(CoreMemoryController *cmc);

    void set_cpu_d(CpuDriver *c);

    void set_state(TAPSTATE t);

    pid_t BE_pid() const { return _BE_pid; }

    pid_t LC_pid() const { return _LC_pid; }

    TAPSTATE state() const { return _state; }

    void cool_down_little();

    int run();
};

#endif