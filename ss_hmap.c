#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "ss_hmap.h"

#define BITS_TO_HSIZE(b) (1U << (b))
#define HASH_SIZE(ht) ((ht)->size)
#define for_each_ss_bucket(ht,e) \
    for(e=(ht)->table;e!=(ht)->table+HASH_SIZE(ht);e++)

#define _HSH_P0 5U
#define _HSH_P1 11U
#define _HSH_P2 23U
#define _HSH_P3 47U
#define _HSH_P4 97U
#define _HSH_P5 199U
#define _HSH_P6 409U
#define _HSH_P7 823U
#define _HSH_P8 1741U
#define _HSH_P9 3469U
#define _HSH_P10 6949U
#define _HSH_P11 14033U
#define _HSH_P12 28411U
#define _HSH_P13 57557U
#define _HSH_P14 116731U
#define _HSH_P15 236897U
#define _HSH_P16 480881U
#define _HSH_P17 976369U
#define _HSH_P18 1982627U
#define _HSH_P19 4026031U
#define _HSH_P20 8175383U
#define _HSH_P21 16601593U
#define _HSH_P22 33712729U
#define _HSH_P23 68460391U
#define _HSH_P24 139022417U
#define _HSH_P25 282312799U
#define _HSH_P26 573292817U
#define _HSH_P27 1164186217U
#define _HSH_P28 2364114217U
#define _HSH_P29 4294967291U

static uint32_t _primes[] = {
    _HSH_P0,
    _HSH_P1,
    _HSH_P2,
    _HSH_P3,
    _HSH_P4,
    _HSH_P5,
    _HSH_P6,
    _HSH_P7,
    _HSH_P8,
    _HSH_P9,
    _HSH_P10,
    _HSH_P11,
    _HSH_P12,
    _HSH_P13,
    _HSH_P14,
    _HSH_P15,
    _HSH_P16,
    _HSH_P17,
    _HSH_P18,
    _HSH_P19,
    _HSH_P20,
    _HSH_P21,
    _HSH_P22,
    _HSH_P23,
    _HSH_P24,
    _HSH_P25,
    _HSH_P26,
    _HSH_P27,
    _HSH_P28,
    _HSH_P29,
    0U
};

#define _primes_len (sizeof(_primes)/sizeof(_primes[0])-1)

/* See "A fast alternative to the modulo reduction":
 * lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
 */
#define ss_fast_mod32(x,N) (((uint64_t) (x) * (uint64_t) (N)) >> 32)

static uint32_t mod_hash_size(ss_hmap_t* ht, uint32_t k)
{
    switch (ht->size) {
        case _HSH_P0:  return k % _HSH_P0; 
        case _HSH_P1:  return k % _HSH_P1; 
        case _HSH_P2:  return k % _HSH_P2; 
        case _HSH_P3:  return k % _HSH_P3; 
        case _HSH_P4:  return k % _HSH_P4; 
        case _HSH_P5:  return k % _HSH_P5; 
        case _HSH_P6:  return k % _HSH_P6; 
        case _HSH_P7:  return k % _HSH_P7; 
        case _HSH_P8:  return k % _HSH_P8; 
        case _HSH_P9:  return k % _HSH_P9; 
        case _HSH_P10: return k %_HSH_P10; 
        case _HSH_P11: return k %_HSH_P11; 
        case _HSH_P12: return k %_HSH_P12; 
        case _HSH_P13: return k %_HSH_P13; 
        case _HSH_P14: return k %_HSH_P14; 
        case _HSH_P15: return k %_HSH_P15; 
        case _HSH_P16: return k %_HSH_P16; 
        case _HSH_P17: return k %_HSH_P17; 
        case _HSH_P18: return k %_HSH_P18; 
        case _HSH_P19: return k %_HSH_P19; 
        case _HSH_P20: return k %_HSH_P20; 
        case _HSH_P21: return k %_HSH_P21; 
        case _HSH_P22: return k %_HSH_P22; 
        case _HSH_P23: return k %_HSH_P23; 
        case _HSH_P24: return k %_HSH_P24; 
        case _HSH_P25: return k %_HSH_P25; 
        case _HSH_P26: return k %_HSH_P26; 
        case _HSH_P27: return k %_HSH_P27; 
        case _HSH_P28: return k %_HSH_P28; 
        case _HSH_P29: return k %_HSH_P29; 
        default:
                       return ss_fast_mod32(k, ht->size);
    };
}
#define ht_hash_to_bucket(ht,h)  ((ht)->table + (mod_hash_size((ht), (h))))

#define SS_HMAP_ENTRY_VALUE(ht,e) ((void*)((char*)(e) - (ht)->offset))

