#include "network_controller.h"
#include <cassert>

NetworkController::NetworkController(Tap *tap) : tap(tap) {

    assert(init_config() == true);

    assert(init_drivers() == true);

    available_bw = total_bw - total_bw / 10;

    print_log("[NET] total_bw: %llu", total_bw);
    print_log("[NET_CONTROLLER] inited.");
}

bool NetworkController::init_config() {
    total_bw = get_opt<uint64_t>("NET_TOTAL_BANDWIDTH", 1e9);
    sleep_time = get_opt<double>("NET_SLEEP_TIME", 2);
    return true;
}

bool NetworkController::init_drivers() {
    n_d = new NetworkDriver();
    n_m = new NetworkMonitor();
    return true;
}

void NetworkController::run() {

    while (true) {
        n_d->set_LC_procs(tap->LC_pid());
        n_d->set_BE_procs(tap->BE_pid());

        usleep(sleep_time * 1000000);

        uint64_t LC_bw = n_m->LC_bytes();
        uint64_t BE_bw = n_m->BE_bytes();
        print_log("[NET] LC_bw: %llu BE_bw: %llu", LC_bw, BE_bw);

        uint64_t new_bw = LC_bw < available_bw ? available_bw - LC_bw : 1;
        n_d->set_BE_bw(new_bw);

        print_log("[NET] set new BE_bw: %llu", new_bw);
    }
}