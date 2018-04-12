#ifndef HELPERS_H
#define HELPERS_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#define OPT_LC 0
#define OPT_BE 1

static void print_err(std::string msg) {
    std::cerr << "[Heracles] " << msg << std::endl;
}

template <typename T> static T get_opt(const char *name, T defVal) {
    const char *opt = getenv(name);

    std::cout << name << " = " << opt << std::endl;
    if (!opt)
        return defVal;
    std::stringstream ss(opt);
    if (ss.str().length() == 0)
        return defVal;
    T res;
    ss >> res;
    if (ss.fail()) {
        std::cerr << "WARNING: Option " << name << "(" << opt << ") could not"
                  << " be parsed, using default" << std::endl;
        return defVal;
    }
    return res;
}

#endif