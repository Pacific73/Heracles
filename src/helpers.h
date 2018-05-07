#ifndef HELPERS_H
#define HELPERS_H

#include <iostream>
#include <cstdlib>
#include <sstream>

#define OPT_LC 0
#define OPT_BE 1

void set_debug(bool d);

void print_err(std::string msg);

bool intel_init(bool mbm = false);

bool intel_fini();

inline static double bytes_to_kb(const double bytes) { return bytes / 1024.0; }

inline static double bytes_to_mb(const double bytes) {
    return bytes / (1024.0 * 1024.0);
}

template <typename T>
static T get_opt(const char *name, T defVal) {
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