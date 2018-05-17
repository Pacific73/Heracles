#ifndef NETWORK_DRIVER_H
#define NETWORK_DRIVER_H
#include <string>

class NetworkDriver {
  private:
    std::string cgroup_path;
    std::string device;
    uint64_t total_bw;

    uint32_t LC_classid;
    uint32_t BE_classid;

    pid_t LC_pid;
    pid_t BE_pid;

    bool init_config();
    bool init_dir();
    bool init_classid();
    bool init_tc();

  public:
    NetworkDriver();

    bool set_LC_procs(pid_t pid);

    bool set_BE_procs(pid_t pid);
    bool set_BE_bw(uint64_t bw);
};

#endif