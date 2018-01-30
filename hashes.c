#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "list.h"

#define hashsize(n) (1U << (n))
#define hashmask(n) (hashsize(n) - 1)

#define mix(a,b,c) \
{ \
    a -= b; a -= c; a ^= (c >> 13); \
    b -= c; b -= a; b ^= (a << 8); \
    c -= a; c -= b; c ^= (b >> 13); \
    a -= b; a -= c; a ^= (c >> 12); \
    b -= c; b -= a; b ^= (a << 16); \
    c -= a; c -= b; c ^= (b >> 5); \
    a -= b; a -= c; a ^= (c >> 3); \
    b -= c; b -= a; b ^= (a << 10); \
    c -= a; c -= b; c ^= (b >> 15); \
}

uint32_t jen_hash(const char *k, size_t length)
{
    unsigned initval = 42;
    unsigned a, b;
    unsigned c = initval;
    unsigned len = length;

    a = b = 0x9e3779b9;

    while (len >= 12)
    {
        a += (k[0] + ((unsigned)k[1] << 8) + ((unsigned)k[2] << 16) + ((unsigned)k[3] << 24));
        b += (k[4] + ((unsigned)k[5] << 8) + ((unsigned)k[6] << 16) + ((unsigned)k[7] << 24));
        c += (k[8] + ((unsigned)k[9] << 8) + ((unsigned)k[10] << 16) + ((unsigned)k[11] << 24));

        mix(a, b, c);

        k += 12;
        len -= 12;
    }

    c += length;

    switch (len)
    {
    case 11: c += ((unsigned)k[10] << 24);
    case 10: c += ((unsigned)k[9] << 16);
    case 9: c += ((unsigned)k[8] << 8);
        /* First byte of c reserved for length */
    case 8: b += ((unsigned)k[7] << 24);
    case 7: b += ((unsigned)k[6] << 16);
    case 6: b += ((unsigned)k[5] << 8);
    case 5: b += k[4];
    case 4: a += ((unsigned)k[3] << 24);
    case 3: a += ((unsigned)k[2] << 16);
    case 2: a += ((unsigned)k[1] << 8);
    case 1: a += k[0];
    }

    mix(a, b, c);

    return c;
}
uint32_t elf_hash(const char *key, size_t len)
{
    unsigned char *p = (unsigned char*) key;
    uint32_t h = 0, g;
    int i;

    for (i = 0; i < len; i++)
    {
        h = (h << 4) + p[i];
        g = h & 0xf0000000L;

        if (g != 0)
        {
            h ^= g >> 24;
        }

        h &= ~g;
    }

    return h;
}
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
    uint32_t hash; /* the hash of the key, cached */
    struct entry* next;
};

typedef struct {
    unsigned int len;
    struct entry* e;
} ht_entry;

typedef uint32_t (*hfn_t)(const char*, size_t);
typedef struct  {
    unsigned int n; /* current number of elements (entries) */
    uint32_t size; /* number of buckets */
    unsigned int max_bucket_size;
    hfn_t hfn;
    ht_entry* table;  /* buckets */
} hashtable;


#define BITS_TO_HSIZE(b) (1U << (b))
#define HASH_SIZE(ht) ((ht)->size)
#define for_each_htentry(ht,e) for(e=(ht)->table;e!=(ht)->table+HASH_SIZE(ht);e++)
#define _HSH_P0 5U
#define _HSH_P1 11U
#define _HSH_P2 23U
#define _HSH_P3 47U
#define _HSH_P4 97U
#define _HSH_P5 199U
#define _HSH_P6 409U
#define _HSH_P7 823U
#define _HSH_P8 1741U
#define _HSH_P9 3469U
#define _HSH_P10 6949U
#define _HSH_P11 14033U
#define _HSH_P12 28411U
#define _HSH_P13 57557U
#define _HSH_P14 116731U
#define _HSH_P15 236897U
#define _HSH_P16 480881U
#define _HSH_P17 976369U
#define _HSH_P18 1982627U
#define _HSH_P19 4026031U
#define _HSH_P20 8175383U
#define _HSH_P21 16601593U
#define _HSH_P22 33712729U
#define _HSH_P23 68460391U
#define _HSH_P24 139022417U
#define _HSH_P25 282312799U
#define _HSH_P26 573292817U
#define _HSH_P27 1164186217U
#define _HSH_P28 2364114217U
#define _HSH_P29 4294967291U

static uint32_t _primes[] = {
    _HSH_P0,
    _HSH_P1,
    _HSH_P2,
    _HSH_P3,
    _HSH_P4,
    _HSH_P5,
    _HSH_P6,
    _HSH_P7,
    _HSH_P8,
    _HSH_P9,
    _HSH_P10,
    _HSH_P11,
    _HSH_P12,
    _HSH_P13,
    _HSH_P14,
    _HSH_P15,
    _HSH_P16,
    _HSH_P17,
    _HSH_P18,
    _HSH_P19,
    _HSH_P20,
    _HSH_P21,
    _HSH_P22,
    _HSH_P23,
    _HSH_P24,
    _HSH_P25,
    _HSH_P26,
    _HSH_P27,
    _HSH_P28,
    _HSH_P29,
    0U
};

#define _primes_len (sizeof(_primes)/sizeof(_primes[0])-1)

