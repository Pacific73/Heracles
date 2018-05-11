#ifndef INFO_PULLER_H
#define INFO_PULLER_H

#include <iostream>

class LatencyInfo {
  private:
    double cur_lat;
    double slo_lat;

  public:
    friend class InfoPuller;

    LatencyInfo() : cur_lat(0), slo_lat(0) {}

    double slack();
};

class LoadInfo {
  private:
    size_t cur_load;
    size_t max_load;

  public:
    friend class InfoPuller;

    LoadInfo() : cur_load(0), max_load(0) {}

    double load_percent();
};

class InfoPuller {
  private:
    std::string latency_path;
    std::string max_latency_path;
    std::string max_load_path;
    std::string load_path;

    void init_config();

  public:
    InfoPuller();
    LatencyInfo pull_latency_info();
    LoadInfo pull_load_info();
};

#endif