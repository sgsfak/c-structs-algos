#include <stdlib.h>
#include "ss_treap.h"
#include <string.h>
#include <stdio.h>



#ifdef NDEBUG
#define log(...)
#else
#define log(...) fprintf (stderr, __VA_ARGS__)
#endif

/* From http://nullprogram.com/blog/2017/09/21/ */
static
uint32_t spcg32(uint64_t s[1])
{
    uint64_t m = 0x9b60933458e17d7d;
    uint64_t a = 0xd737232eeccdf7ed;
    *s = *s * m + a;
    int shift = 29 - (*s >> 61);
    return *s >> shift;
}

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
    treap->root = NULL;
    treap->n = treap->max_height = 0;

    treap->random_seed = calloc(1, sizeof(struct ss_treap_prng_state));
    jsf32_init(treap->random_seed, 1u);
}

static
void ss_treap_destroy_node(ss_treap_node* p)
{
    if (p->left)
        ss_treap_destroy_node(p->left);
    if (p->right)
        ss_treap_destroy_node(p->right);
    free(p);
}

void ss_treap_destroy(ss_treap* treap)
{
    if (treap) {
        if (treap->root) {
            ss_treap_destroy_node(treap->root);
            treap->root = NULL;
        }
        free(treap->random_seed);
    }
}


static
ss_treap_node* ss_treap_node_new(void* val, uint32_t pri)
{
    ss_treap_node* p = malloc(sizeof *p);
    p->left = p->right = NULL;
    p->priority = pri;
    p->val = val;
    return p;
}

static
ss_treap_node* ss_treap_insert_node(ss_treap* treap,
        ss_treap_node* node, 
        void* val, uint32_t priority, int depth)
{
    log("...Now at %p\n", node);
    if (node == NULL) {
        depth++;
        if (treap->max_height < depth)
            treap->max_height = depth;
        treap->n++;
        return ss_treap_node_new(val, priority);
    }

    log("...Comparing %p with %p\n", node->val, val);
    int k = treap->compar(node->val, val);
    if (k < 0) {
        depth++;
        node->right = ss_treap_insert_node(treap, node->right, val, priority, depth);
    }
    else if (k > 0) {
        depth++;
        node->left = ss_treap_insert_node(treap, node->left, val, priority, depth);
    }
    else {
        node->priority = priority;
    }
    ss_treap_node* p = node;
    if (node->left != NULL && node->left->priority > p->priority)
        p = node->left;
    if (node->right != NULL && node->right->priority > p->priority)
        p = node->right;


    if (p == node->left) {
        /* Right rotation */
        node->left = p->right;
        p->right = node;
    }
    else if (p == node->right) {
        /* Left rotation */
        node->right = p->left;
        p->left = node;
    }

    return p;
}
static
ss_treap_node* ss_treap_join(ss_treap* treap, ss_treap_node* a, ss_treap_node* b)
{
    if (a == NULL)
        return b;
    if (b == NULL)
        return a;
    ss_treap_node* root = a;
    ss_treap_node* child = b;
    if (b->priority > a->priority) {
        root = b;
        child = a;
    }
    int k = treap->compar(child->val, root->val);
    if (k < 0)
        root->left = ss_treap_join(treap, root->left, child);
    else
        root->right = ss_treap_join(treap, root->right, child);
    return root;
}

static
ss_treap_node* ss_treap_delete_node(ss_treap* treap, ss_treap_node* node, void* val)
{
    if (node == NULL)
        return NULL;
    log("...Comparing %p with %p\n", node->val, val);

    int k = treap->compar(node->val, val);
    if (k < 0) {
        node->right = ss_treap_delete_node(treap, node->right, val);
        return node;
    }
    else if (k > 0) {
        node->left = ss_treap_delete_node(treap, node->left, val);
        return node;
    }

    ss_treap_node* p = ss_treap_join(treap, node->right, node->left);
    free(node);
    treap->n--;
    return p;
}

void ss_treap_delete(ss_treap* treap, void* val)
{
    treap->root = ss_treap_delete_node(treap, treap->root, val);
}


static
void ss_treap_node_to_dot(ss_treap* t, ss_treap_node* p, char* (*tostr)(void*))
{

    char label[200];
    sprintf(label, "<left> | <name> %s:%u | <right>", tostr(p->val), p->priority);
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

void ss_treap_to_dot(ss_treap* t, char* (*tostr)(void*))
{

    printf("digraph graphname {\n"
            "node [shape = record];\n");
    if (t->root)
        ss_treap_node_to_dot(t, t->root, tostr);

    printf("}\n");
}

ss_treap_node* ss_treap_find(ss_treap* treap, const void* val)
{
    ss_treap_node* p = treap->root;
    while(p != NULL) {
        log("checking %p [pri=%u, left=%p, right=%p]\n", p, p->priority, p->left, p->right);
        int k = treap->compar(p->val, val);
        if (k == 0)
            return p;
        if (k < 0) 
            p = p->right;
        else
            p = p->left;
    }
    return NULL;
}

/*
void ss_treap_update(ss_treap* treap, const void* val,
        void (*update_cb)(const void* old_val, const void* val))
{
    ss_treap_node* p = treap->root;
    void* valp;
    while(p) {
        valp = TREAP_NODE_VAL(p);
        int k = treap->compar(valp, val);
        if (k < 0) 
            p = p->left;
        else if (k > 0)
            p = p->right;
        else 
            update_cb(valp, val);
    }
}
*/


void ss_treap_insert_pri(ss_treap* treap, void* val, uint32_t priority)
{
    int depth = 1;
    treap->root = ss_treap_insert_node(treap, treap->root, val, priority, depth);
    log("HEIGHT=%d\n", treap->max_height);
    /* ss_treap_to_dot(treap); */
}

void ss_treap_insert(ss_treap* treap, void* val)
{
    uint32_t pri = jsf32(treap->random_seed) % 100;
    /* uint32_t pri = spcg32(&treap->random_seed->a); */
    log("..priority=%u\n", pri);
    ss_treap_insert_pri(treap, val, pri);
}

