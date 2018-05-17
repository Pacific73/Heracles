#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "network_driver.h"
#include "network_monitor.h"
#include "tap.h"

class NetworkController {
  private:
    Tap *tap;

    NetworkDriver *n_d;
    NetworkMonitor *n_m;

    uint64_t total_bw;
    uint64_t available_bw;
    double sleep_time;

    bool init_config();
    bool init_drivers();

  public:
    NetworkController(Tap *tap);

    void run();
};

#endif