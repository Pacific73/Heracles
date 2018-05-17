#include "memory_driver.h"
#include <ctime>

MemoryDriver::MemoryDriver(Tap *t, CpuDriver *cd) : tap(t), cpu_d(cd) {
    int res = get_opt<int>("HERACLES_IS_NUMA", 0);
    if (res)
        is_numa = true;
    else
        is_numa = false;
    BE_bandwidth = 0;
    int core_cnt = get_opt<int>("HERACLES_TOTAL_CORE_NUM", 16);
    bandwidths = new double[core_cnt];
    for (int i = 0; i < core_cnt; ++i)
        bandwidths[i] = 0;
}

bool MemoryDriver::update() {

    uint64_t new_time = get_cur_ns();
    if (new_time - update_time <= 600000000) {
        return true;
    } else {
        update_time = new_time;
    }

    if (!intel_init(true))
        return 0;

    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;
    const struct pqos_capability *cap_mon = NULL;
    int ret;

    ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err(
            "[MEMORYDRIVER] update() error retrieving PQoS capabilities.");
        return false;
    }
    (void)pqos_cap_get_type(p_cap, PQOS_CAP_TYPE_MON, &cap_mon);
    // get cpu info and capability info

    const enum pqos_mon_event perf_events =
        (pqos_mon_event)(PQOS_PERF_EVENT_IPC | PQOS_PERF_EVENT_LLC_MISS);
    enum pqos_mon_event sel_events_max = (pqos_mon_event)(0);

    for (size_t i = 0; i < cap_mon->u.mon->num_events; i++)
        sel_events_max =
            (pqos_mon_event)(sel_events_max | cap_mon->u.mon->events[i].type);

    sel_events_max = (pqos_mon_event)(sel_events_max & ~perf_events);
    // get all supported events and remove perf events: IPC/LLC-MISSES

    size_t core_cnt = cpu_d->total_core_num();
    pqos_mon_data m_mon_data[core_cnt];
    pqos_mon_data *m_mon_grps[core_cnt];
    for (size_t i = 0; i < core_cnt; ++i)
        m_mon_grps[i] = &m_mon_data[i];
    // create data array and init

    for (size_t i = 0; i < core_cnt; ++i) {
        unsigned lcore = i;
        ret = pqos_mon_start(1, &lcore, sel_events_max, nullptr, m_mon_grps[i]);
        if (ret != PQOS_RETVAL_OK) {
            print_err("[MEMORYDRIVER] update() monitoring start error "
                      "on core %u, status %d.",
                      lcore, ret);
            return false;
        }
    }
    // start monitoring

    ret = pqos_mon_poll(m_mon_grps, core_cnt);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[MEMORYDRIVER] update() 1st time failed to poll "
                  "monitoring data!");
        return false;
    }
    // pull monitor data for the first time

    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    nanosleep(&ts, nullptr);
    // sleep 0.05s to accumulate data

    ret = pqos_mon_poll(m_mon_grps, core_cnt);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[MEMORYDRIVER] update() 2nd time failed to poll "
                  "monitoring data!");
        return false;
    }
    // pull monitor data for the second time

    for (size_t i = 0; i < core_cnt; ++i) {
        const struct pqos_event_values *pv = &m_mon_grps[i]->values;
        double llc = bytes_to_kb(pv->llc);
        double mbr = bytes_to_mb(pv->mbm_remote_delta) * 4;
        double mbl = bytes_to_mb(pv->mbm_local_delta) * 4;
        bandwidths[i] = mbl;
        if (is_numa) {
            bandwidths[i] += mbr;
        }
    }
    // add bandwidths for these cores

    for (size_t i = 0; i < core_cnt; ++i) {
        ret = pqos_mon_stop(m_mon_grps[i]);
        if (ret != PQOS_RETVAL_OK) {
            print_err("[MEMORYDRIVER] update() monitoring stop error.");
            return false;
        }
    }

    if (!intel_fini()) {
        return false;
    }
    return true;
}

double MemoryDriver::measure_dram_bw() {

    if (update() == false) {
        print_err("[MEMORYDRIVER] measure_dram_bw() error.");
        return -1;
    }
    double sum = 0;
    size_t lower = 0;
    size_t upper = cpu_d->total_core_num();
    for (size_t i = lower; i < upper; ++i) {
        sum += bandwidths[i];
    }
    return sum;
}

double MemoryDriver::predicted_total_bw() { return 0; }

double MemoryDriver::LC_bw() {

    if (update() == false) {
        print_err("[MEMORYDRIVER] LC_bw() error.");
        return -1;
    }
    double sum = 0;
    size_t lower = cpu_d->BE_core_num();
    size_t upper = cpu_d->total_core_num() - cpu_d->sys_core_num();
    for (size_t i = lower; i < upper; ++i) {
        sum += bandwidths[i];
    }
    return sum;
}

double MemoryDriver::BE_bw() {

    if (update() == false) {
        print_err("[MEMORYDRIVER] BE_bw() error.");
        return -1;
    }
    double sum = 0;
    size_t lower = 0;
    size_t upper = cpu_d->BE_core_num();
    for (size_t i = lower; i < upper; ++i) {
        sum += bandwidths[i];
    }
    return sum;
}

double MemoryDriver::BE_bw_per_core() {
    size_t cnt = cpu_d->BE_core_num();

    double sum = 0;
    size_t lower = 0;
    size_t upper = cpu_d->BE_core_num();
    for (size_t i = lower; i < upper; ++i) {
        sum += bandwidths[i];
    }
    return sum / cnt;
}

void MemoryDriver::clear() {}