#ifndef HERACLES_H
#define HERACLES_H

#include "core_memory_controller.h"
#include "info_puller.h"
#include "tap.h"
#include "top_controller.h"
#include <pthread.h>

class Heracles {
  private:
    Tap *tap;
    InfoPuller *puller;

    CoreMemoryController *cm_ctr;
    TopController *t_ctr;

  public:
    Heracles() {
        tap = new Tap();
        puller = new InfoPuller();

        cm_ctr = new CoreMemoryController(tap, puller);
        t_ctr = new TopController(tap, puller);
    }
    
    void exec() {
        int errno;
        pthread_t cm;
        errno = pthread_create(&cm, nullptr, cm_ctr->run, nullptr);
        if (errno != 0) {
            print_err("can't create core_memory_controller.");
            exit(-1);
        }

        t_ctr->run();
    }
};

#endif