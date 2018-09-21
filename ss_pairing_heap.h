#pragma once

typedef struct ss_pairing_node {
    struct ss_pairing_node* child;
    struct ss_pairing_node* prev;
    struct ss_pairing_node* next;
    void* data;
    int pri;
} ss_pairing_node;

typedef struct {
    ss_pairing_node* root;
    unsigned int size;
} ss_pairing_heap;

#define SS_PAIRING_NODE_INIT(n,p,v) ((n)->prev=(n)->next=(n)->child=NULL,(n)->pri=(p),(n)->data=v)

void ss_pairing_init(ss_pairing_heap* h);
void* ss_pairing_find_min(ss_pairing_heap* h);
void* ss_pairing_extract_min(ss_pairing_heap* h);
ss_pairing_node* ss_pairing_insert(ss_pairing_heap* h, void* data, int pri);
ss_pairing_node* ss_pairing_insert_node(ss_pairing_heap* h, ss_pairing_node* node);
void ss_pairing_delete(ss_pairing_heap* h, ss_pairing_node* item);
void ss_pairing_decrease_pri(ss_pairing_heap* h, ss_pairing_node* item, int decr);
ss_pairing_heap* ss_pairing_merge(ss_pairing_heap* h1, ss_pairing_heap* h2);
