#ifndef CPU_DRIVER
#define CPU_DRIVER

class CpuDriver {
  private:
    std::string path;

    size_t BE_cores;
    size_t total_cores;
    size_t sys_cores;

    pid_t BE_pid;
    pid_t LC_pid;

    bool init_cgroups_dir();

    bool set_pid_cores(size_t type, size_t left, size_t right);

    bool update();

  public:
    CpuDriver(std::string p, pid_t lc);

    size_t total_cores() const;

    bool set_new_BE_task(pid_t pid);

    void end_BE_task();

    bool BE_cores_inc(size_t inc); // bound check

    bool BE_cores_dec(size_t dec); // bound check

    
};

#endif