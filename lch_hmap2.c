#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "lch_hmap.h"

typedef struct {
    char* key;
    lch_value_t val;
    uint32_t hash; /* the hash of the key, cached */
} lch_hmap_entry_t;

#define LCH_BIN_SIZE 5
typedef struct lch_hmap_bucket {
    struct lch_hmap_bucket* next;
    unsigned int len; /* how many of the LCH_BIN_SIZE are used */
    lch_hmap_entry_t entries[LCH_BIN_SIZE];
} lch_hmap_bucket_t;

#define lch_bucket_empty(b) ((b)->len == 0 && (b)->next == NULL) 
#define lch_bucket_full(b) ((b)->len == LCH_BIN_SIZE) 

typedef uint32_t (*hfn_t)(const char*, size_t);
struct lch_hmap  {
    unsigned int n; /* current number of elements (entries) */
    uint32_t size; /* number of buckets */
    unsigned int max_bucket_size;
    unsigned long long generation;
    hfn_t hfn;
    lch_hmap_bucket_t** table;  /* pointers to buckets */
};

#define for_each_lch_bucket(ht,bkt) \
    for(bkt=(ht)->table;bkt!=(ht)->table+HASH_SIZE(ht);bkt++)

#define for_each_lch_bucket_entry(bkt, e) \
      for(lch_hmap_bucket_t* __bkt2=(bkt); __bkt2; __bkt2=__bkt2->next)\
        for(e=__bkt2->entries;e!=__bkt2->entries+__bkt2->len;++e)

#define for_each_lch_entry(h, bkt, e) \
    for_each_lch_bucket(ht,bkt)\
        for_each_lch_bucket_entry(bkt,e)

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

#define HASH_SIZE(ht) ((ht)->size)

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
#define ht_hash_to_bucket(ht,h)  ((ht)->table[mod_hash_size((ht), (h))])

static inline uint32_t _next_prime_for_expand(uint32_t minSize)
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
    h->table = calloc(h->size, sizeof *h->table);
    if (h->table == NULL) {
        perror("ht_create");
        free(h);
        return NULL;
    }
    /* printf("Hashtable created with size %u\n", HASH_SIZE(h)); */
    return h;
}

static inline void _ht_destroy_entries(lch_hmap_bucket_t* bkt,
        void (*destroy_val_fn) (lch_value_t))
{
    while(bkt) {
        for(lch_hmap_entry_t* e=bkt->entries; e!=bkt->entries+bkt->len; ++e) {
            if (destroy_val_fn != NULL)
                destroy_val_fn(e->val);
            free(e->key);
            e->key = NULL;
        }
        bkt->len = 0;
        bkt = bkt->next;
    }
}

void ht_destroy(lch_hmap_t* ht, void (*destroy_val_fn) (lch_value_t))
{
    for (uint32_t i=0; i<ht->size; ++i) {
        for(lch_hmap_bucket_t* bkt = ht->table[i]; bkt;) {
            _ht_destroy_entries(bkt, destroy_val_fn);
            lch_hmap_bucket_t* bnext = bkt->next;
            free(bkt);
            bkt = bnext;
        }
        ht->table[i] = NULL;
    }

    free(ht->table);
    free(ht);
}

void ht_traverse(lch_hmap_t* ht,
        int (*action) (lch_key_t, lch_value_t, void*), void* arg)
{
    unsigned long long generation = ht->generation;
    lch_hmap_entry_t* e;
    for (uint32_t i=0; i<ht->size; ++i) {
        for(lch_hmap_bucket_t* bkt = ht->table[i]; bkt; bkt = bkt->next) {
            for_each_lch_bucket_entry(bkt, e) {
                int w = action(e->key, e->val, arg);
                assert(ht->generation == generation);
                if (w < 0)
                    return;
            }
        }
    }
}

