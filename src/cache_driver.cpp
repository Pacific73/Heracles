#include "cache_driver.h"
#include "helpers.h"
#include "pqos.h"
#include <cstring>

#define SYS_CLASS_ID (0)
#define LC_CLASS_ID (1)
#define BE_CLASS_ID (2)

CacheDriver::CacheDriver() {
    if (!intel_init()) {
        exit(-1);
    }

    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;

    int ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] error retrieving PQoS capabilities.\n");
        exit(-1);
    }
    // check CMT capability and CPU info pointer

    ret = pqos_l3ca_get_min_cbm_bits(&min_bits);
    // min_bits should be 8/16/32 for most machines

    if (ret == PQOS_RETVAL_RESOURCE) {
        print_err("[CACHEDRIVER] can't determine the min_bits for CLOS.");
    } else if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] error getting min_bits for CLOS.");
    } else {
        init_masks();
    }
    // get min_bits for CAT settings and init all masks
    // e.g. min_bits = 16, LC_mask = 0xfffc, sys_mask = 0x0003, BE_mask = 0

    if (!intel_fini() || ret != PQOS_RETVAL_OK) {
        exit(-1);
    }
}

void CacheDriver::init_masks() {
    uint64_t init;

    sys_bits = min_bits / 8;
    init = 0;
    for (size_t i = 0; i < sys_bits; ++i)
        init = init << 1 + 1;
    sys_mask = init;

    BE_bits = 1;
    init = 1;
    init = init << (min_bits - 1);
    BE_mask = init;

    LC_bits = min_bits - sys_bits - 1;
    init = 0;
    for (size_t i = 0; i < LC_bits; ++i)
        init = init << 1 + 1;
    init = init << sys_bits;
    LC_mask = init;
}

bool CacheDriver::intel_init() {
    pqos_config cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fd_log = STDOUT_FILENO;
    cfg.verbose = 0;
    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
        print_err(
            "[CACHEDRIVER] intel_init() error initializing PQoS library.\n");
        return false;
    }
    return true;
}

bool CacheDriver::intel_fini() {
    int ret = pqos_fini();
    if (ret != PQOS_RETVAL_OK) {
        print_err(
            "[CACHEDRIVER] intel_fini() error shutting down PQoS library.\n");
        return false;
    }
    return true;
}

bool CacheDriver::update_association(size_t BE_core_num, size_t sys_core_num,
                                     size_t total_core_num) {
    if (!intel_init()) {
        return false;
    }

    int ret;
    for (size_t i = 0; i < total_core_num; ++i) {
        if (0 <= i && i < BE_core_num) {
            ret = pqos_alloc_assoc_set(i, LC_CLASS_ID);
        } else if (BE_core_num <= i && i < total_core_num - sys_core_num) {
            ret = pqos_alloc_assoc_set(i, BE_CLASS_ID);
        } else {
            ret = pqos_alloc_assoc_set(i, SYS_CLASS_ID);
        }
        if (ret != PQOS_RETVAL_OK) {
            print_err("[CACHEDRIVER] update_association() setting allocation "
                      "class of service "
                      "association failed.\n");
            return false;
        }
    }
    // set core [0, BE-1]:                  for LC  class - CLOS 1
    // set core [BE, total-sys-1]:          for BE  class - CLOS 2
    // set core [total-sys, total-1]:       for sys class - CLOS 0 (default
    // CLOS)

    if (!intel_fini()) {
        return false;
    }
    return true;
}

bool CacheDriver::update_allocation() {
    if (!intel_init()) {
        return false;
    }
    unsigned sock_count, *sockets = NULL;

    sockets = pqos_cpu_get_sockets(p_cpu, &sock_count);
    if (sockets == NULL) {
        print_err("[CACHEDRIVER] update_allocation() error retrieving CPU "
                  "socket information.\n");
        return false;
    }

    pqos_l3ca tab[3];

    tab[0].class_id = SYS_CLASS_ID;
    tab[0].cdp = 0;
    tab[0].u.ways_mask = sys_mask;

    tab[1].class_id = BE_CLASS_ID;
    tab[1].cdp = 0;
    tab[1].u.ways_mask = BE_mask;

    tab[2].class_id = LC_CLASS_ID;
    tab[2].cdp = 0;
    tab[2].u.ways_mask = LC_mask;

    int ret = pqos_l3ca_set(*sockets, 3, tab);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] update_allocation() error setting CLOS masks.")
    }

    if (!intel_fini()) {
        return false;
    }
    return true;
}

bool CacheDriver::BE_cache_grow() {}

bool CacheDriver::BE_cache_roll_back() {}

void CacheDriver::clear() { init_masks(); }