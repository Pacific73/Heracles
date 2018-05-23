#include "network_driver.h"
#include "helpers.h"
#include <cassert>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

NetworkDriver::NetworkDriver() {
    assert(init_config() == true);
    assert(init_dir() == true);
    assert(init_classid() == true);
    assert(init_tc() == true);

    print_log("[NET_DRIVER] inited.");
}

bool NetworkDriver::init_config() {
    device = get_opt<std::string>("NIC_NAME", "lo");
    cgroup_path = get_opt<std::string>("CGROUPS_DIR", "/sys/fs/cgroup");
    total_bw = get_opt<uint64_t>("NET_TOTAL_BANDWIDTH", 1e9);
    LC_classid = 0x10003;
    BE_classid = 0x10004;
    return true;
}

bool NetworkDriver::init_dir() {
    std::string paths[2] = {cgroup_path + "/net_cls/LC",
                            cgroup_path + "/net_cls/BE"};

    for (int i = 0; i < 2; ++i) {
        int res = access(paths[i].c_str(), F_OK);
        if (res == 0) {
            res = rmdir(paths[i].c_str());
            if (res != 0) {
                print_err(
                    "[NET_DRIVER] can't remove existed cgroups_net_cls dir.");
                return false;
            }
        }
        res = mkdir(paths[i].c_str(),
                    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (res != 0) {
            print_err("[NET_DRIVER] can't create new cgroups_net_cls dir.");
            return false;
        }
    }

    return true;
}

bool NetworkDriver::init_classid() {
    uint32_t ids[2] = {LC_classid, BE_classid};
    std::string tasks[2] = {"LC", "BE"};

    for (int i = 0; i < 2; ++i) {
        std::ofstream classid_f;
        classid_f.open(cgroup_path + "/net_cls/" + tasks[i] +
                       "/net_cls.classid");
        if (!classid_f) {
            print_err("[NET_DRIVER] echo %s's classid failed.", tasks[i]);
            return false;
        }
        classid_f << ids[i] << std::endl;
        classid_f.close();
    }
    return true;
}

bool NetworkDriver::init_tc() {

    size_t cnt = 6;
    std::string command[cnt];

    command[0] = str_format("tc qdisc del dev %s root", device.c_str());
    command[1] = str_format("tc qdisc add dev %s root handle 1: htb", device.c_str());
    command[2] = str_format(
        "tc class add dev %s parent 1: classid 1: htb rate %llu ceil %llu",
        device.c_str(), total_bw, total_bw);
    command[3] =
        str_format("tc class add dev %s parent 1: classid 1:%u htb rate %llu",
                   device.c_str(), LC_classid % (1 << 16), total_bw);
    command[4] =
        str_format("tc class add dev %s parent 1: classid 1:%u htb rate %llu",
                   device.c_str(), BE_classid % (1 << 16), total_bw);
    command[5] = str_format(
        "tc filter add dev %s protocol ip parent 1:0 prio 1 handle 1: cgroup",
        device.c_str());

    for (size_t i = 0; i < cnt; ++i) {
        int res = system(command[i].c_str());
        // if (res != 0) {
        //     print_err("[NET_DRIVER] init_tc() error: %s", command[i].c_str());
        //     return false;
        // }
    }
    return true;
}

bool NetworkDriver::set_LC_procs(pid_t pid) {
    if (LC_pid == pid)
        return true;
    LC_pid = pid;

    std::ofstream procs;
    procs.open(cgroup_path + "/net_cls/LC/cgroup.procs", std::ios::out);
    if (!procs) {
        print_err("[NET_DRIVER] set_LC_procs error.");
        return false;
    }

    if(LC_pid != -1)
        procs << LC_pid;
    procs << std::endl;
    procs.close();
    return true;
}

bool NetworkDriver::set_BE_procs(pid_t pid) {
    if (BE_pid == pid)
        return true;
    BE_pid = pid;

    std::ofstream procs;
    procs.open(cgroup_path + "/net_cls/BE/cgroup.procs", std::ios::out);
    if (!procs) {
        print_err("[NET_DRIVER] set_BE_procs error.");
        return false;
    }

    if(BE_pid != -1)
        procs << BE_pid;
    procs << std::endl;
    procs.close();
    return true;
}

bool NetworkDriver::set_BE_bw(uint64_t bw) {
    std::string command;
    command = str_format(
        "tc class change dev %s parent 1: classid 1:%u htb rate %llu", device.c_str(),
        BE_classid % (1 << 16), bw);

    int res = system(command.c_str());
    if(res != 0) {
        print_err("[NET_DRIVER] set_BE_bw() error");
        return false;
    }
    return true;
}