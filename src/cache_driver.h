#ifndef CACHE_DRIVER_H
#define CACHE_DRIVER_H

class CacheDriver {
private:
    uint64_t LC_mask;
    uint64_t BE_mask;
    uint64_t sys_mask;

    size_t LC_bits;
    size_t BE_bits;
    size_t sys_bits;
    size_t min_bits;

    void init_masks(); // only LC task running

    bool update_allocation();

public:
    CacheDriver();

    bool intel_init();
    bool intel_fini();

    bool update_association(size_t BE_core_num, size_t sys_core_num, size_t total_core_num);

    bool new_BE_task
    bool BE_cache_grow();
    bool BE_cache_roll_back();

    void clear();
};

#endif