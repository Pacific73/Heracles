#ifndef HELPERS_H
#define HELPERS_H

#include "pqos.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define OPT_LC 0
#define OPT_BE 1

static bool debug = false;

static void set_debug(bool d) { debug = d; }

void print_err(std::string msg) {
    if (debug) {
        std::cerr << "[Heracles] - " << msg << std::endl;
    }
}

template <typename T> static T get_opt(const char *name, T defVal) {
    const char *opt = getenv(name);

    std::cout << name << " = " << opt << std::endl;
    if (!opt)
        return defVal;
    std::stringstream ss(opt);
    if (ss.str().length() == 0)
        return defVal;
    T res;
    ss >> res;
    if (ss.fail()) {
        std::cerr << "WARNING: Option " << name << "(" << opt << ") could not"
                  << " be parsed, using default" << std::endl;
        return defVal;
    }
    return res;
}

bool intel_init(bool mbm = false) {
    pqos_config cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fd_log = STDOUT_FILENO;
    cfg.verbose = 0;
    if (mbm) {
        cfg.interface = PQOS_INTER_MSR;
    }
    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_init() error initializing PQoS library.\n");
        return false;
    }
    return true;
}

bool intel_fini() {
    int ret = pqos_fini();
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_fini() error shutting down PQoS library.\n");
        return false;
    }
    return true;
}

inline double bytes_to_kb(const double bytes) { return bytes / 1024.0; }

inline double bytes_to_mb(const double bytes) {
    return bytes / (1024.0 * 1024.0);
}

#endif