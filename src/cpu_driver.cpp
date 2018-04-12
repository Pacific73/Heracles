#include "cpu_driver.h"
#include "helpers.h"
#include <cassert>
#include <csignal>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

CpuDriver::CpuDriver(std::string p, pid_t lc) : path(p) {

    assert(init_cgroups_dir() == true);

    BE_cores = 0;
    total_cores = (size_t)get_nprocs();
    sys_cores = get_opt("HERACLES_IDLE_CORE_NUM", 1);

    assert(total_cores > sys_cores + 2);

    BE_pid = -1;
    LC_pid = lc;

    if(!update()) {
        print_err("[CPU_DRIVER] init update failed.");
    }
}

bool init_cgroups_dir() {
    std::string paths[2] = {path + "/cpuset/LC", path + "/cpuset/BE"};
    for (int i = 0; i < 2; ++i) {
        int res = access(paths[i], F_OK);
        if (res == 0) {
            res = rmdir(paths[i]);
            if (res != 0) {
                print_err("[CPU_DRIVER] can't remove old cgroups_cpuset dir.");
                return false;
            }
        }
        res = mkdir(paths[i], S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (res != 0) {
            print_err("[CPU_DRIVER] can't create new cgroups_cpuset dir.");
            return false;
        }
    }
    return true;
}

bool CpuDriver::set_pid_cores(size_t type, size_t left, size_t right) {
    std::string p;
    if (type == OPT_LC)
        p = path + "/cpuset/LC";
    else if (type == OPT_BE)
        p = path + "/cpuset/BE";
    else
        return false;

    ofstream proc;
    proc.open((p + "/procs").c_ctr(), std::ios::out);
    if (!proc) {
        print_err("[CPU_DRIVER] can't echo to cpuset.procs file.");
        return false;
    }
    proc << cur_pid << endl;
    proc.close();

    ofstream cpu_file;
    cpu_file.open((p + "/cpus").c_str(), std::ios::out);
    if (!cpu_file) {
        print_err("[CPU_DRIVER] can't echo to cpuset.cpus file.");
        return false;
    }
    cpu_file << left << "-" << right << endl;
    cpu_file.close();

    ofstream ex_file;
    ex_file.open((p + "/cpu_exclusive").c_str(), std::ios::out);
    if (!ex_file) {
        print_err("can't echo to cpuset.exclusive file.");
        return false;
    }
    ex_file << 1 << endl;
    ex_file.close();
    return true;
}

size_t CpuDriver::total_cores() { return total_cores; }

bool CpuDriver::update() {
    if (cur_pid == -1)
        return false;
    if (!set_pid_cores(OPT_BE, 0, BE_cores - 1)) {
        print_err("[CPU_DRIVER] set BE cores error.");
        return false;
    }
    if (!set_pid_cores(OPT_LC, BE_cores, total_cores - sys_cores - 1)) {
        print_err("[CPU_DRIVER] set LC cores error.");
        return false;
    }
    return true;
}

bool CpuDriver::set_new_BE_task(pid_t pid) {
    int check_res = kill(pid, 0);
    if (check_res != 0) {
        print_err("[CPU_DRIVER] new BE task's pid error.");
        return false;
    }
    cur_pid = pid;
}

void CpuDriver::end_BE_task() {
    BE_cores = 0;
    cur_pid = -1;
}

bool CpuDriver::BE_cores_inc(size_t inc) {
    size_t tmp = BE_cores;
    if (BE_cores + inc >= total_cores - sys_cores - 1) {
        return false;
    }
    BE_cores += inc;
    if (update()) {
        return true;
    } else {
        BE_cores = tmp;
        return false;
    }
}

bool CpuDriver::BE_cores_dec(size_t dec) {
    size_t tmp = BE_cores;
    if (dec >= BE_cores - sys_cores - 1) {
        BE_cores = 0;
    } else {
        BE_cores -= dec;
    }
    if (update()) {
        return true;
    } else {
        BE_cores = tmp;
        return false;
    }
}
