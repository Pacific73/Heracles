#ifndef TAP_H
#define TAP_H

#include "helpers.h"
#include "db_driver.h"
#include <pthread.h>
#include <unistd.h>

class Tap {
  private:
    bool BE_enabled;
    bool BE_growth_allowed;

    size_t total_cores;
    size_t BE_cores;

    size_t total_cache; // temporarily written as this form
    size_t BE_cache;

    pid_t LC_pid;
    pid_t BE_pid;

    DatabaseDriver db;

    pthread_mutex_t mutex;

    void get_total_cache();
    void get_total_cores();
    void init_database();

    void run_new_BE();
    void BE_end();

    bool update_cores();
    bool update_cache();

  public:
    Tap();

    bool is_BE_enabled() const { return BE_enabled; }

    bool is_BE_growth_allowed() const { return BE_growth_allowed; }

    void set_BE_enabled(bool e);
    
    void set_BE_growth_enabled(bool e);

    // bound check

    void BE_cores_inc(int inc);

    void BE_cores_dec(int dec);

    void BE_cache_grow();

    void BE_cache_roll_back();
    
    // bound check

};

#endif