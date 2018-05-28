#include "top_controller.h"
#include "helpers.h"
#include <fstream>
#include <iostream>

TopController::TopController(Tap *t, InfoPuller *i) : tap(t), puller(i) {
    t->set_t_c(this);

    init_config();
}

void TopController::init_config() {
    sleep_time = get_opt<time_t>("TOP_SLEEP_TIME", 5);
    disable_bound = get_opt<double>("TOP_LOAD_DISABLE_BOUND", 0.8);
    enable_bound = get_opt<double>("TOP_LOAD_ENABLE_BOUND", 0.6);
    slow_BE_bound = get_opt<double>("TOP_SLOW_BE_BOUND", 0.1);
}

int TopController::run() {

    int disable_checker = 0;

    while (true) {
        usleep(sleep_time * 1000000);

        double slack = puller->pull_latency_info().slack();
        double load_percent = puller->pull_load_info().load_percent();
        print_log("[TOP] slack: %4.1lf%% load_percent:%4.1lf%%", slack * 100,
                  load_percent * 100);

        if (slack < 0) {
            disable_checker++;
            if (disable_checker > 1) {
                tap->set_state(TAPSTATE::DISABLED);
                // enter_cooling_down()...
                print_log("[TOP] slack < 0. disable BE. cooling down...");
            }

        } else if (load_percent > disable_bound) {
            disable_checker++;
            if (disable_checker > 1) {
                tap->set_state(TAPSTATE::DISABLED);
                print_log("[TOP] load_percent > disable bound. disable BE.");
            }

        } else if (load_percent < enable_bound) {
            tap->set_state(TAPSTATE::ENABLED);
            disable_checker = 0;
            print_log("[TOP] load_percent < enable bound. enable BE.");

        } else if (slack < slow_BE_bound) {
            tap->set_state(TAPSTATE::PAUSED);
            disable_checker = 0;
            if (slack < slow_BE_bound / 2) {
                tap->cool_down_little();
            }
            print_log("[TOP] slack < slow_BE_bound. pause BE.");
        }
    }
    return 0;
}