static uint32_t _next_prime_for_expand(uint32_t minSize)
{
    uint32_t* p;

    for (p=_primes; *p && *p < minSize; ++p);
    return (*p ? *p : _primes[_primes_len - 1]);
}

ss_hmap_t* ht_init(ss_hmap_t* h,
                   uint32_t initial_size, 
                   int (*fn)(void* e1, void* e2),
                   size_t offset)
{
    h->n = 0;
    h->size = _next_prime_for_expand(initial_size);
    h->offset = 0;
    h->compar = fn;
    h->table = calloc(h->size, sizeof(ss_hmap_bucket_t));
    if (h->table == NULL) {
        perror("ht_create");
        return NULL;
    }
    /* printf("Hashtable created with size %u\n", HASH_SIZE(h)); */
    return h;
}


void ht_clear(ss_hmap_t* ht)
{
    free(ht->table);
    ht->table = NULL;
    ht->size = 0;
    ht->n = 0;
}


float ht_load_factor(ss_hmap_t* h)
{
    return h->n*1.0/HASH_SIZE(h);
}

#define _ht_insert_entry(ht,bucket,entry) ((entry)->next = (bucket)->e,(bucket)->e = (entry),(ht)->n++)

static void _ht_rehash(ss_hmap_t* ht)
{
    uint32_t newSize = _next_prime_for_expand(2*HASH_SIZE(ht)+1);
    // printf("Current load factor %4.2f.. (size=%u, N=%u, max bkt size=%u) rehashing to %u ..\n", ht_load_factor(ht), ht->size, ht->n, ht->max_bucket_size, newSize);
    ss_hmap_t hnew = {};
    hnew.size = newSize;
    hnew.table = calloc(newSize, sizeof(ss_hmap_bucket_t));
    if (!hnew.table)
        return;
    ss_hmap_bucket_t* he;
    for_each_ss_bucket(ht,he) {
        ss_hmap_entry_t* t;
        for (ss_hmap_entry_t* e = he->e; e; e = t) {
            t = e->next;
            ss_hmap_bucket_t* b = ht_hash_to_bucket(&hnew, e->hashcode);
            _ht_insert_entry(&hnew, b, e);
        }
        he->e = NULL;
    }

    free(ht->table);
    ht->table = hnew.table;
    ht->size = newSize;
}

void ht_delete(ss_hmap_t* ht, ss_hmap_entry_t* entry)
{
    uint32_t h = entry->hashcode;
    void* key = SS_HMAP_ENTRY_VALUE(ht, entry);
    ss_hmap_bucket_t* b = ht_hash_to_bucket(ht, h);

    if (b->e == NULL) {
        return;
    }
    ss_hmap_entry_t** e;
    for (e = &b->e; *e; ) {
        ss_hmap_entry_t* t = *e;
        if (h == t->hashcode && ht->compar(SS_HMAP_ENTRY_VALUE(ht, t), key) == 0) {
            *e = t->next;

            ht->n--;
            return;
        }
        e = &t->next;
    }
}

void* ht_get(ss_hmap_t* ht, ss_hmap_entry_t* entry)
{
    uint32_t h = entry->hashcode;
    void* key = SS_HMAP_ENTRY_VALUE(ht, entry);
    ss_hmap_bucket_t* b = ht_hash_to_bucket(ht, h);
    for (ss_hmap_entry_t* e = b->e; e; e = e->next) {
        if (h == e->hashcode && ht->compar(key, SS_HMAP_ENTRY_VALUE(ht, e)) == 0) {
            return SS_HMAP_ENTRY_VALUE(ht,e);
        }
    }
    return NULL;
}

void* ht_put(ss_hmap_t* ht, ss_hmap_entry_t* entry)
{
    uint32_t h = entry->hashcode;
    void* key = SS_HMAP_ENTRY_VALUE(ht, entry);
    ss_hmap_bucket_t* b = ht_hash_to_bucket(ht, h);

    ss_hmap_entry_t* e;
    for (e = b->e; e; e = e->next) {
        if (h == e->hashcode && ht->compar(key, SS_HMAP_ENTRY_VALUE(ht, e)) == 0) {
            return SS_HMAP_ENTRY_VALUE(ht,e);
        }
    }

    if (ht->n + 1 > (3*ht->size >> 2)) { /* Use the 0.75 factor */
        /* We need to rehash ... */
        _ht_rehash(ht);
        b = ht_hash_to_bucket(ht, h);
    }

    _ht_insert_entry(ht, b, entry);
    return SS_HMAP_ENTRY_VALUE(ht,entry);
}


