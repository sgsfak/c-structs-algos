#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    /*
     * the type of the keys : C strings
     */
    typedef char* lch_key_t;
    /* 
     * The Value associated with each key.
     * It's a union for "type punning"
     */
    typedef union {
        long l;
        double d;
        void* p;
    } lch_value_t;

    typedef struct {
        unsigned int nbr_elems;
        unsigned int capacity;
        unsigned int max_bucket_size;
        unsigned long long generation;
    } lch_hmap_stats_t;

    typedef struct lch_hmap lch_hmap_t;
    /*
     * Creates a new chained hashmap, with the initial_size given
     * and the provided hash function
     */
    lch_hmap_t* ht_create(uint32_t initial_size, 
            uint32_t (*hfn_t)(const char*, size_t));

    /*
     * Returns the current "load factor" of the hashmap
     */
    float ht_load_factor(lch_hmap_t* h);
    lch_hmap_stats_t ht_stats(lch_hmap_t* h);


    void ht_delete(lch_hmap_t* ht, const char* word);

    /*
     * Finds the entry with the given key.
     * Returns NULL if not found
     */
    lch_value_t* ht_get(lch_hmap_t* ht, const char* word);

    /*
     * Inserts a new key in the hashmap and returns the 
     * inserted lch_value_t. If the key exists already
     * it returns the existing value
     */
    lch_value_t* ht_put(lch_hmap_t* ht, const char* word);

    /*
     * Checks is the given key exists in the hashmap
     */
    bool ht_exists(lch_hmap_t* ht, const char* word);

    void ht_traverse(lch_hmap_t* ht, 
            int (*action) (lch_key_t, lch_value_t, void*), void* arg);

    /*
     * Destroys/deallocates the hashmap. If destroy_val_fn is not NULL
     * it is called for each lch_value_t in the hashmap
     */
    void ht_destroy(lch_hmap_t* ht, 
            void (*destroy_val_fn) (lch_value_t)) ;

#ifdef __cplusplus
}
#endif
