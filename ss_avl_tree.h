#pragma once

#include <stdint.h>

typedef struct ss_avl_node {
    struct ss_avl_node* left;
    struct ss_avl_node* right;
    int height;
} ss_avl_node;

typedef struct {
    struct ss_avl_node*  root;
    int (*compar)(ss_avl_node* a, ss_avl_node* b);

    unsigned n;
} ss_avl_tree;


void ss_avl_init(ss_avl_tree* avl, int (*compar)(ss_avl_node* a, ss_avl_node* b));
void ss_avl_insert(ss_avl_tree* avl, ss_avl_node* val);
ss_avl_node* ss_avl_find(ss_avl_tree* avl, ss_avl_node* val);
ss_avl_node* ss_avl_delete(ss_avl_tree* avl, ss_avl_node* val);

#define ss_avl_height(avl) ((avl)->root ? (avl)->root->height : 0)

void ss_avl_to_dot(ss_avl_tree* t, char* (*tostr)(const ss_avl_node*));

#define container_of(ptr,type,member) ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))
