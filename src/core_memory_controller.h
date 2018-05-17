#ifndef CORE_MEMORY_CONTROLLER_H
#define CORE_MEMORY_CONTROLLER_H

#include "info_puller.h"
#include "tap.h"
#include "cpu_driver.h"
#include "memory_driver.h"
#include "cache_driver.h"

enum STATE {
    GROW_LLC,
    GROW_CORES
};

class CoreMemoryController {
  private:
    Tap *tap;
    InfoPuller *puller;

    CpuDriver *cpu_d;
    MemoryDriver *mm_d;
    CacheDriver *cc_d;

    time_t sleep_time;
    double dram_limit; // mean to be percentile,
                       // but need to set as numerical numbers.

    STATE state;

    pid_t LC_pid;

    bool init_config();
    bool init_drivers();

  public:
    CoreMemoryController(Tap *t, InfoPuller *i);

    int run();

    bool set_new_BE_task(pid_t pid);

    void clear();
};

#endif