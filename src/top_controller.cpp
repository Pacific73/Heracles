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

bool TopController::load_config() {

    std::string prefix = "/home/" + std::string(getlogin()) + "/";
    std::string default_latency_file = prefix + "latency_info";
    latency_path = get_opt("HERACLES_LATENCY_FILE", default_latency_file);

    std::string default_load_file = prefix + "load_info";
    load_path = get_opt("HERACLES_LOAD_FILE", default_load_file);

    std::string default_max_load_file = prefix + "max_load_info";
    max_load_file = get_opt("HERACLES_MAX_LOAD_FILE", default_max_load_file);
}
bool TopController::update() {
    ifstream load_in(load_path);
    if(!load_in.is_open()) {
        print_err("can't open load file.");
        return false;
    }
    if(!load_in.eof()) {
        print_err("nothing in load file.");
        return false;
    }
    load_in >> cur_load;
    load_in.close();

    ifstream max_load_in(max_load_path);
    if(!max_load_in.is_open()) {
        print_err("can't open max_load file.");
        return false;
    }
    if(!max_load_in.eof()) {
        print_err("nothing in max_load file.");
        return false;
    }
    max_load_in >> max_load;
    max_load_in.close();

    ifstream latency_in(latency_path);
    if(!latency_in.is_open()) {
        print_err("can't open latency file.");
        return false;
    }
    if(!latency_in.eof()) {
        print_err("nothing in latency file.");
        return false;
    }
    latency_in >> l95 >> l99 >> lmax;
    latency_in.close();
    return true;
}

int run() {}