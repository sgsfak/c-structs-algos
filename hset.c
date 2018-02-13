#include <stddef.h>
#include "hset.h"
#include "lch_hmap.h"
#include "hfn.h"


hset_t* hs_create(uint32_t initialCapasity)
{
    return ht_create(initialCapasity, h31_hash);
}

void hs_add(hset_t* h, const char* key)
{
    ht_put(h, key);
}
void hs_remove(hset_t* h, const char* key)
{
    ht_delete(h, key);
}
size_t hs_size(hset_t* h)
{
    return ht_stats(h).nbr_elems;
}
void hs_destroy(hset_t* h)
{
    ht_destroy(h, NULL);
}

