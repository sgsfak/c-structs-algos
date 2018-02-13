#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

    typedef struct lch_hmap hset_t;

    hset_t* hs_create(uint32_t initialCapasity);
    void hs_add(hset_t* h, const char* key);
    void hs_remove(hset_t* h, const char* key);
    size_t hs_size(hset_t* h);
    void hs_destroy(hset_t* h);

#ifdef __cplusplus
}
#endif
