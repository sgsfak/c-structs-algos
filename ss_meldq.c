#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "meldq.h"


/* splitmix64 for seeding xoroshiro: http://prng.di.unimi.it/splitmix64.c 
 * The input "seed" can be any value 
 */
static uint64_t splitmix64(uint64_t x) {
	uint64_t z = (x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

static inline uint32_t rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

/*
 * This is xoshiro128++ 1.0, one of our 32-bit all-purpose, rock-solid
 * generators. It has excellent speed, a state size (128 bits) that is
 * large enough for mild parallelism, and it passes all tests we are aware
 * of.
 * See http://prng.di.unimi.it/xoshiro128plusplus.c
 */
static uint32_t xoshiro128plusplus(uint32_t s[4]) {
    const uint32_t result = rotl(s[0] + s[3], 7) + s[0];

	const uint32_t t = s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 11);

	return result;
}

void ss_meldq_init(ss_meldq* h, int (*compar)(ss_meldq_node*, ss_meldq_node*), int rand_seed)
{
    h->root = NULL;
    h->compar = compar;
    uint64_t a = splitmix64(rand_seed);
    uint64_t b = splitmix64(a);
    h->rand_state[0] = a >> 32;
    h->rand_state[1] = a & 0xFFFFFFFF;
    h->rand_state[2] = b >> 32;
    h->rand_state[3] = b & 0xFFFFFFFF;
    h->n = 0;
}

static 
ss_meldq_node* meld(ss_meldq* q, ss_meldq_node* a, ss_meldq_node* b)
{
    if (a == NULL)
        return b;
    if (b == NULL)
        return a;
    /* swap a and b if needed, so that a is always the min */
    if (q->compar(a, b) > 0) {
        ss_meldq_node* t = a;
        a = b;
        b = t;
    }
    uint32_t n = xoshiro128plusplus(q->rand_state);
    if (n & 1) {
        a->left = meld(q, a->left, b);
        a->left->parent = a;
    }
    else {
        a->right = meld(q, a->right, b);
        a->right->parent = a;
    }
    return a;
}

static 
void detach_tree(ss_meldq* q, ss_meldq_node* a)
{
    if (q->root == a) {
        if (a) a->parent = NULL;
        q->root = NULL;
        return;
    }
    ss_meldq_node* p = a->parent;
    if (p->left == a)
        p->left = NULL;
    else
        p->right = NULL;
    a->parent = NULL;
}




ss_meldq_node* ss_meldq_find_min(ss_meldq* h)
{
    return h->root;
}

ss_meldq_node* ss_meldq_extract_min(ss_meldq* q)
{
    ss_meldq_node* r = q->root;
    if (r == NULL)
        return NULL;
    q->root = meld(q, r->left, r->right);
    if (q->root)
        q->root->parent = NULL;
    q->n--;
    r->parent = r->left = r->right = NULL;
    return r;
}

void ss_meldq_insert(ss_meldq* h, ss_meldq_node* node)
{
    node->left = node->right = NULL;
    h->root = meld(h, h->root, node);
    h->root->parent = NULL;
    h->n++;
}

void ss_meldq_delete(ss_meldq* q, ss_meldq_node* node)
{
    if (node == q->root) {
        (void) ss_meldq_extract_min(q);
        return;
    }
    ss_meldq_node* p = node->parent;
    ss_meldq_node* n = meld(q, node->left, node->right);
    if (n)
        n->parent = p;
    if (p->left == node)
        p->left = n;
    else
        p->right = n;
    q->n--;
}

void ss_meldq_update_pri(ss_meldq* h, ss_meldq_node* item);

typedef struct {
    ss_meldq_node t;
    uint32_t pri;
} data;

int cmp(ss_meldq_node* a, ss_meldq_node* b)
{
    data* da = container_of(a, data, t);
    data* db = container_of(b, data, t);

    return (da->pri > db->pri) - (da->pri < db->pri);
}


int height(ss_meldq_node* r)
{
    if (r == NULL || (r->left == NULL && r->right == NULL))
        return 0;
    int h1 = height(r->left);
    int h2 = height(r->right);
    return (h1 > h2 ? h1 + 1 : h2 + 1);
}

int main(int argc, char *argv[])
{
    
    uint64_t a = splitmix64(42);
    uint64_t b = splitmix64(a);
    uint32_t s[4] = { a >> 32, a & 0xFFFFFFFF, b >> 32, b & 0xFFFFFFFF};

    ss_meldq q;
    ss_meldq_init(&q, cmp, 42);
    const int N = 1000000;
    data* bb = calloc(N, sizeof(data));
    for (int i = 0; i < N; i++) {
        uint32_t n = xoshiro128plusplus(s);
        /* printf("%u last bit: %d\n", n, n&1); */
        bb[i].pri = n;
        ss_meldq_insert(&q, &(bb[i].t));

    }

    ss_meldq_node* r = q.root;
    data* min = container_of(r, data, t);
    int h = height(r);
    printf("Min %u (height:%d) out of %u, left:%u, right:%u\n", min->pri, h, q.n, container_of(r->left, data, t)->pri, container_of(r->right, data, t)->pri);

    ss_meldq_extract_min(&q);
    r = q.root;
    min = container_of(r, data, t);
    printf("Extracted min, current min %u out of %u, left:%u, right:%u\n", min->pri, q.n, container_of(r->left, data, t)->pri, container_of(r->right, data, t)->pri);

    int c = N/2;
    data* o = bb+c;
    ss_meldq_delete(&q, &o->t);
    r = q.root;
    min = container_of(r, data, t);
    printf("Deleted priority %u, current min %u out of %u, left:%u, right:%u\n", o->pri, min->pri, q.n, container_of(r->left, data, t)->pri, container_of(r->right, data, t)->pri);

    return 0;
}
