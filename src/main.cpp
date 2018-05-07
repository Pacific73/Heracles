#include "helpers.h"
#include "heracles.h"
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv) {

    pid_t lc_pid = -1;

    if (argc >= 2) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "-debug") == 0) {
                set_debug(true);
            } else if (strcmp(argv[i], "-lc") == 0) {
                lc_pid = atoi(argv[++i]);
            } else {
                std::cout   << "Usage: " << argv[0] << "[option]" << std::endl
                            << std::endl
                            << "Options:" << std::endl
                            << "-debug      open debug mode and print debug info" << std::endl
                            << "-lc LC_pid  set LC_pid instead of setting it in env.sh"
                            << std::endl;
            }
        }
    }

    Heracles *heracles = new Heracles(lc_pid);
    heracles->exec();

    return 0;
}