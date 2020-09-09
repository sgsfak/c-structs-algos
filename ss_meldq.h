#ifndef __SS_MELD_QUEUE__
#define __SS_MELD_QUEUE__

#include <stdint.h>
#include <stddef.h>

typedef struct ss_meldq_node {
    struct ss_meldq_node* parent;
    struct ss_meldq_node* left;
    struct ss_meldq_node* right;
} ss_meldq_node;

typedef struct {
    struct ss_meldq_node*  root;
    int (*compar)(ss_meldq_node* a, ss_meldq_node* b);
    uint32_t rand_state[4];
    unsigned int n;
} ss_meldq;

void ss_meldq_init(ss_meldq* h, int (*compar)(ss_meldq_node*, ss_meldq_node*), int rand_seed);
ss_meldq_node* ss_meldq_find_min(ss_meldq* h);
ss_meldq_node* ss_meldq_extract_min(ss_meldq* h);
void ss_meldq_insert(ss_meldq* h, ss_meldq_node* node);
void ss_meldq_delete(ss_meldq* h, ss_meldq_node* node);
void ss_meldq_update_pri(ss_meldq* h, ss_meldq_node* item);

/* Stolen from wikipedia (https://en.wikipedia.org/wiki/Offsetof) :*/
#define container_of(ptr,type,member) ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))

#endif
