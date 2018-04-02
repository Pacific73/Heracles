#ifndef TAP_H
#define TAP_H

#include "helpers.h"
#include <pthread.h>

class Tap {
  private:
    bool enabled;
    bool growth_allowed;
    pthread_mutex_t mutex;

  public:
    Tap() : enabled(false), growth_allowed(false) {
        int res = pthread_mutex_init(&mutex, nullptr);
        if (res != 0) {
            print_err("Tap init failed.");
        }
    }

    bool is_enabled() const { return enabled; }
    bool is_growth_allowed() const { return growth_allowed; }

    void enable_BE() {
        pthread_mutex_lock(&mutex);
        enabled = true;
        pthread_mutex_unlock(&mutex);
    }
    void disable_BE() {
        pthread_mutex_lock(&mutex);
        enabled = false;
        pthread_mutex_unlock(&mutex);
    }
    void enable_growth() {
        pthread_mutex_lock(&mutex);
        growth_allowed = true;
        pthread_mutex_unlock(&mutex);
    }
    void disable_growth() {
        pthread_mutex_lock(&mutex);
        growth_allowed = false;
        pthread_mutex_unlock(&mutex);
    }
};

#endif