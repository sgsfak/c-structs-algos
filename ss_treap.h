#pragma once

#include <stdint.h>

typedef struct ss_treap_node {
    struct ss_treap_node* left;
    struct ss_treap_node* right;
    void* val;
    uint32_t priority;
} ss_treap_node;

typedef struct {
    struct ss_treap_node*  root;
    struct ss_treap_prng_state* random_seed;

    int (*compar)(const void* a, const void* b);

    unsigned n;
    unsigned max_height;

} ss_treap;


void ss_treap_init(ss_treap* treap);
void ss_treap_destroy(ss_treap* treap);

void ss_treap_insert(ss_treap* treap, void* val);
void ss_treap_insert_pri(ss_treap* treap, void* val, uint32_t priority);
struct ss_treap_node* ss_treap_find(ss_treap* treap, const void* val);
void ss_treap_delete(ss_treap* treap, void* val);
void ss_treap_update(ss_treap* treap, void* val,
        void* (*update_cb)(const void* old_val, const void* val));

void ss_treap_to_dot(ss_treap* t, char* (*tostr)(void*));
