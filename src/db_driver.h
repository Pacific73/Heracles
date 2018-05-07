#ifndef DB_DRIVER_H
#define DB_DRIVER_H

#include <string>

class DatabaseDriver {
  private:
    std::string path;
  public:
    DatabaseDriver();
    std::string next_command();
    void task_finish();
};

#endif