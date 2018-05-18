#include "cpu_driver.h"
#include "helpers.h"
#include "pqos.h"
#include <cassert>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

CpuDriver::CpuDriver(Tap *t, CacheDriver *cd) : tap(t), cc_d(cd) {
    if (pthread_mutex_init(&mutex, nullptr) != 0) {
        print_err("[CPU_DRIVER] mutex init failed.");
    }

    assert(init_config() == true);

    assert(init_cgroups_dir() == true);
    // init cgroups VFS

    assert(init_core_num() == true);
    // init core nums

    assert(init_mem_data() == true);
    // get cpuset.mems data

    if (!update()) {
        print_err("[CPU_DRIVER] CpuDriver() init update failed.");
        exit(-1);
    }
    // set

    if (!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        print_err("[CPU_DRIVER] CpuDriver() init cache_driver failed.");
        exit(-1);
    }
}

bool CpuDriver::init_config() {
    path = get_opt<std::string>("CGROUPS_DIR", "/sys/fs/cgroups");
    return true;
}

bool CpuDriver::init_cgroups_dir() {
    std::string paths[2] = {path + "/cpuset/LC", path + "/cpuset/BE"};
    for (int i = 0; i < 2; ++i) {
        int res = access(paths[i].c_str(), F_OK);
        if (res == 0) {
            res = rmdir(paths[i].c_str());
            if (res != 0) {
                print_err(
                    "[CPU_DRIVER] can't remove existed cgroups_cpuset dir.");
                return false;
            }
        }
        res = mkdir(paths[i].c_str(),
                    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (res != 0) {
            print_err("[CPU_DRIVER] can't create new cgroups_cpuset dir.");
            return false;
        }
    }
    return true;
}

bool CpuDriver::init_core_num() {

    if (!intel_init()) {
        return false;
    }

    unsigned *sockets, sock_count;
    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;

    int ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[CPU_DRIVER] error retrieving PQoS capabilities!\n");
        return false;
    }

    sockets = pqos_cpu_get_sockets(p_cpu, &sock_count);
    if (sockets == NULL) {
        print_err("[CPU_DRIVER] error retrieving CPU socket information.");
        return false;
    }
    assert(sock_count >= 1);
    // assert: sock_count >= 1

    unsigned *lcores = NULL;
    unsigned lcount = 0;
    lcores = pqos_cpu_get_cores(p_cpu, sockets[0], &lcount);
    if (lcores == NULL || lcount == 0) {
        print_err("[CPU_DRIVER] error retrieving core information.");
        free(lcores);
        free(sockets);
        return false;
    }
    // get core_count on socket 0 (this program is only for single socket
    // structure)

    BE_cores = 0;
    sys_cores = get_opt<size_t>("HERACLES_IDLE_CORE_NUM", 1);
    total_cores = get_opt<size_t>("HERACLES_TOTAL_CORE_NUM", 8);

    assert(total_cores <= lcount);
    assert(total_cores > sys_cores + 2);

    if (!intel_fini()) {
        return false;
    }

    if (!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        return false;
    }
    // set core_num and check bounds, update core CLOS mapping

    return true;
}

bool CpuDriver::init_mem_data() {
    std::ifstream mem;
    mem.open((path + "/cpuset/cpuset.mems").c_str(), std::ios::in);
    if (!mem) {
        print_err("[CPU_DRIVER] can't get cpuset.mems data.");
        return false;
    }
    mem >> mem_data;
    mem.close();
    return true;
}

