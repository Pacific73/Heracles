#ifndef CACHE_DRIVER_H
#define CACHE_DRIVER_H

#include <stdint.h>
#include <cstring>

class CacheDriver {
private:
    uint64_t LC_mask;
    uint64_t BE_mask;
    uint64_t sys_mask;

    size_t LC_bits;
    size_t BE_bits;
    size_t sys_bits;

    uint32_t min_bits;

    bool init_env();
    bool init_masks(); // only LC task running

    bool update_allocation();
    bool update_masks();

public:
    CacheDriver();

    bool update_association(size_t BE_core_num, size_t sys_core_num, size_t total_core_num);

    bool BE_cache_grow();
    bool BE_cache_roll_back();

    void clear();
};

#endif