uint32_t mod_hash_size(hashtable* ht, uint32_t k)
{
    switch (ht->size) {
        case _HSH_P0:  return k % _HSH_P0; 
        case _HSH_P1:  return k % _HSH_P1; 
        case _HSH_P2:  return k % _HSH_P2; 
        case _HSH_P3:  return k % _HSH_P3; 
        case _HSH_P4:  return k % _HSH_P4; 
        case _HSH_P5:  return k % _HSH_P5; 
        case _HSH_P6:  return k % _HSH_P6; 
        case _HSH_P7:  return k % _HSH_P7; 
        case _HSH_P8:  return k % _HSH_P8; 
        case _HSH_P9:  return k % _HSH_P9; 
        case _HSH_P10: return k %_HSH_P10; 
        case _HSH_P11: return k %_HSH_P11; 
        case _HSH_P12: return k %_HSH_P12; 
        case _HSH_P13: return k %_HSH_P13; 
        case _HSH_P14: return k %_HSH_P14; 
        case _HSH_P15: return k %_HSH_P15; 
        case _HSH_P16: return k %_HSH_P16; 
        case _HSH_P17: return k %_HSH_P17; 
        case _HSH_P18: return k %_HSH_P18; 
        case _HSH_P19: return k %_HSH_P19; 
        case _HSH_P20: return k %_HSH_P20; 
        case _HSH_P21: return k %_HSH_P21; 
        case _HSH_P22: return k %_HSH_P22; 
        case _HSH_P23: return k %_HSH_P23; 
        case _HSH_P24: return k %_HSH_P24; 
        case _HSH_P25: return k %_HSH_P25; 
        case _HSH_P26: return k %_HSH_P26; 
        case _HSH_P27: return k %_HSH_P27; 
        case _HSH_P28: return k %_HSH_P28; 
        case _HSH_P29: return k %_HSH_P29; 
        default:
                       return k % ht->size;
    };
}
#define ht_hash_to_bucket(ht,h)  ((ht)->table + (mod_hash_size((ht), (h))))

int _next_prime_for_expand(uint32_t minSize)
{
    uint32_t* p;

    for (p=_primes; *p && *p < minSize; ++p);
    return (*p ? *p : _primes[_primes_len - 1]);
}

hashtable* ht_create(uint32_t initial_size, hfn_t hfn)
{
    hashtable *h = calloc(1U, sizeof(hashtable));
    if (!h) {
        perror("ht_create");
        return NULL;
    }
    h->size = _next_prime_for_expand(initial_size);
    h->hfn = hfn;
    h->table = calloc(HASH_SIZE(h), sizeof(ht_entry));
    if (h->table == NULL) {
        perror("ht_create");
        free(h);
        return NULL;
    }
    printf("Hashtable created with size %u\n", HASH_SIZE(h));
    return h;
}

struct entry* _ht_entry_create(hashtable* ht, const char* word, uint32_t h)
{
    struct entry* e = calloc(1, sizeof(struct entry));
    if (!e) {
        perror("_ht_entry_create");
        return NULL;
    }
    e->key = strdup(word);
    e->hash = h;
    return e;
}

void _ht_entry_destroy(hashtable* ht, ht_entry* he, void (*destroy_val_fn)(void*))
{
    for(struct entry* t = he->e; t;) {
        struct entry* tnext = t->next;
        free(t->key);
        destroy_val_fn(t->val);
        free(t);
        t = tnext;
    }
    he->e = NULL;
    he->len = 0;
} 

void ht_destroy(hashtable* ht, void (*destroy_val_fn) (void*))
{
    ht_entry* e;
    for_each_htentry(ht,e) _ht_entry_destroy(ht, e, destroy_val_fn);
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
    uint32_t newSize = _next_prime_for_expand(2*HASH_SIZE(ht));
    printf("Current load factor %4.2f.. (max bkt size=%u) rehashing to %u ..\n",
            ht_load_factor(ht), ht->max_bucket_size, newSize);
    hashtable* hnew = ht_create(newSize, ht->hfn);
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
    ht->size = hnew->size;
    ht->max_bucket_size = hnew->max_bucket_size;
    free(hnew);
}
void _ht_insert_entry(hashtable* ht, ht_entry* bucket, struct entry* e)
{
    uint32_t size = HASH_SIZE(ht);
    unsigned int n = ht->n;
    if (n + 1 > (3*size >> 2)) { // USe the 0.75 factor
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

char* trim_str(char* s)
{
    char* p, *q;
    for(p=s, q=s; *p; p++) {
        if (isalnum(*p)) {
            *q = tolower(*p);
            ++q;
        } 
    }
    *q = '\0';
    return s;
}

int main(int argc, char* argv[])
{
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
            case 6:
                hfn = elf_hash;
                break;
            case 7:
                hfn = jen_hash;
                break;
            default:
                break;
        }
    }
    hashtable* ht = ht_create(1000, hfn);

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
        char key[BUFSIZ];
        strcpy(key, word);
        trim_str(key);
        struct entry* e = ht_put(ht, key);
        if (e->val == NULL) {
            e->val = strdup(word);
        }
        e->freq++;
        k++;
    }
    printf("Read %d words..\n", k);


    fclose(fp);

    printf("Size: %u (total: %u) Load: %4.2f max bucket size=%d\n", 
            ht->n, HASH_SIZE(ht), ht_load_factor(ht),ht->max_bucket_size);
    strcpy(word, "president,");
    trim_str(word);
    struct entry* e = ht_get(ht, word);
    if (e) {
        ht_entry* b = ht_hash_to_bucket(ht, e->hash);
        printf("'%s' found: val='%s', freq = %d collisions=%d\n",
                word, e->val, e->freq, b->len);
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
    /*
    int max_freq = 0;
    ht_traverse(ht, find_max, &max_freq);
    printf("Max frequency: %d\n", max_freq);
    */
    ht_destroy(ht, free);

}