void ht_traverse_ordered(lch_hmap_t* ht,
        int (*action) (lch_key_t, lch_value_t, void*), void* arg) {
  /* XXX : not implemented */
  ht_traverse(ht, action, arg);

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

static inline lch_value_t* _ht_insert_entry(lch_hmap_bucket_t* b, uint32_t hash, char* word)
{
    /*
     * There are two cases:
     *
     * a) either the bucket element in the hashtable has
     *  empty space left and therefore we can add the new 
     *  entry here, or
     * 
     * b) there is no space left, which means that neither of
     *  subsequent buckets in the chain has space either, so we care a
     *  new bucket, transfer the entries of the bucket there, and then add
     *  the new element in the bucket.
     */

    if (lch_bucket_full(b)) {
        lch_hmap_bucket_t* new_bkt = malloc(sizeof *new_bkt);
        if (new_bkt == NULL) {
            perror("malloc");
            return NULL;
        }
        *new_bkt = *b;

        b->next = new_bkt;
        b->len = 0;
    }
    lch_hmap_entry_t* e = b->entries + b->len;
    e->key = word;
    e->hash = hash;
    b->len++;
    return &e->val;
}

static void _ht_rehash(lch_hmap_t* ht)
{
    uint32_t newSize = _next_prime_for_expand(2*HASH_SIZE(ht));
    /* printf("Current load factor %4.2f.. (max bkt size=%u) rehashing to %u ..\n", ht_load_factor(ht), ht->max_bucket_size, newSize); */
    lch_hmap_t* hnew = ht_create(newSize, ht->hfn);
    if (!hnew) {
        perror("_ht_rehash");
        return;
    }
    for (size_t i=0; i<ht->size; ++i) {
        lch_hmap_bucket_t* b = ht->table[i];
        lch_hmap_entry_t* e;
        for_each_lch_bucket_entry(b, e) {
            uint32_t idx = mod_hash_size(hnew, e->hash);
            lch_hmap_bucket_t* new_bkt = hnew->table[idx];
            if (new_bkt == NULL) {
                new_bkt = calloc(1, sizeof *new_bkt);
                if (new_bkt == NULL) {
                    perror("_ht_rehash");
                    goto rehash_end;
                }
                hnew->table[idx] = new_bkt;
            }
            lch_value_t* v = _ht_insert_entry(new_bkt, e->hash, e->key);
            if (v == NULL)
                /* XXX: well, here we can have done the half rehashes... */
                goto rehash_end;
            e->key = NULL;
        }

        lch_hmap_bucket_t* bkt=b->next;
        while(bkt) {
            lch_hmap_bucket_t* bnext = bkt->next;
            free(bkt);
            bkt = bnext;
        }
        b->next = NULL;
    }

    /* adopt the new hash table */
    lch_hmap_bucket_t** new_table = hnew->table;
    hnew->table = ht->table;
    ht->table = new_table;

    ht->size = hnew->size;
    ht->max_bucket_size = hnew->max_bucket_size;
rehash_end:
    /* ht_destroy(hnew, NULL); */
    free(hnew->table);
}

void ht_delete(lch_hmap_t* ht, const char* word)
{
    ht->generation++;
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket_t* b = ht_hash_to_bucket(ht, h);
    if (b == NULL)
        return;

    /* XXX: This is not so correct yet!!! */
    lch_hmap_entry_t* e;
    for(lch_hmap_bucket_t* bkt=b; bkt; bkt=bkt->next) {
        for(e=bkt->entries;e!=bkt->entries+bkt->len;++e) {
            if (h == e->hash && strcmp(e->key, word) == 0) {
                /* Found it!
                 * Swap it with the last element in the b bucket
                 */
                lch_hmap_entry_t temp = b->entries[b->len - 1];
                temp = *e;
                *e = temp;
                b->len--;
                free(temp.key);
                /* return &temp->val; */
                return;
            }
        }
    }
}

lch_value_t* ht_get(lch_hmap_t* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    lch_hmap_bucket_t* b = ht_hash_to_bucket(ht, h);
    if (b == NULL)
        return NULL;
    lch_hmap_entry_t* e;
    for_each_lch_bucket_entry(b,e) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return &e->val;
        }
    }
    return NULL;
}


lch_value_t* ht_put(lch_hmap_t* ht, const char* word)
{
    if (ht->n + 1 > (3*ht->size >> 2)) { // USe the 0.75 factor
        // We need to rehash ...
        _ht_rehash(ht);
    }
    ht->generation++;
    uint32_t h = ht->hfn(word, strlen(word));
    uint32_t idx = mod_hash_size(ht, h);
    lch_hmap_bucket_t* b = ht->table[idx];
    if (b == NULL) {
        b = calloc(1U, sizeof (lch_hmap_bucket_t));
        if (b == NULL) {
            perror("ht_put");
            return NULL;
        }
        ht->table[idx] = b;
    }

    /* First search to see if the new item exists
     * already in the hash map
     */
    lch_hmap_entry_t* e;
    for_each_lch_bucket_entry(b,e) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return &e->val;
        }
    }

    char* key = strdup(word);
    if (key == NULL) {
        perror("strdup");
        return NULL;
    }
    lch_value_t* val = _ht_insert_entry(b, h, key);
    if (val) {
        ht->n++;
    }
    return val;
}

bool ht_contains(lch_hmap_t* ht, const char* word)
{
    return ht_get(ht, word) != NULL;
}

