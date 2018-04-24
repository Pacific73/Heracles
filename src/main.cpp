#include "helpers.h"
#include "heracles.h"
#include <cstring>

int main(int argc, char **argv) {

    if (argc >= 2) {
        if (strcmp(argv[1], "-debug") == 0) {
            set_debug(true);
        }
    }

    Heracles *heracles = new Heracles();
    heracles->exec();

    return 0;
}