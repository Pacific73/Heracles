#ifndef CPU_DRIVER
#define CPU_DRIVER

#include "tap.h"
#include "cache_driver.h"
#include <pthread.h>

class CpuDriver {
  private:
    std::string path;
    size_t max_BE_cores;

    std::string mem_data;

    Tap *tap;
    CacheDriver *cc_d;

    size_t BE_cores;
    size_t total_cores;
    size_t sys_cores;

    pid_t BE_pid;
    pid_t LC_pid;

    pthread_mutex_t mutex;

    bool init_config();

    bool init_cgroups_dir();

    bool init_core_num();

    bool init_mem_data();

    bool set_cores_for_pid(size_t type, size_t left, size_t right);

    bool update(bool inc = true);

  public:
    CpuDriver(Tap *t, CacheDriver* cd);

    size_t total_core_num() const;

    size_t BE_core_num() const;

    size_t sys_core_num() const;

    bool set_new_BE_task(pid_t pid);

    bool BE_cores_inc(size_t inc); // bound check

    bool BE_cores_dec(size_t dec); // bound check

    void clear();
};

#endif