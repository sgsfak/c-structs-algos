#include <stdlib.h>
#include <stdio.h>
#include "ss_pairing_heap.h"

void ss_pairing_init(ss_pairing_heap* h)
{
    h->size = 0;
    h->root = NULL;
}

void* ss_pairing_find_min(ss_pairing_heap* h)
{
    return h->root ? h->root->data : NULL;
}

#define DQ_INS(head, item) ((item)->next_sibling=(head), \
        (item)->prev_sibling=(head)->prev_sibling,       \
        (head)->prev_sibling->next->sibling=(item),      \
        (head)->prev_sibling=(item))                        
#define DQ_DEL(item) ((item)->next_sibling->prev_sibling = (item)->prev_sibling,\
        (item)->prev_sibling->next_sibling = (item)->next_sibling,              \
        (item)->prev_sibling = (item)->next_sibling = (item))

static
ss_pairing_node* merge_nodes(ss_pairing_node* h1, ss_pairing_node* h2)
{
    ss_pairing_node *parent, *child;

    if (h1 == NULL)
        return h2;
    if (h2 == NULL)
        return h1;
    if (h1->pri < h2->pri) {
        parent = h1;
        child = h2;
    }
    else {
        parent = h2;
        child = h1;
    }

    child->next = parent->child;
    if (parent->child) {
        parent->child->prev = child;
    }

    parent->child = child;
    child->prev = parent;

    parent->next = NULL;
    parent->prev = NULL;

    return parent;
}

static
ss_pairing_node* collapse(ss_pairing_node* child)
{
    ss_pairing_node *item, *next, *prev, *last, *parent, *temp;

    /* parent     = child->prev; */
    item       = child;
    prev       = NULL;
    last       = item;
    while(item != NULL) {
        next = item->next;
        if (next != NULL) {
            temp = next->next;
            last = merge_nodes(item, next);
            last->prev = prev;
            prev = last;
            item = temp;
        }
        else {
            last = item;
            last->prev = prev;
            break;
        }
    }
    while (prev != NULL) {
        temp = prev->prev;
        last = merge_nodes(prev, last);
        prev = temp;
    }
    /* last->prev = parent; */
    return last;
}

ss_pairing_node* ss_pairing_insert(ss_pairing_heap* h, void* data, int pri)
{
    ss_pairing_node* node;

    node = malloc(sizeof *node);
    if (!node) {
        exit(EXIT_FAILURE);
    }
    node->data  = data;
    node->pri   = pri;
    node->child = NULL;
    node->next  = NULL;
    node->prev  = NULL;
    h->root     = merge_nodes(h->root, node);
    h->size++;
    return node;
}

ss_pairing_heap* ss_pairing_merge(ss_pairing_heap* h1, ss_pairing_heap* h2)
{
    ss_pairing_node* root;
    unsigned int size = h1->size + h2->size;

    root      = merge_nodes(h1->root, h2->root);
    h1->root  = root;
    h1->size = size;
    h2->root  = NULL;
    h2->size = 0;
    return h1;
}

void ss_pairing_delete(ss_pairing_heap* h, ss_pairing_node* item)
{
    if (h->root == item) {
        h->root = collapse(item->child);
    }
    else {
        if (item->prev == item->prev->child)
            item->prev->child = item->next;
        else
            item->prev->next = item->next;
        if (item->next)
            item->next->prev = item->prev;

        h->root = merge_nodes(h->root, collapse(item->child));
    }
    free(item);
    h->size--;
}

void ss_pairing_decrease_pri(ss_pairing_heap* h, ss_pairing_node* item, int new_pri)
{
    item->pri = new_pri;
    if (h->root == item)
        return;

    if (item->prev == item->prev->child)
        item->prev->child = item->next;
    else
        item->prev->next = item->next;
    if (item->next)
        item->next->prev = item->prev;
    item->next = NULL;
    item->prev = NULL;

    h->root = merge_nodes(h->root, item);
}

void* ss_pairing_extract_min(ss_pairing_heap* h)
{
    void* data = ss_pairing_find_min(h);
    ss_pairing_delete(h, h->root); /* XXX */
    return data;
}
