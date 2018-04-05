#include "core_memory_controller.h"
#include "helpers.h"


CoreMemoryController::CoreMemoryController(Tap *t, InfoPuller *i): tap(t), puller(i) {
    state = STATE::GROW_LLC;
}

void CoreMemoryController::load_config() {

    dram_limit = get_opt("HERACLES_DRAM_LIMIT", 1024);
    sleep_time = get_opt("CORE_MEMORY_SLEEP_TIME", 2);
}

double CoreMemoryController::measure_dram_bw() {

}

double CoreMemoryController::predicted_total_bw() {

}

double CoreMemoryController::LC_bw() {
    
}

double CoreMemoryController::BE_bw() {

}

double CoreMemoryController::BE_bw_per_core() {

}

bool CoreMemoryController::core_update() {

}

bool CoreMemoryController::cache_update() {

}

int CoreMemoryController::run() {
    load_config();

    struct timespec ts;
    ts.tv_sec = sleep_time;
    ts.tv_nsec = 0;

    while(true) {
        nanosleep(&ts, nullptr);
        double total_bw = measure_dram_bw();
        if(total_bw > dram_limit) {
            double overage = total_bw - dram_limit;
            tap->BE_cores_dec(overage / BE_bw_per_core());
            continue;

        }
        if(!tap->is_enabled()) {
            continue;
        }
        if(state == STATE::GROW_LLC) {
            if(predicted_total_bw() > dram_limit) {
                state = STATE::GROW_CORES;
            } else {
                tap->BE_cache_grow();
                // nanosleep? to wait for CAT take effect
                double bw_derivative = measure_dram_bw() - total_bw();
                if(bw_derivative >= 0) {
                    tap->BE_cache_roll_back();
                    state = STATE::GROW_CORES;
                }
                // if(!benefit()) {                    // benefit() ???
                //     state = STATE::GROW_CORES;
                // }
            }
        } else if(state == STATE::GROW_CORES) {
            double needed = LC_bw() + BE_bw() + BE_bw_per_core();
            double slack = puller->pull_latency_info().slack_95();
            if(needed > dram_limit) {
                state = STATE::GROW_LLC;
            } else if(tap->slack > 0.10) { // !!!!->slack!!!! 0.10!!!!
                tap->BE_cores_inc(1);
            }
        }
    }
}