#include "top_controller.h"
#include "helpers.h"
#include <fstream>
#include <iostream>

TopController::TopController(Tap *t) : tap(t) {
    l95 = 0;
    l99 = 0;
    lmax = 0;
    max_load = 0;
    load = 0;
}

void TopController::load_config() {

    std::string prefix = "/home/" + std::string(getlogin()) + "/";

    std::string default_latency_file = prefix + "latency_info";
    std::string default_max_latency_file = prefix + "max_latency_info";
    std::string default_load_file = prefix + "load_info";
    std::string default_max_load_file = prefix + "max_load_info";

    latency_path = get_opt("HERACLES_LATENCY_FILE", default_latency_file);
    max_latency_path =
        get_opt("HERACLES_MAX_LATENCY_FILE", default_max_latency_file);
    load_path = get_opt("HERACLES_LOAD_FILE", default_load_file);
    max_load_file = get_opt("HERACLES_MAX_LOAD_FILE", default_max_load_file);

    sleep_time = get_opt("TOP_SLEEP_TIME", 5);
    disable_bound = get_opt("TOP_DISABLE_BOUND", 0.85);
    enable_bound = get_opt("TOP_ENABLE_BOUND", 0.8);
    slow_be_bound = get_opt("TOP_SLOW_BE_BOUND", 0.1);
}

bool TopController::update() {
    ifstream load_in(load_path);
    if (!load_in.is_open() || !load_in.eof()) {
        print_err("can't read load file.");
        return false;
    }
    load_in >> cur_load;
    load_in.close();

    ifstream max_load_in(max_load_path);
    if (!max_load_in.is_open() || !max_load_in.eof()) {
        print_err("can't read max_load file.");
        return false;
    }
    max_load_in >> max_load;
    max_load_in.close();

    ifstream latency_in(latency_path);
    if (!latency_in.is_open() || !latency_in.eof()) {
        print_err("can't open latency file.");
        return false;
    }
    latency_in >> l95 >> l99 >> lmax;
    latency_in.close();

    ifstream max_latency_in(max_latency_path);
    if (!max_latency_in.is_open() || !max_latency_in.eof()) {
        print_err("can't open max_latency file.");
        return false;
    }
    max_latency_in >> max_latency;
    max_latency_in.close();

    return true;
}

int TopController::run() {

    load_config();

    struct timespec ts;
    ts.tv_sec = sleep_time;
    ts.tv_nsec = 0;

    while (true) {
        nanosleep(ts);
        if (!update()) {
            return -1;
        }
        double slack = (max_latency - latency) / max_latency;
        double load_percent = load / max_load;
        if (slack < 0) {
            tap->disable_BE();
            // enter_cooling_down()...
        } else if (load_percent > disable_bound) {
            tap->disable_BE();
        } else if (load_percent < enable_bound) {
            tap->enable_BE();
        } else if (slack < slow_be_bound) {
            // disallow_be_growth()...
            // if (slack < slow_be_bound / 2) {
            //     be_cores.remove(be_cores.size() - 2)
            // }
        }
    }
    return 0;
}