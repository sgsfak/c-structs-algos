#include <stdlib.h>
#include "ss_treap.h"
#include <string.h>
#include <stdio.h>



#ifdef NDEBUG
#define log(...)
#else
#define log(...) fprintf (stdout, __VA_ARGS__)
#endif

typedef struct ss_treap_prng_state {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} jsf32_state;


/* Bob Jenkins "small fast" PRNG:
 * http://burtleburtle.net/bob/rand/smallprng.html
 */
#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
static
uint32_t jsf32( jsf32_state *x ) {
    uint32_t e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

static
void jsf32_init( jsf32_state *x, uint32_t seed ) {
    int i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i=0; i<20; ++i) {
        (void)jsf32(x);
    }
}

void ss_treap_init(ss_treap* treap)
{
    static struct ss_treap_prng_state* rng;
    treap->root = NULL;
    treap->n = treap->max_height = 0;

    if (!rng) {
        rng = calloc(1, sizeof(struct ss_treap_prng_state));
        jsf32_init(rng, 42u);
    }
    treap->random_seed = rng;
}


ss_treap_node* ss_treap_insert_node(ss_treap* treap,
        ss_treap_node* current, ss_treap_node* node)
{
    if (current == NULL) {
        treap->n++;
        return node;
    }

    log("[INS] Comparing %p with %p\n", current, node);
    int k = treap->compar(current, node);
    if (k < 0) {
        current->right = ss_treap_insert_node(treap, current->right, node);
    }
    else if (k > 0) {
        current->left = ss_treap_insert_node(treap, current->left, node);
    }
    else {
        return current;
    }
    ss_treap_node* p = current;
    if (current->left != NULL && current->left->priority > p->priority)
        p = current->left;
    if (current->right != NULL && current->right->priority > p->priority)
        p = current->right;


    if (p == current->left) {
        /* Right rotation */
        current->left = p->right;
        p->right = current;
    }
    else if (p == current->right) {
        /* Left rotation */
        current->right = p->left;
        p->left = current;
    }

    return p;
}

static
ss_treap_node* ss_treap_merge_children(ss_treap* treap, ss_treap_node* node)
{
    if (node->left == NULL && node->right == NULL)
        return NULL;

    if (node->left == NULL)
        return node->right;

    if (node->right == NULL)
        return node->left;

    ss_treap_node* p = node->left->priority > node->right->priority ? node->left : node->right;

    if (p == node->left) {
        /* Right rotation */
        node->left = p->right;
        p->right = ss_treap_merge_children(treap, node);
    }
    else {
        /* Left rotation */
        node->right = p->left;
        p->left = ss_treap_merge_children(treap, node);
    }

    return p;
}

static
ss_treap_node* ss_treap_delete_node(ss_treap* treap, ss_treap_node* current, ss_treap_node* node)
{
    if (current == NULL)
        return NULL;

    int k = treap->compar(current, node);
    if (k < 0) {
        current->right = ss_treap_delete_node(treap, current->right, node);
        return current;
    }
    else if (k > 0) {
        current->left = ss_treap_delete_node(treap, current->left, node);
        return current;
    }

    ss_treap_node* p = ss_treap_merge_children(treap, current);
    treap->n--;
    return p;
}

void ss_treap_delete(ss_treap* treap, ss_treap_node* node)
{
    treap->root = ss_treap_delete_node(treap, treap->root, node);
}

static
ss_treap_node* ss_treap_increase_node(ss_treap* treap,
        ss_treap_node* current, ss_treap_node* node, uint32_t pri)
{
    if (current == NULL) {
        return NULL;
    }

    int k = treap->compar(current, node);
    log("[UPD] Comparing %p with %p (%s)\n", current, node, k == 0 ? "SAME!" : "differ");
    if (k == 0) {
        log("[UPD] UPDATING priority of %p from %u to %u\n", current, current->priority, pri);
        current->priority = pri;
        return current;
    }

    if (k < 0)
        current->right = ss_treap_increase_node(treap, current->right, node, pri);
    else
        current->left = ss_treap_increase_node(treap, current->left, node, pri);

    ss_treap_node* p = current;
    if (current->left != NULL && current->left->priority > p->priority)
        p = current->left;
    if (current->right != NULL && current->right->priority > p->priority)
        p = current->right;

    if (p == current->left) {
        /* Right rotation */
        current->left = p->right;
        p->right = current;
    }
    else if (p == current->right) {
        /* Left rotation */
        current->right = p->left;
        p->left = current;
    }

    return p;
}

static
void ss_treap_node_to_dot(ss_treap* t, ss_treap_node* p, char* (*tostr)(const ss_treap_node*))
{

    char label[200];
    snprintf(label, 200, "<left> | <name> %s:%u | <right>", tostr(p), p->priority);
    printf("n%p [label=\"%s\"];\n", p, label);

    if (p->left) {
        printf("\"n%p\":left -> \"n%p\":name;\n", p, p->left);
        ss_treap_node_to_dot(t, p->left, tostr);
    }
    if (p->right) {
        printf("\"n%p\":right -> \"n%p\":name;\n", p, p->right);
        ss_treap_node_to_dot(t, p->right, tostr);
    }

}

void ss_treap_to_dot(ss_treap* t, char* (*tostr)(const ss_treap_node*))
{

    printf("digraph graphname {\n"
            "node [shape = record];\n");
    if (t->root)
        ss_treap_node_to_dot(t, t->root, tostr);

    printf("}\n");
}

ss_treap_node* ss_treap_find(ss_treap* treap, ss_treap_node* node)
{
    ss_treap_node* p = treap->root;
    while(p != NULL) {
        log("checking %p [pri=%u, left=%p, right=%p]\n", p, p->priority, p->left, p->right);
        int k = treap->compar(p, node);
        if (k == 0)
            return p;
        if (k < 0) 
            p = p->right;
        else
            p = p->left;
    }
    return NULL;
}

void ss_treap_insert_pri(ss_treap* treap,
        ss_treap_node* node, uint32_t priority)
{

    node->left     = NULL;
    node->right    = NULL;
    node->priority = priority;
    treap->root = ss_treap_insert_node(treap, treap->root, node);
}

void ss_treap_insert(ss_treap* treap, ss_treap_node* node)
{
    uint32_t pri = jsf32(treap->random_seed);
    log("..priority=%u\n", pri);
    ss_treap_insert_pri(treap, node, pri);
}
void ss_treap_increase_pri(ss_treap* treap, ss_treap_node* node, uint32_t priority)
{
    treap->root = ss_treap_increase_node(treap, treap->root, node, priority);
}

int ss_treap_height_r(ss_treap_node* n)
{
    if (n == NULL)
        return 0;
    int hl = ss_treap_height_r(n->left);
    int hr = ss_treap_height_r(n->right);

    return (hr > hl ? hr : hl) + 1;
}
int ss_treap_height(ss_treap* treap)
{
    return ss_treap_height_r(treap->root);
}

