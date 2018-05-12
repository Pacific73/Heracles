#include "memory_driver.h"
#include <ctime>

MemoryDriver::MemoryDriver(Tap *t, CpuDriver *cd) : tap(t), cpu_d(cd) {
    int res = get_opt<int>("HERACLES_IS_NUMA", 0);
    if (res)
        is_numa = true;
    else
        is_numa = false;
}

double MemoryDriver::measure_bw(size_t lower, size_t upper) {
    if (!intel_init(true) || upper < lower)
        return 0;

    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;
    const struct pqos_capability *cap_mon = NULL;
    int ret;

    ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err(
            "[MEMORYDRIVER] measure_bw() error retrieving PQoS capabilities.");
        return -1;
    }
    (void)pqos_cap_get_type(p_cap, PQOS_CAP_TYPE_MON, &cap_mon);
    // get cpu info and capability info

    const enum pqos_mon_event perf_events = (pqos_mon_event)(
        PQOS_PERF_EVENT_IPC | PQOS_PERF_EVENT_LLC_MISS);
    enum pqos_mon_event sel_events_max = (pqos_mon_event)(0);

    for (size_t i = 0; i < cap_mon->u.mon->num_events; i++)
        sel_events_max = (pqos_mon_event)(sel_events_max | cap_mon->u.mon->events[i].type);

    sel_events_max = (pqos_mon_event)(sel_events_max & ~perf_events);
    // get all supported events and remove perf events: IPC/LLC-MISSES

    size_t core_cnt = upper - lower + 1;
    pqos_mon_data m_mon_data[core_cnt];
    pqos_mon_data *m_mon_grps[core_cnt];
    for(size_t i = 0; i < core_cnt; ++i)
        m_mon_grps[i] = &m_mon_data[i];
    // create data array and init

    for (size_t i = 0; i < core_cnt; ++i) {
        unsigned lcore = lower + i;
        ret = pqos_mon_start(1, &lcore, sel_events_max, nullptr,
                             m_mon_grps[i]);
        if (ret != PQOS_RETVAL_OK) {
            print_err("[MEMORYDRIVER] measure_bw() monitoring start error "
                         "on core %u, status %d.", lcore, ret);
            return -1;
        }
    }
    // start monitoring

    ret = pqos_mon_poll(m_mon_grps, core_cnt);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[MEMORYDRIVER] measure_bw() 1st time failed to poll "
                  "monitoring data!");
        return -1;
    }
    // pull monitor data for the first time

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 50000000;
    nanosleep(&ts, nullptr);
    // sleep 0.05s to accumulate data

    ret = pqos_mon_poll(m_mon_grps, core_cnt);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[MEMORYDRIVER] measure_bw() 2nd time failed to poll "
                  "monitoring data!");
        return -1;
    }
    // pull monitor data for the second time

    double total_bw = 0;
    for (size_t i = 0; i < core_cnt; ++i) {
        const struct pqos_event_values *pv = &m_mon_grps[i]->values;
        double llc = bytes_to_kb(pv->llc);
        double mbr = bytes_to_mb(pv->mbm_remote_delta) * 20;
        double mbl = bytes_to_mb(pv->mbm_local_delta) * 20;
        total_bw += mbl;
        if (is_numa) {
            total_bw += mbr;
        }
    }
    // add bandwidths for these cores

    for (size_t i = 0; i < core_cnt; ++i) {
        ret = pqos_mon_stop(m_mon_grps[i]);
        if (ret != PQOS_RETVAL_OK) {
            print_err("[MEMORYDRIVER] measure_bw() monitoring stop error.");
            return -1;
        }
    }

    if (!intel_fini()) {
        return -1;
    }
    return total_bw;
}

double MemoryDriver::measure_dram_bw() {
    double res;
    size_t lower = 0;
    size_t upper = cpu_d->total_core_num() - 1;
    if ((res = measure_bw(lower, upper)) == -1) {
        print_err("[MEMORYDRIVER] measure_dram_bw() error.");
        return 0;
    }
    return res;
}

double MemoryDriver::predicted_total_bw() {
    return 0;
}

double MemoryDriver::LC_bw() {
    double res;
    size_t lower = cpu_d->BE_core_num();
    size_t upper = cpu_d->total_core_num() - cpu_d->sys_core_num() - 1;
    if ((res = measure_bw(lower, upper)) == 0) {
        print_err("[MEMORYDRIVER] measure_LC_bw() error.");
        return 0;
    }
    return res;
}

double MemoryDriver::BE_bw() {
    double res;
    size_t lower = 0;
    size_t upper = cpu_d->BE_core_num() - 1;
    if ((res = measure_bw(lower, upper)) == 0) {
        print_err("[MEMORYDRIVER] measure_BE_bw() error.");
        return 0;
    }
    return res;
}

double MemoryDriver::BE_bw_per_core() {
     double res = BE_bw();
     size_t cnt = cpu_d->BE_core_num();
     return res / cnt;
}

void MemoryDriver::clear() {}