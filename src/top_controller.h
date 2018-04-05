#ifndef TOP_CONTROLLER_H
#define TOP_CONTROLLER_H

#include "tap.h"
#include "info_puller.h"
#include <string>
#include <ctime>

class TopController {
  private:

    time_t sleep_time;
    double disable_bound;
    double enable_bound;
    double slow_BE_bound;
    
    Tap *tap;
    InfoPuller* puller;

    void load_config();
    bool update();

  public:
    TopController(Tap *t, InfoPuller* i);

    int run();
};

#endif