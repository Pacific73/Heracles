#include "core_memory_controller.h"
#include "helpers.h"

CoreMemoryController::CoreMemoryController(Tap *t, InfoPuller *i)
    : tap(t), puller(i) {
    t->set_cm_c(this);

    state = STATE::GROW_LLC;

    init_config();

    init_cache_driver();

    init_cpu_driver();
    init_memory_driver();
}

void CoreMemoryController::init_config() {

    dram_limit = get_opt<double>("HERACLES_DRAM_LIMIT", 10240);
    sleep_time = get_opt<time_t>("CORE_MEMORY_SLEEP_TIME", 2);
}

void CoreMemoryController::init_cpu_driver() {
    cpu_d = new CpuDriver(tap, cc_d);
    tap->set_cpu_d(cpu_d);
    //...
}

void CoreMemoryController::init_memory_driver() {
    mm_d = new MemoryDriver(tap, cpu_d);
}

void CoreMemoryController::init_cache_driver() { cc_d = new CacheDriver(); }

int CoreMemoryController::run() {

    struct timespec ts;
    ts.tv_sec = sleep_time;
    ts.tv_nsec = 0;

    while (true) {
        nanosleep(&ts, nullptr);

        if (tap->BE_pid() == -1 || tap->state() == TAPSTATE::DISABLED) {
            print_log("[CMC] tap disabled detected. clearing...");
            clear();
            continue;
        }
        // if no BE task is running (or BE is disabled), give all capabilities
        // to LC

        if (tap->state() == TAPSTATE::PAUSED) {
            print_log("[CMC] tap paused detected.");
            continue;
        }
        // if BE growth is not allowed, keep current settings

        double total_bw = mm_d->measure_dram_bw();
        print_log("[CMC] total_bw = %6.1lf", total_bw);
        if (total_bw > dram_limit) {
            double overage = total_bw - dram_limit;
            size_t minus = (size_t)(overage / mm_d->BE_bw_per_core());
            cpu_d->BE_cores_dec(minus);
            print_log(
                "[CMC] BW(%6.1lf) greater than dram_limit(%lf). dec %u cores.",
                total_bw, dram_limit, minus);
            continue;
        } // if memory bw is overused, cut the extra tasks

        if (state == STATE::GROW_LLC) {
            print_log("[CMC] GROW_LLC.");

            double old_bw = mm_d->measure_dram_bw();
            cc_d->BE_cache_grow();
            // nanosleep? to wait for CAT take effect

            double new_bw = mm_d->measure_dram_bw();
            if (new_bw > dram_limit) {
                state = STATE::GROW_CORES;
                continue;
            }

            double derivative = new_bw - old_bw;
            if (derivative >= 0) {
                print_log("[CMC] cache_grow invalid. roll_back().");
                cc_d->BE_cache_roll_back();
                state = STATE::GROW_CORES;
            }
            // if(!benefit()) {                    // benefit() ???
            //     state = STATE::GROW_CORES;
            // }

        } else if (state == STATE::GROW_CORES) {
            print_log("[CMC] GROW_CORES.");
            double needed = mm_d->LC_bw() + mm_d->BE_bw() +
                            mm_d->BE_bw_per_core(); // one more BE core
            double slack = puller->pull_latency_info().slack();
            if (needed > dram_limit) {
                state = STATE::GROW_LLC;
            } else if (slack > 0.10) { // !!!!->slack!!!! 0.10!!!!
                if(cpu_d->BE_cores_inc(1) == false) {
                    print_err("[CMC] BE_cores_inc failed.");
                }
            }
        }
    }
}

void CoreMemoryController::clear() {
    if (tap->BE_pid() != -1) {
        // kill job!
    }
    cpu_d->clear();
    mm_d->clear();
    cc_d->clear();
}

bool CoreMemoryController::set_new_BE_task(pid_t pid) {
    return cpu_d->set_new_BE_task(pid);
}
