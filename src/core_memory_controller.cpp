#include "core_memory_controller.h"
#include "helpers.h"


CoreMemoryController::CoreMemoryController(Tap *t, InfoPuller *i, pid_t lc): tap(t), puller(i) {
    state = STATE::GROW_LLC;

    init_cpu_driver(lc);
    init_memory_driver();
    init_cache_driver();
}

void CoreMemoryController::load_config() {

    dram_limit = get_opt("HERACLES_DRAM_LIMIT", 1024);
    sleep_time = get_opt("CORE_MEMORY_SLEEP_TIME", 2);
}

void CoreMemoryController::init_cpu_driver(pid_t lc) {
    std::string path = get_opt("CGROUPS_DIR", "/sys/fs/cgroups");
    cpu_d = new CpuDriver(path, lc);
    //...
}

void CoreMemoryController::init_memory_driver() {
    mm_d = new MemoryDriver();
}

void CoreMemoryController::init_cache_driver() {
    cc_d = new CacheDriver();
}

int CoreMemoryController::run() {
    load_config();

    struct timespec ts;
    ts.tv_sec = sleep_time;
    ts.tv_nsec = 0;

    while(true) {
        nanosleep(&ts, nullptr);
        double total_bw = mm_d->measure_dram_bw();
        if(total_bw > dram_limit) {
            double overage = total_bw - dram_limit;
            cpu_d->BE_cores_dec(overage / mm_d->BE_bw_per_core());
            continue;

        }
        if(!tap->is_enabled()) {
            continue;
        }
        if(state == STATE::GROW_LLC) {
            if(mm_d->predicted_total_bw() > dram_limit) {
                state = STATE::GROW_CORES;
            } else {
                cc_d->BE_cache_grow();
                // nanosleep? to wait for CAT take effect
                double bw_derivative = mm_d->measure_dram_bw() - mm_d->total_bw();
                if(bw_derivative >= 0) {
                    cc_d->BE_cache_roll_back();
                    state = STATE::GROW_CORES;
                }
                // if(!benefit()) {                    // benefit() ???
                //     state = STATE::GROW_CORES;
                // }
            }
        } else if(state == STATE::GROW_CORES) {
            double needed = mm_d->LC_bw() + mm_d->BE_bw() + mm_d->BE_bw_per_core();
            double slack = puller->pull_latency_info().slack_95();
            if(needed > dram_limit) {
                state = STATE::GROW_LLC;
            } else if(tap->slack > 0.10) { // !!!!->slack!!!! 0.10!!!!
                cpu_d->BE_cores_inc(1);
            }
        }
    }
}