#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include "BPF.h"
#include <map>
#include <pthread.h>
#include <string>
#include <vector>

struct info_t {
    uint32_t classid;
    char name[16];
};

class NetworkMonitor {
  private:
    ebpf::BPF *bpf;
    static const std::string BPF_PROGRAM;

    std::string device;
    uint32_t LC_classid;
    uint32_t BE_classid;

    pthread_t tid;

    std::vector<std::pair<info_t, uint64_t>> class_dev_bytes;
    std::map<uint32_t, uint64_t> class_bytes;

    bool init_ebpf();
    bool init_config();

  public:
    NetworkMonitor();
    ~NetworkMonitor();

    void run();

    uint64_t LC_bytes();
    uint64_t BE_bytes();
};

#endif