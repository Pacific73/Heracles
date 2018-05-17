#include "helpers.h"
#include <cstdarg>
#include <cstring>
#include <pqos.h>
#include <pthread.h>
#include <string>

bool is_debug = false;
bool is_log = false;

void set_debug(bool d) { is_debug = d; }

void set_log(bool l) { is_log = l; }

void print_err(const char *fmt, ...) {
    if (is_debug) {
        std::cout << "[ERROR] - ";
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        std::cout << std::endl;
    }
}

void print_log(const char *fmt, ...) {
    if (is_log) {
        std::cout << "[ LOG ] - ";
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        std::cout << std::endl;
    }
}

pthread_mutex_t mutex;

void intel_mutex_init() { pthread_mutex_init(&mutex, nullptr); }

bool intel_init(bool mbm) {
    pthread_mutex_lock(&mutex);
    pqos_config cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fd_log = STDOUT_FILENO;
    cfg.verbose = 0;
    if (mbm) {
        cfg.interface = PQOS_INTER_MSR;
    }
    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    int ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_init() error initializing PQoS library.");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    return true;
}

bool intel_fini() {
    int ret = pqos_fini();
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_fini() error shutting down PQoS library.");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    pthread_mutex_unlock(&mutex);
    return true;
}

void str_split(const std::string &s, std::vector<std::string> &v,
                  const std::string &c) {
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2) {
        v.push_back(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));
}

std::string str_format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
 
    const size_t SIZE = 512;
    char buffer[SIZE] = { 0 };
    vsnprintf(buffer, SIZE, fmt, ap);
 
    va_end(ap);
 
    return std::string(buffer);
}

uint64_t get_cur_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t t = ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
    return t;
}
