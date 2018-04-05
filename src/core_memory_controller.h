#ifndef CORE_MEMORY_CONTROLLER_H
#define CORE_MEMORY_CONTROLLER_H

#include "tap.h"
#include "info_puller.h"

enum STATE {
    GROW_LLC,
    GROW_CORES
}

class CoreMemoryController {
  private:
    Tap *tap;
    InfoPuller *puller;

    time_t sleep_time;

    double dram_limit; // mean to be percentile, 
                       // but need to set as numerical numbers.

    STATE state;

    void load_config();

    double measure_dram_bw();
    double predicted_total_bw();
    double LC_bw();
    double BE_bw();
    double BE_bw_per_core();


  public:
    CoreMemoryController(Tap *t, InfoPuller *i);

    int run();
};


#endif