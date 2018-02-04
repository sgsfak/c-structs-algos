#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "lch_hmap.h"

typedef struct lch_hmap_entry {
    char* key;
    lch_value_t val;
    uint32_t hash; /* the hash of the key, cached */
    struct lch_hmap_entry* next;
} lch_hmap_entry_t;

typedef struct {
    unsigned int len;
    struct lch_hmap_entry* e;
} lch_hmap_bucket;

typedef uint32_t (*hfn_t)(const char*, size_t);
struct lch_hmap  {
    unsigned int n; /* current number of elements (entries) */
    uint32_t size; /* number of buckets */
    unsigned int max_bucket_size;
    unsigned long long generation;
    hfn_t hfn;
    lch_hmap_bucket* table;  /* buckets */
};

lch_hmap_stats_t ht_stats(lch_hmap_t* h)
{
    lch_hmap_stats_t t = {
        .capacity = h->size,
        .nbr_elems = h->n,
        .max_bucket_size = h->max_bucket_size,
        .generation = h->generation
    };
    return t;
}

#define BITS_TO_HSIZE(b) (1U << (b))
#define HASH_SIZE(ht) ((ht)->size)
#define for_each_lch_bucket(ht,e) \
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
#define lch_fast_mod32(x,N) (((uint64_t) (x) * (uint64_t) (N)) >> 32)

static inline uint32_t mod_hash_size(lch_hmap_t* ht, uint32_t k)
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
                       return lch_fast_mod32(k, ht->size);
    };
}
#define ht_hash_to_bucket(ht,h)  ((ht)->table + (mod_hash_size((ht), (h))))

static uint32_t _next_prime_for_expand(uint32_t minSize)
{
    uint32_t* p;

    for (p=_primes; *p && *p < minSize; ++p);
    return (*p ? *p : _primes[_primes_len - 1]);
}

lch_hmap_t* ht_create(uint32_t initial_size, hfn_t hfn)
{
    lch_hmap_t *h = calloc(1U, sizeof *h);
    if (!h) {
        perror("ht_create");
        return NULL;
    }
    h->size = _next_prime_for_expand(initial_size);
    h->hfn = hfn;
    h->table = calloc(HASH_SIZE(h), sizeof(lch_hmap_bucket));
    if (h->table == NULL) {
        perror("ht_create");
        free(h);
        return NULL;
    }
    /* printf("Hashtable created with size %u\n", HASH_SIZE(h)); */
    return h;
}

static lch_hmap_entry_t* _ht_entry_create(lch_hmap_t* ht, const char* word, uint32_t h)
{
    lch_hmap_entry_t* e = calloc(1, sizeof *e);
    if (!e) {
        perror("_ht_entry_create");
        return NULL;
    }
    e->key = strdup(word);
    e->hash = h;
    return e;
}

static void _ht_entry_destroy(lch_hmap_t* ht, lch_hmap_bucket* he,
        void (*destroy_val_fn)(lch_value_t))
{
    for(lch_hmap_entry_t* t = he->e; t;) {
        lch_hmap_entry_t* tnext = t->next;
        free(t->key);
        if (destroy_val_fn != NULL)
            destroy_val_fn(t->val);
        free(t);
        t = tnext;
    }
    he->e = NULL;
    he->len = 0;
} 

void ht_destroy(lch_hmap_t* ht, void (*destroy_val_fn) (lch_value_t))
{
    lch_hmap_bucket* e;
    for_each_lch_bucket(ht,e) _ht_entry_destroy(ht, e, destroy_val_fn);
    free(ht->table);
    free(ht);
}

void ht_traverse(lch_hmap_t* ht,
        int (*action) (lch_key_t, lch_value_t, void*), void* arg)
{
    unsigned long long generation = ht->generation;
    lch_hmap_bucket* he;
    for_each_lch_bucket(ht,he) {
        for (lch_hmap_entry_t* e = he->e; e; e = e->next) {
            int w = action(e->key, e->val, arg);
            assert(ht->generation == generation);
            if (w < 0)
                return;
        }
    }
}

float ht_load_factor(lch_hmap_t* h)
{
    return h->n*1.0/HASH_SIZE(h);
}

/*
static lch_hmap_bucket* ht_key_to_bucket(lch_hmap_t* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket* b = ht_hash_to_bucket(ht, h);
    return b;
}
*/

static void _ht_insert_entry(lch_hmap_t* ht, lch_hmap_bucket* bucket,
        lch_hmap_entry_t* e);
static void _ht_rehash(lch_hmap_t* ht)
{
    uint32_t newSize = _next_prime_for_expand(2*HASH_SIZE(ht));
    /* printf("Current load factor %4.2f.. (max bkt size=%u) rehashing to %u ..\n", ht_load_factor(ht), ht->max_bucket_size, newSize); */
    lch_hmap_t* hnew = ht_create(newSize, ht->hfn);
    if (!hnew)
        return;
    lch_hmap_bucket* he;
    for_each_lch_bucket(ht,he) {
        for (lch_hmap_entry_t* e = he->e; e;) {
            lch_hmap_entry_t* t = e->next;
            lch_hmap_bucket* b = ht_hash_to_bucket(hnew, e->hash);
            _ht_insert_entry(hnew, b, e);
            e = t;
        }
        he->e = NULL;
    }

    free(ht->table);
    ht->table = hnew->table;
    ht->size = hnew->size;
    ht->max_bucket_size = hnew->max_bucket_size;
    free(hnew);
}
static void _ht_insert_entry(lch_hmap_t* ht, lch_hmap_bucket* bucket, lch_hmap_entry_t* e)
{
    uint32_t size = HASH_SIZE(ht);
    unsigned int n = ht->n;
    if (n + 1 > (3*size >> 2)) { // USe the 0.75 factor
        /* We need to rehash ... */
        _ht_rehash(ht);
    }

    bucket->len++;
    e->next = bucket->e;
    bucket->e = e;
    ht->n++;
    if (bucket->len > ht->max_bucket_size)
        ht->max_bucket_size = bucket->len;
}

void ht_delete(lch_hmap_t* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket* b = ht_hash_to_bucket(ht, h);

    if (b->e == NULL) {
        return;
    }
    lch_hmap_entry_t** e;
    for (e = &b->e; *e; ) {
        lch_hmap_entry_t* t = *e;
        if (h == t->hash && strcmp(t->key, word) == 0) {
            *e = t->next;
            free(t);
            b->len--;
            ht->n--;
            ht->generation--;
            return;
        }
        e = &t->next;
    }
}

lch_value_t* ht_get(lch_hmap_t* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket* b = ht_hash_to_bucket(ht, h);
    for (lch_hmap_entry_t* e = b->e; e; e = e->next) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return &e->val;
        }
    }
    return NULL;
}

lch_value_t* ht_put(lch_hmap_t* ht, const char* word)
{
    ht->generation++;
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket* b = ht_hash_to_bucket(ht, h);

    lch_hmap_entry_t* e;
    for (e = b->e; e; e = e->next) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return &e->val;
        }
    }

    e = _ht_entry_create(ht, word, h);
    if (!e)
        return NULL;
    _ht_insert_entry(ht, b, e);
    return &e->val;
}

bool ht_exists(lch_hmap_t* ht, const char* word)
{
    return ht_get(ht, word) != NULL;
}

