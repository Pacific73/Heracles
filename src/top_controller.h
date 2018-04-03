#ifndef TOP_CONTROLLER_H
#define TOP_CONTROLLER_H

#include "tap.h"
#include <string>
#include <ctime>

class TopController {
  private:
    std::string latency_path;
    std::string max_latency_path;
    std::string max_load_path;
    str::string load_path;

    time_t sleep_time;
    double disable_bound;
    double enable_bound;
    double slow_be_bound;
    
    Tap *tap;

    double l95;
    double l99;
    double lmax;
    double max_latency;

    int max_load;
    int load;

    void load_config();
    bool update();

  public:
    TopController(Tap *t);

    int run();
};

#endif