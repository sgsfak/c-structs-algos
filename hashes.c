#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "list.h"

/*
 * The Dan Bernstein popuralized hash..  See
 * https://github.com/pjps/ndjbdns/blob/master/cdb_hash.c#L26 Due to hash
 * collisions it seems to be replaced with "siphash" in n-djbdns, see
 * https://github.com/pjps/ndjbdns/commit/16cb625eccbd68045737729792f09b4945a4b508
 */
uint32_t djb33_hash(const char* s, size_t len)
{
    uint32_t h = 5381;
    while (len) {
        /* h = 33 * h ^ s[i]; */
        h += (h << 5); h ^= *s++; --len;
    }
    return h;
}


/*
 *
 * The Java hash, but really no-one seems to know where it came from, see
 * https://bugs.java.com/bugdatabase/view_bug.do?bug_id=4045622
 */
uint32_t h31_hash(const char* s, size_t len)
{
    uint32_t h = 0;
    while (len) {
        h = 31 * h + *s++;
        --len;
    }
    return h;
}

/*
 * The FNV Hash
 * See: http://www.isthe.com/chongo/tech/comp/fnv/
 *      https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 */
uint32_t fnv32_hash(const char *str, size_t len)
{
    unsigned char *s = (unsigned char *)str;

    const uint32_t FNV_32_PRIME = 0x01000193;

    uint32_t h = 0x811c9dc5; /* 2166136261 */
    while (len--) {
        h *= FNV_32_PRIME;
        h ^= *s++;
    }

    return h;
}


/*
 * "This came from ejb's hsearch."
 */
uint32_t ejb_hash(const char *s, size_t len)
{
	unsigned char *key = (unsigned char*) s;
	const uint32_t PRIME1 = 37;
	const uint32_t PRIME2 = 1048583;
	uint32_t h = 0;

	while (len--) {
		h = h * PRIME1 ^ (*key++ - ' ');
	}
	h %= PRIME2;

	return h;
}

/*
 * Bob Jenkins "One-at-a-time" hash
 */
uint32_t oat_hash(const char *s, size_t len)
{
    unsigned char *p = (unsigned char*) s;
    uint32_t h = 0;

    while(len--) {
        h += *p++;
        h += (h << 10);
        h ^= (h >> 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
}

struct entry {
    char* key;
    void* val;
    int freq;
    uint32_t hash;
    struct entry* next;
};

typedef struct {
    unsigned int len;
    struct entry* e;
} ht_entry;

typedef uint32_t (*hfn_t)(const char*, size_t);
typedef struct  {
    unsigned int n;
    uint8_t bits;
    unsigned int max_bucket_size;
    hfn_t hfn;
    ht_entry* table; 
} hashtable;


#define BITS_TO_HSIZE(b) (1U << (b))
#define HASH_SIZE(ht) (BITS_TO_HSIZE((ht)->bits))
#define for_each_htentry(ht,e) for(e=(ht)->table;e!=(ht)->table+HASH_SIZE(ht);e++)
#define ht_hash_to_bucket(ht,h)  ((ht)->table + ((h) & (HASH_SIZE((ht)) - 1)));

hashtable* ht_create(uint8_t bits, hfn_t hfn)
{
    hashtable *h = calloc(1U, sizeof(hashtable));
    if (!h) {
        perror("ht_create");
        return NULL;
    }
    h->bits = bits;
    h->hfn = hfn;
    h->table = calloc(HASH_SIZE(h), sizeof(ht_entry));
    if (h->table == NULL) {
        perror("ht_create");
        free(h);
        return NULL;
    }
    return h;
}

struct entry* _ht_entry_create(hashtable* ht, const char* word, uint32_t h)
{
    struct entry* e = malloc(sizeof(struct entry));
    if (!e) {
        perror("_ht_entry_create");
        return NULL;
    }
    e->key = strdup(word);
    e->hash = h;
    return e;
}

void _ht_entry_destroy(hashtable* ht, ht_entry* he)
{
    for(struct entry* t = he->e; t;) {
        struct entry* tnext = t->next;
        free(t->key);
        free(t);
        t = tnext;
    }
    he->e = NULL;
    he->len = 0;
} 

void ht_destroy(hashtable* ht)
{
    ht_entry* e;
    for_each_htentry(ht,e) _ht_entry_destroy(ht, e);
    free(ht->table);
    free(ht);
}

void ht_traverse(hashtable* ht, int (*action) (struct entry*, void*), void* arg)
{
    ht_entry* he;
    for_each_htentry(ht,he) {
        for (struct entry* e = he->e; e; e = e->next) {
            int w = action(e, arg);
            if (w < 0)
                return;
        }
    }
}

float ht_load_factor(hashtable* h)
{
    return h->n*1.0/HASH_SIZE(h);
}

ht_entry* ht_key_to_bucket(hashtable* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    ht_entry* b = ht_hash_to_bucket(ht, h);
    return b;
}


void _ht_insert_entry(hashtable* ht, ht_entry* bucket, struct entry* e);
void _ht_rehash(hashtable* ht)
{
    printf("Current load factor %4.2f.. rehashing to %d bits..\n", ht_load_factor(ht), ht->bits+1);
    hashtable* hnew = ht_create(ht->bits + 1, ht->hfn);
    if (!hnew)
        return;
    ht_entry* he;
    for_each_htentry(ht,he) {
        for (struct entry* e = he->e; e;) {
            struct entry* t = e->next;
            ht_entry* b = ht_hash_to_bucket(hnew, e->hash);
            _ht_insert_entry(hnew, b, e);
            e = t;
        }
        he->e = NULL;
    }

    free(ht->table);
    ht->table = hnew->table;
    ht->bits ++;
    free(hnew);
}
void _ht_insert_entry(hashtable* ht, ht_entry* bucket, struct entry* e)
{
    unsigned int size = HASH_SIZE(ht);
    unsigned int n = ht->n;
    if (n + 1 > (3*size >> 2)) {
        /* We need to rehash ... */
        _ht_rehash(ht);
    }

    bucket->len++;
    e->next = bucket->e;
    bucket->e = e;
    ht->n++;
    if (bucket->len > ht->max_bucket_size)
        ht->max_bucket_size = bucket->len;
}

void ht_delete(hashtable* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    ht_entry* b = ht_hash_to_bucket(ht, h);

    if (b->e == NULL) {
        return;
    }
    struct entry** e;
    for (e = &b->e; *e; ) {
        struct entry* t = *e;
        if (h == t->hash && strcmp(t->key, word) == 0) {
            *e = t->next;
            free(t);
            b->len--;
            ht->n--;
            return;
        }
        e = &t->next;
    }
}

struct entry* ht_get(hashtable* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    ht_entry* b = ht_hash_to_bucket(ht, h);
    for (struct entry* e = b->e; e; e = e->next) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return e;
        }
    }
    return NULL;
}