bool CpuDriver::set_cores_for_pid(size_t type, size_t left, size_t right) {

    std::string p;
    pid_t pid;
    if (type == OPT_LC) {
        p = path + "/cpuset/LC";
        pid = tap->LC_pid();
    } else if (type == OPT_BE) {
        p = path + "/cpuset/BE";
        pid = tap->BE_pid();
    } else
        return false;

    std::ofstream mem_file;
    mem_file.open((p + "/cpuset.mems").c_str(), std::ios::out);
    if (!mem_file) {
        print_err("[CPU_DRIVER] can't echo to cpuset.mems file.");
        return false;
    }
    mem_file << mem_data << std::endl;
    mem_file.close();

    std::ofstream cpu_file;
    cpu_file.open((p + "/cpuset.cpus").c_str(), std::ios::out);
    if (!cpu_file) {
        print_err("[CPU_DRIVER] can't echo to cpuset.cpus file.");
        return false;
    }
    if (pid != -1) {
        if (left != right)
            cpu_file << left << "-" << right << std::endl;
        else
            cpu_file << left << std::endl;
    } else {
        cpu_file << std::endl;
    }
    cpu_file.close();

    std::ofstream proc;
    proc.open((p + "/cgroup.procs").c_str(), std::ios::out);
    if (!proc) {
        print_err("[CPU_DRIVER] can't echo to cpuset.procs file.");
        return false;
    }
    if (pid != -1) {
        proc << pid << std::endl;
    } else {
        proc << std::endl;
    }
    proc.close();

    std::ofstream ex_file;
    ex_file.open((p + "/cpuset.cpu_exclusive").c_str(), std::ios::out);
    if (!ex_file) {
        print_err("[CPU_RDRIVER] can't echo to cpuset.exclusive file.");
        return false;
    }
    ex_file << 1 << std::endl;
    ex_file.close();

    if (pid != -1) {
        print_log("[CPU_DRIVER] [%s] cgroup.proc: %d cpuset.cpus: %u-%u",
                  type == OPT_LC ? "LC" : "BE", pid, left, right);
    } else {
        print_log("[CPU_DRIVER] [%s] cgroup.proc: NULL cpuset.cpus: NULL",
                  type == OPT_LC ? "LC" : "BE");
    }

    return true;
}

size_t CpuDriver::total_core_num() const { return total_cores; }

size_t CpuDriver::BE_core_num() const { return BE_cores; }

size_t CpuDriver::sys_core_num() const { return sys_cores; }

bool CpuDriver::update(bool inc) {
    if (tap->LC_pid() == -1) {
        print_err("[CPU_DRIVER] LC_pid == -1!");
        return false;
    }
    if (inc) {
        if (!set_cores_for_pid(OPT_LC, BE_cores, total_cores - sys_cores - 1)) {
            print_err("[CPU_DRIVER] update() set LC cores error.");
            return false;
        }
        if (!set_cores_for_pid(OPT_BE, 0, BE_cores - 1)) {
            print_err("[CPU_DRIVER] update() set BE cores error.");
            return false;
        }
    } else {
        if (!set_cores_for_pid(OPT_BE, 0, BE_cores - 1)) {
            print_err("[CPU_DRIVER] update() set BE cores error.");
            return false;
        }
        if (!set_cores_for_pid(OPT_LC, BE_cores, total_cores - sys_cores - 1)) {
            print_err("[CPU_DRIVER] update() set LC cores error.");
            return false;
        }
    }

    if (!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        print_err("[CPU_DRIVER] update() update association error.");
        return false;
    }
    return true;
}

bool CpuDriver::set_new_BE_task(pid_t pid) { return BE_cores_inc(1); }

bool CpuDriver::BE_cores_inc(size_t inc) {

    pthread_mutex_lock(&mutex);

    size_t tmp = BE_cores;
    if (BE_cores + inc > total_cores - sys_cores - 1) {
        pthread_mutex_unlock(&mutex);
        return false;
    }

    BE_cores += inc;

    if (update(true)) {
        pthread_mutex_unlock(&mutex);
        print_log("[CPU_DRIVER] BE_cores_inc(%u) success -> %u", inc, BE_cores);
        return true;
    } else {
        BE_cores = tmp;
        pthread_mutex_unlock(&mutex);
        return false;
    }
}

bool CpuDriver::BE_cores_dec(size_t dec) {
    pthread_mutex_lock(&mutex);
    size_t tmp = BE_cores;

    if (dec >= BE_cores - sys_cores - 1) {
        BE_cores = 0;
    } else {
        BE_cores -= dec;
    }

    if (update(false)) {
        pthread_mutex_unlock(&mutex);
        print_log("[CPU_DRIVER] BE_cores_dec(%u) success -> %u", dec, BE_cores);
        return true;
    } else {
        BE_cores = tmp;
        pthread_mutex_unlock(&mutex);
        return false;
    }
}

void CpuDriver::clear() {
    BE_cores = 0;
    update();
}
