#include "memory_driver.h"

MemoryDriver::MemoryDriver(std::string p) : path(p) {}

double MemoryDriver::measure_dram_bw() {}
double MemoryDriver::predicted_total_bw() {}
double MemoryDriver::LC_bw() {}
double MemoryDriver::BE_bw() {}
double MemoryDriver::BE_bw_per_core() {}