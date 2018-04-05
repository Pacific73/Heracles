#ifndef INFO_PULLER_H
#define INFO_PULLER_H

class LatencyInfo {
  private:
    double cur_95;
    double cur_99;
    double cur_max;
    double slo_latency;
    bool complete;

  public:
    friend class InfoPuller;

    Latency() : cur_95(0), cur_99(0), cur_max(0), complete(false) {}

    double slack_95();
    double slack_99();
};

class LoadInfo {
  private:
    size_t cur_load;
    size_t max_load;
    bool complete;

  public:
    friend class InfoPuller;

    LoadInfo() : cur_load(0), max_load(0), complete(false) {}

    double load_percent();
};

class InfoPuller {
  private:
    std::string latency_path;
    std::string max_latency_path;
    std::string max_load_path;
    str::string load_path;

  public:
    InfoPuller();
    LatencyInfo pull_latency_info();
    LoadInfo pull_load_info();
};

#endif