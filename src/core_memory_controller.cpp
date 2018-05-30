#include "core_memory_controller.h"
#include "helpers.h"
#include <cassert>

CoreMemoryController::CoreMemoryController(Tap *t, InfoPuller *i)
    : tap(t), puller(i) {

    t->set_cm_c(this);
    state = STATE::GROW_LLC;

    assert(init_config() == true);

    assert(init_drivers() == true);

    print_log("[CMC] inited.");
}

bool CoreMemoryController::init_config() {
    dram_limit = get_opt<double>("HERACLES_DRAM_LIMIT", 10240);
    sleep_time = get_opt<time_t>("CORE_MEMORY_SLEEP_TIME", 2);
    return true;
}

bool CoreMemoryController::init_drivers() {
    cc_d = new CacheDriver();

    cpu_d = new CpuDriver(tap, cc_d);
    tap->set_cpu_d(cpu_d);

    mm_d = new MemoryDriver(tap, cpu_d);
    // p.s. init order cannot be changed!

    return true;
}

int CoreMemoryController::run() {

    while (true) {
        usleep(sleep_time * 1000000);

        double total_bw = mm_d->measure_dram_bw();
        print_log("[CMC] total_bw = %6.1lf", total_bw);
        // measure total memory bandwidth

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

        if (total_bw > dram_limit) {
            double overage = total_bw - dram_limit;
            size_t minus = (size_t)(overage / mm_d->BE_bw_per_core());
            print_log(
                "[CMC] BW(%6.1lf) greater than dram_limit(%lf). dec %u cores.",
                total_bw, dram_limit, minus);

            if (cpu_d->BE_cores_dec(minus) == false) {
                print_err("[CMC] BE_cores_dec() failed.");
            }
            continue;
        }
        // if memory bw is overused, cut the extra tasks

        if (state == STATE::GROW_LLC) {
            print_log("[CMC] GROW_LLC.");
            // GROW_LLC stage.

            double old_slack = puller->pull_latency_info().slack();
            // pull slack

            cc_d->BE_cache_grow();
            usleep(1500000);

            double new_bw = mm_d->measure_dram_bw();
            // grow cache, wait 1 second for it to take effect, measure again.
            double new_slack = puller->pull_latency_info().slack();

            if (new_bw > dram_limit) {
                print_log("[CMC] cache_growth causes bw overflow. roll_back().");
                cc_d->BE_cache_roll_back();
                state = STATE::GROW_CORES;
                continue;
            }
            // if cache_growth causes bw overuse, then roll_back() and grow_cores

            double derivative = new_bw - total_bw;
            if (derivative >= 0) {
                print_log("[CMC] cache growth causes bw increasing. roll_back().");
                cc_d->BE_cache_roll_back();
                state = STATE::GROW_CORES;
                continue;
            }
            // if cache_growth causes bw increasing, then roll_back() and grow cores

            double diff = new_slack - old_slack;
            if(diff < -0.15 || new_slack < 0) {
                print_log("[CMC] cache_growth causes latency explosion. roll_back().");
                cc_d->BE_cache_roll_back();
                state = STATE::GROW_CORES;
                continue;
            }
            // if cache_growth causes latency explosion, then roll_back() and grow cores

        } else if (state == STATE::GROW_CORES) {
            print_log("[CMC] GROW_CORES.");
            // GROW_CORES stage.

            double needed = mm_d->LC_bw() + mm_d->BE_bw() +
                            mm_d->BE_bw_per_core(); 
            print_log("[CMC] predict BW needed: %6.1lf", needed);
            // predict the bw after giving one more core to BE.

            double slack = puller->pull_latency_info().slack();
            // pull the slack

            if (needed > dram_limit) {
                state = STATE::GROW_LLC;
                // if predicted bw is more than limit, then grow llc.

            } else if (slack > 0.30) {
                if (cpu_d->BE_cores_inc(1) == false) {
                    print_err("[CMC] BE_cores_inc() failed.");
                } else {
                    state = STATE::GROW_LLC;
                }
                // if slack > 30%, then give one more core to BE.
            }
        }
    }
}

void CoreMemoryController::clear() {
    if (tap->BE_pid() != -1) {
        tap->kill_BE();
    }
    cpu_d->clear();
    mm_d->clear();
    cc_d->clear();
}

bool CoreMemoryController::set_new_BE_task(pid_t pid) {
    return cpu_d->set_new_BE_task(pid);
}
