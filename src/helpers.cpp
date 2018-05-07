#include "helpers.h"
#include <pqos.h>
#include <string>
#include <cstring>

bool debug = false;

void set_debug(bool d) {
    debug = d;
}

void print_err(std::string msg)  {
    if (debug) {
        std::cerr << "[Heracles] - " << msg << std::endl;
    }
}

bool intel_init(bool mbm) {
    pqos_config cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fd_log = STDOUT_FILENO;
    cfg.verbose = 0;
    if (mbm) {
        cfg.interface = PQOS_INTER_MSR;
    }
    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    int ret = pqos_init(&cfg);
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