struct entry* ht_put(hashtable* ht, const char* word)
{
    uint32_t h = ht->hfn(word, strlen(word));
    ht_entry* b = ht_hash_to_bucket(ht, h);

    struct entry* e;
    for (e = b->e; e; e = e->next) {
        if (h == e->hash && strcmp(e->key, word) == 0) {
            return e;
        }
    }

    e = _ht_entry_create(ht, word, h);
    if (!e)
        return NULL;
    _ht_insert_entry(ht, b, e);
    return e;
}

bool ht_exists(hashtable* ht, const char* word)
{
    return ht_get(ht, word) != NULL;
}

int find_max(struct entry* e, void * arg)
{
    int* a = (int*) arg;
    if (e->freq > *a)
        *a = e->freq;
    return 0;
}

int main(int argc, char* argv[])
{
    uint8_t bits = 6;
    hfn_t hfn = fnv32_hash;
    if (argc>1) {
        switch (atoi(argv[1])) {
            case 1:
                hfn = h31_hash;
                break;
            case 2:
                hfn = ejb_hash;
                break;
            case 3:
                hfn = oat_hash;
                break;
            case 4:
                hfn = fnv32_hash;
                break;
            case 5:
                hfn = djb33_hash;
                break;
            default:
                break;
        }
    }
    hashtable* ht = ht_create(bits, hfn);

    FILE* fp = fopen("book.txt", "r");
    if (!fp) {
        perror("main");
        exit(-1);
    }
    char word[BUFSIZ]; /*big enough*/
    int k = 0;
    while(1)
    {
        if(fscanf(fp,"%s",word)!=1)
            break;
        struct entry* e = ht_put(ht, word);
        e->freq++;
        k++;
    }
    printf("Read %d words..\n", k);


    fclose(fp);

    printf("Size: %u (total: %u) Load: %4.2f max bucket size=%d\n", 
            ht->n, BITS_TO_HSIZE(ht->bits), ht_load_factor(ht),ht->max_bucket_size);
    struct entry* e = ht_get(ht, "president,");
    if (e) {
        ht_entry* b = ht_hash_to_bucket(ht, e->hash);
        printf("'president' found: freq = %d collisions=%d\n", e->freq, b->len);
    }
    ht_delete(ht, "the");
    e = ht_get(ht, "the");
    printf("Now %p\n", e);

    ht_entry* b, *m;
    m = ht->table;
    for_each_htentry(ht, b) {
        if (b->len > m->len)
            m = b;
    }
    printf("Most collisions: %d at %ld\n", m->len, m - ht->table);
    for (struct entry* e= m->e; e; e= e->next) {
        printf("\t'%s' hash=%u freq=%d\n", e->key, e->hash, e->freq); 
    }
    int max_freq = 0;
    ht_traverse(ht, find_max, &max_freq);
    printf("Max frequency: %d\n", max_freq);
    ht_destroy(ht);
}
