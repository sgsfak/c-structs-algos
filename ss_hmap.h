#ifndef __SS_HMAP_H__
#define __SS_HMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

    /* 
     * The Value associated with each key.
     * It's a union for "type punning"
     */
    typedef struct ss_hmap_entry {
        struct ss_hmap_entry* next;
        uint32_t hashcode;
    } ss_hmap_entry_t;


    typedef struct ss_hmap_bucket {
        ss_hmap_entry_t* e;
    } ss_hmap_bucket_t;

    typedef struct ss_hmap  {
        ss_hmap_bucket_t* table;  /* buckets */
        uint32_t size; /* number of buckets */
        unsigned int n; /* current number of elements (entries) */
        int (*compar)(void* e1, void* e2);
        size_t offset; 
    } ss_hmap_t;

    /*
     * Creates a new chained hashmap, with the initial_size given
     * and the provided hash function
     */
    ss_hmap_t* ht_init(ss_hmap_t* h,
            uint32_t initial_size, 
            int (*compar)(void* e1, void* e2),
            size_t offset);

    /*
     * Returns the current "load factor" of the hashmap
     */
    float ht_load_factor(ss_hmap_t* h);


    void ht_delete(ss_hmap_t* ht, ss_hmap_entry_t* entry);

    /*
     * Finds the entry with the given key.
     * Returns NULL if not found
     */
    void* ht_get(ss_hmap_t* ht, ss_hmap_entry_t* entry);

    /*
     * Inserts a new key in the hashmap and returns the 
     * inserted ss_value_t. If the key exists already
     * it returns the existing value
     */
    void* ht_put(ss_hmap_t* ht, ss_hmap_entry_t* entry);

    /*
     * Checks if the given key is contained in the hashmap
     */
    bool ht_contains(ss_hmap_t* ht, ss_hmap_entry_t* entry);
/*
    void ht_traverse(ss_hmap_t* ht, 
            int (*action) (ss_key_t, ss_value_t, void*), void* arg);

    void ht_traverse_ordered(ss_hmap_t* ht, 
            int (*action) (ss_key_t, ss_value_t, void*), void* arg);

    void ht_clear(ss_hmap_t* ht, 
        void (*destroy_val_fn) (ss_value_t));
*/
    /*
     * frees the memory used internally by the hash map. The caller
     * is responsible for freeing the pointer to the map itself.
     */
    void ht_clear(ss_hmap_t* ht) ;

#ifdef __cplusplus
}
#endif

#endif