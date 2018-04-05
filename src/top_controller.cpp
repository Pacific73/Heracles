#include "top_controller.h"
#include "helpers.h"
#include <fstream>
#include <iostream>

TopController::TopController(Tap *t, InfoPuller *i) : tap(t), puller(i) {}

void TopController::load_config() {
    sleep_time = get_opt("TOP_SLEEP_TIME", 5);
    disable_bound = get_opt("TOP_DISABLE_BOUND", 0.85);
    enable_bound = get_opt("TOP_ENABLE_BOUND", 0.8);
    slow_be_bound = get_opt("TOP_SLOW_BE_BOUND", 0.1);
}

int TopController::run() {

    load_config();

    struct timespec ts;
    ts.tv_sec = sleep_time;
    ts.tv_nsec = 0;

    while (true) {
        nanosleep(&ts, nullptr);
        if (!update()) {
            return -1;
        }
        double slack = puller->pull_latency_info().slack_95();
        double load_percent = puller->pull_load_info().load_percent();

        if (slack < 0) {
            tap->set_BE_enabled(false);
            // enter_cooling_down()...
        } else if (load_percent > disable_bound) {
            tap->set_BE_enabled(false);
        } else if (load_percent < enable_bound) {
            tap->set_BE_eabled(true);
        } else if (slack < slow_BE_bound) {
            tap->set_BE_growth_enabled(false);
            if (slack < slow_BE_bound / 2) {
                tap->BE_cores_dec(2);
            }
        }
    }
    return 0;
}