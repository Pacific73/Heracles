#ifndef DB_DRIVER_H
#define DB_DRIVER_H

#include <string>

class DatabaseDriver {
  private:
  public:
    DatabaseDriver(std::string path);
    std::string next_command();
    bool finish_task();
};

#endif