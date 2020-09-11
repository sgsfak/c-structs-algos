#include <stdlib.h>
#include "ss_pairing_heap.h"

void ss_pairing_init(ss_pairing_heap* h, int (*compar)(ss_pairing_node*, ss_pairing_node*))
{
    h->size = 0;
    h->root = NULL;
    h->compar = compar;
}

ss_pairing_node* ss_pairing_find_min(ss_pairing_heap* h)
{
    return h->root;
}

static
ss_pairing_node* merge_nodes(ss_pairing_heap* h, ss_pairing_node* h1, ss_pairing_node* h2)
{
    ss_pairing_node *parent, *child;

    if (h1 == NULL)
        return h2;
    if (h2 == NULL)
        return h1;
    if (h->compar(h1, h2) < 0) {
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
ss_pairing_node* link(ss_pairing_heap* h, ss_pairing_node* head)
{
    ss_pairing_node *current, *next, *prev;

    if (head == NULL)
        return NULL;

    prev = head->prev;
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
        current = merge_nodes(h, current, current->next);

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
        head = merge_nodes(h, head, head->next);
        head->next = next;
    }
    head->prev = prev;
    return head;
}

void ss_pairing_insert(ss_pairing_heap* h, ss_pairing_node* node)
{
    node->prev  = NULL;
    node->next  = NULL;
    node->child = NULL;
    h->root     = merge_nodes(h, h->root, node);
    h->size++;
}

ss_pairing_heap* ss_pairing_merge(ss_pairing_heap* h1, ss_pairing_heap* h2)
{
    ss_pairing_node* root;
    unsigned int size = h1->size + h2->size;

    root     = merge_nodes(h1, h1->root, h2->root);
    h1->root = root;
    h1->size = size;
    h2->root = NULL;
    h2->size = 0;
    return h1;
}

void ss_pairing_delete(ss_pairing_heap* h, ss_pairing_node* item)
{
    if (h->root == item) {
        h->root = link(h, item->child);
    }
    else {
        if (item == item->prev->child)
            item->prev->child = item->next;
        else
            item->prev->next = item->next;
        if (item->next != NULL)
            item->next->prev = item->prev;

        h->root = merge_nodes(h, h->root, link(h, item->child));
    }
    item->prev = NULL;
    item->next = NULL;
    h->size--;
}

void ss_pairing_update_item(ss_pairing_heap* h, ss_pairing_node* item)
{
    ss_pairing_delete(h, item);
    ss_pairing_insert(h, item);
}

ss_pairing_node* ss_pairing_extract_min(ss_pairing_heap* h)
{
    ss_pairing_node* node = h->root;
    ss_pairing_delete(h, node);
    return node;
}

#include <stdio.h>

static
void ss_pairing_node_to_dot(ss_pairing_node* p, char* (*tostr)(const ss_pairing_node*))
{

    char label[400];
    // snprintf(label, 400, "{<name> %s | <child> } | <next>}", tostr ? tostr(p) : "");

    // printf("n%p [label=\"%s\"];\n", p, label);

    snprintf(label, 400,
            "<TABLE BORDER='0' CELLBORDER='1' CELLSPACING='0'><TR>"
            " <TD PORT='name'>%s</TD>"
            " <TD PORT='next'></TD>"
            "</TR></TABLE>", tostr ? tostr(p) : "");
    printf("n%p [label=<%s>];\n", p, label);

    if (p->child) {
        printf("\"n%p\":child -> \"n%p\":name;\n", p, p->child);
        printf("{rank=same");
        for (ss_pairing_node *c = p->child; c; c = c->next) {
            printf(";\"n%p\"", c);
        }
        printf("};\n");
    }
    for (ss_pairing_node *c = p->child; c && c->next; c = c->next) {
        printf("\"n%p\":next -> \"n%p\":name;\n", c, c->next);
    }

    for (ss_pairing_node *c = p->child; c; c = c->next) {
        ss_pairing_node_to_dot(c, tostr);
    }
}

void ss_pairing_to_dot(ss_pairing_heap* t, char* (*tostr)(const ss_pairing_node*))
{

    printf("digraph graphname {\n"
           "rankdir=TB;"
           "node [shape=plaintext];\n");
    if (t->root)
        ss_pairing_node_to_dot(t->root, tostr);

    printf("}\n");
}

