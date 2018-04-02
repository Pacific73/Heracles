#ifndef TOP_CONTROLLER_H
#define TOP_CONTROLLER_H

#include "tap.h"
#include <string>

class TopController {
  private:
    std::string latency_path;
    std::string max_load_path;
    str::string load_path;
    Tap *tap;

    double l95;
    double l99;
    double lmax;
    int max_load;
    int load;

    bool update();

  public:
    TopController(Tap *t);

    bool is_config_ok();

    int run();
};

#endif