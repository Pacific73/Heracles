#ifndef MEMORY_DRIVER_H
#define MEMORY_DRIVER_H

#include <string>

class MemoryDriver {
  private:
    std::string path;

  public:
    MemoryDriver();
    double measure_dram_bw();
    double predicted_total_bw();
    double LC_bw();
    double BE_bw();
    double BE_bw_per_core();

    void clear();
};

#endif