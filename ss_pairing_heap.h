#pragma once

#include <stddef.h>

/*
 * An intrusive min Pairing heap!
 */
typedef struct ss_pairing_node {
    struct ss_pairing_node* child;
    struct ss_pairing_node* prev;
    struct ss_pairing_node* next;
} ss_pairing_node;

typedef struct {
    ss_pairing_node* root;
    int (*compar)(ss_pairing_node *, ss_pairing_node *);
    unsigned int size;
} ss_pairing_heap;

void ss_pairing_init(ss_pairing_heap* h, int (*compar)(ss_pairing_node*, ss_pairing_node*));
ss_pairing_node* ss_pairing_find_min(ss_pairing_heap* h);
ss_pairing_node* ss_pairing_extract_min(ss_pairing_heap* h);
void ss_pairing_insert(ss_pairing_heap* h, ss_pairing_node* node);
void ss_pairing_delete(ss_pairing_heap* h, ss_pairing_node* item);
void ss_pairing_update_item(ss_pairing_heap* h, ss_pairing_node* item);
ss_pairing_heap* ss_pairing_merge(ss_pairing_heap* h1, ss_pairing_heap* h2);

void ss_pairing_to_dot(ss_pairing_heap* h, char* (*tostr)(const ss_pairing_node*));

/* Stolen from wikipedia (https://en.wikipedia.org/wiki/Offsetof) :*/
#define container_of(ptr,type,member) ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))
