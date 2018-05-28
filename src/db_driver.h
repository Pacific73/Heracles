#ifndef DB_DRIVER_H
#define DB_DRIVER_H

#include <string>
#include <vector>

class Task {
  public:
    char *program;
    std::vector<char*> argv;
    std::vector<std::string> v;

    bool complete;
    Task() : program(nullptr), complete(false) {}
};

class DatabaseDriver {
  private:
    std::string path;

  public:
    DatabaseDriver();
    Task next_task();
    void task_finish(bool is_normal);
};

#endif