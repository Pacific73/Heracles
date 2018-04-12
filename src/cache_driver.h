#ifndef CACHE_DRIVER_H
#define CACHE_DRIVER_H

class CacheDriver {
private:

public:
    CacheDriver();
    
    bool BE_cache_grow();
    bool BE_cache_roll_back()
};

#endif