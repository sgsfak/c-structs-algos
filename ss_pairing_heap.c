#include <stdlib.h>
#include <stdio.h>
#include "ss_pairing_heap.h"

static
void panic(const char* msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

void ss_pairing_init(ss_pairing_heap* h)
{
    h->size = 0;
    h->root = NULL;
}

void* ss_pairing_find_min(ss_pairing_heap* h)
{
    return h->root ? h->root->data : NULL;
}

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
    if (parent->child != NULL) {
        parent->child->prev = child;
    }

    parent->child = child;
    child->prev = parent;

    parent->next = NULL;
    parent->prev = NULL;

    return parent;
}
static
ss_pairing_node* link(ss_pairing_node* head)
{
    ss_pairing_node *current, *next;

    /* 
    1st pass:

    We start from left, i.e. from 'head' and merge pairs of subsequent nodes. 
    The result will be saved as a new 'head', effectively putting the nodes
    in reverse order (as in stack) so that the 2nd pass visits them as if from
    the tail!
    */
    current = head;
    head = NULL;
    while(current != NULL) {
        next = current->next != NULL ? current->next->next : NULL;
        current = merge_nodes(current, current->next);

        current->next = head;
        head = current;
        current = next;
    }

    /*
     2nd pass: merge each head with its next and store the result 
     as a new head..
    */
    while (head != NULL && head->next != NULL) {
        next = head->next->next;
        head = merge_nodes(head, head->next);
        head->next = next;
    }
    return head;
}

ss_pairing_node* ss_pairing_insert_node(ss_pairing_heap* h, ss_pairing_node* node)
{
    h->root = merge_nodes(h->root, node);
    h->size++;
    return node;
}

ss_pairing_node* ss_pairing_insert(ss_pairing_heap* h, void* data, int pri)
{

    ss_pairing_node* node;

    node = malloc(sizeof *node);
    if (node == NULL) {
        panic("malloc returned NULL. Out of memory?");
    }
    SS_PAIRING_NODE_INIT(node,pri,data);
    ss_pairing_insert_node(h, node);
    return node;
}

ss_pairing_heap* ss_pairing_merge(ss_pairing_heap* h1, ss_pairing_heap* h2)
{
    ss_pairing_node* root;
    unsigned int size = h1->size + h2->size;

    root     = merge_nodes(h1->root, h2->root);
    h1->root = root;
    h1->size = size;
    h2->root = NULL;
    h2->size = 0;
    return h1;
}

void ss_pairing_delete(ss_pairing_heap* h, ss_pairing_node* item)
{
    if (h->root == item) {
        h->root = link(item->child);
    }
    else {
        if (item->prev == item->prev->child)
            item->prev->child = item->next;
        else
            item->prev->next = item->next;
        if (item->next != NULL)
            item->next->prev = item->prev;

        h->root = merge_nodes(h->root, link(item->child));
    }
    free(item);
    h->size--;
}

void ss_pairing_decrease_pri(ss_pairing_heap* h, ss_pairing_node* item, int new_pri)
{
    if (new_pri < item->pri) {
        item->pri = new_pri;
        if (h->root == item)
            return;

        if (item->prev == item->prev->child)
            item->prev->child = item->next;
        else
            item->prev->next = item->next;
        if (item->next != NULL)
            item->next->prev = item->prev;
        item->next = NULL;
        item->prev = NULL;

        h->root = merge_nodes(h->root, item);
    }
    else {
        
    }
}

void* ss_pairing_extract_min(ss_pairing_heap* h)
{
    void* data = ss_pairing_find_min(h);
    ss_pairing_delete(h, h->root);
    return data;
}
