#pragma once

#include <stdint.h>

typedef struct ss_treap_node {
    struct ss_treap_node* left;
    struct ss_treap_node* right;
    uint32_t priority;
} ss_treap_node;

typedef struct {
    struct ss_treap_node*  root;
    struct ss_treap_prng_state* random_seed;

    int (*compar)(ss_treap_node* a, ss_treap_node* b);

    unsigned n;
    unsigned max_height;

} ss_treap;


void ss_treap_init(ss_treap* treap);
void ss_treap_insert(ss_treap* treap, ss_treap_node* val);
void ss_treap_insert_pri(ss_treap* treap, ss_treap_node* val, uint32_t priority);
struct ss_treap_node* ss_treap_find(ss_treap* treap, ss_treap_node* val);
void ss_treap_delete(ss_treap* treap, ss_treap_node* val);
void ss_treap_update(ss_treap* treap, ss_treap_node* val, uint32_t priority);

void ss_treap_to_dot(ss_treap* t, char* (*tostr)(const ss_treap_node*));
int ss_treap_height(ss_treap* treap);

#define container_of(ptr,type,member) ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))
