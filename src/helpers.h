#ifndef HELPERS_H
#define HELPERS_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#define OPT_LC 0
#define OPT_BE 1

void set_debug(bool d);

void set_log(bool l);

void print_err(const char *fmt, ...);

void print_log(const char *fmt, ...);

void intel_mutex_init();

bool intel_init(bool mbm = false);

bool intel_fini();

inline static double bytes_to_kb(const double bytes) { return bytes / 1024.0; }

inline static double bytes_to_mb(const double bytes) {
    return bytes / (1024.0 * 1024.0);
}

void split_string(const std::string& s, std::vector<std::string>& v, const std::string& c);

template <typename T> static T get_opt(const char *name, T defVal) {
    const char *opt = getenv(name);

    print_log("[ENV] %s = %s", name, opt);
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