#ifndef MEMORY_DRIVER_H
#define MEMORY_DRIVER_H

#include "cpu_driver.h"
#include "tap.h"
#include "pqos.h"
#include <string>

struct Setting {
    unsigned core;
    struct pqos_mon_data pgrp;
    enum pqos_mon_event events;
};

class MemoryDriver {
  private:
    std::string path;
    Tap *tap;
    CpuDriver *cpu_d;

    bool is_numa;

    double measure_bw(size_t lower, size_t upper);

  public:
    MemoryDriver(Tap *t, CpuDriver *cd);
    double measure_dram_bw();
    double predicted_total_bw();
    double LC_bw();
    double BE_bw();
    double BE_bw_per_core();

    void clear();
};

#endif