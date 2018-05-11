#include "info_puller.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "helpers.h"

double LatencyInfo::slack() {
    if (slo_lat == 0)
        return 0;
    return (slo_lat - cur_lat) / slo_lat;
}

double LoadInfo::load_percent() {
    if (max_load == 0)
        return 0;
    return cur_load / max_load;
}

void InfoPuller::init_config() {
    std::string prefix = "/home/" + std::string(getlogin()) + "/heracles/";

    std::string default_latency_file = prefix + "latency_info";
    std::string default_max_latency_file = prefix + "max_latency_info";
    std::string default_load_file = prefix + "load_info";
    std::string default_max_load_file = prefix + "max_load_info";

    //mkdir!!!

    latency_path = get_opt("HERACLES_LATENCY_FILE", default_latency_file);
    max_latency_path =
        get_opt("HERACLES_MAX_LATENCY_FILE", default_max_latency_file);
    load_path = get_opt("HERACLES_LOAD_FILE", default_load_file);
    max_load_path = get_opt("HERACLES_MAX_LOAD_FILE", default_max_load_file);
}

InfoPuller::InfoPuller() {
    init_config();
    print_log("[INFOPULLER] inited.");
}

LatencyInfo InfoPuller::pull_latency_info() {
    LatencyInfo ret;

    std::ifstream latency_in(latency_path);
    if (!latency_in.is_open() || latency_in.eof()) {
        print_err("[INFOPULLER] can't open latency file.");
        return ret;
    }
    latency_in >> ret.cur_lat;
    latency_in.close();

    std::ifstream max_latency_in(max_latency_path);
    if (!max_latency_in.is_open() || max_latency_in.eof()) {
        print_err("[INFOPULLER] can't open max_latency file.");
        return ret;
    }
    max_latency_in >> ret.slo_lat;
    max_latency_in.close();

    return ret;
}

LoadInfo InfoPuller::pull_load_info() {
    LoadInfo ret;

    std::ifstream load_in(load_path);
    if (!load_in.is_open() || load_in.eof()) {
        print_err("[INFOPULLER] can't read load file.");
        return ret;
    }
    load_in >> ret.cur_load;
    load_in.close();

    std::ifstream max_load_in(max_load_path);
    if (!max_load_in.is_open() || max_load_in.eof()) {
        print_err("[INFOPULER] can't read max_load file.");
        return ret;
    }
    max_load_in >> ret.max_load;
    max_load_in.close();

    return ret;
}