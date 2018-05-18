#ifndef HERACLES_H
#define HERACLES_H

#include "core_memory_controller.h"
#include "helpers.h"
#include "info_puller.h"
#include "network_controller.h"
#include "tap.h"
#include "top_controller.h"
#include <pthread.h>

void *run_cm_ctr(void *p) {
    CoreMemoryController *cm_ctr = reinterpret_cast<CoreMemoryController *>(p);
    cm_ctr->run();
    return nullptr;
}

void *run_tap(void *p) {
    Tap *tap = reinterpret_cast<Tap *>(p);
    tap->run();
    return nullptr;
}

void *run_net_ctr(void *p) {
    NetworkController *net_ctr = reinterpret_cast<NetworkController *>(p);
    net_ctr->run();
    return nullptr;
}

class Heracles {
  private:
    Tap *tap;
    InfoPuller *puller;

    NetworkController *net_ctr;
    CoreMemoryController *cm_ctr;
    TopController *t_ctr;

  public:
    Heracles(pid_t lc_pid) {
        intel_mutex_init();

        tap = new Tap(lc_pid);
        puller = new InfoPuller();

        net_ctr = new NetworkController(tap);
        cm_ctr = new CoreMemoryController(tap, puller);
        t_ctr = new TopController(tap, puller);
    }

    void exec() {
        int errno;
        pthread_t t, cmc, nc;

        errno = pthread_create(&t, nullptr, run_tap, tap);
        if (errno != 0) {
            print_err("[HARACLES] can't create core_memory_controller.");
            exit(-1);
        }

        errno = pthread_create(&cmc, nullptr, run_cm_ctr, cm_ctr);
        if (errno != 0) {
            print_err("[HARACLES] can't create core_memory_controller.");
            exit(-1);
        }

        errno = pthread_create(&nc, nullptr, run_net_ctr, net_ctr);
        if (errno != 0) {
            print_err("[HERACLES] can't create network_controller.");
            exit(-1);
        }

        t_ctr->run();
    }
};

#endif