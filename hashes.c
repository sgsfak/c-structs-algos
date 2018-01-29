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
    unsigned char *s = (unsigned char *)str;	/* unsigned string */

    const uint32_t FNV_32_PRIME = 0x01000193;

    uint32_t h = 0x811c9dc5; /* 2166136261 */
    while (len) {
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        h *= FNV_32_PRIME;
        /* xor the bottom with the current octet */
        h ^= *s++;
        --len;
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


#define HASHBITS 15
#define HASHSIZE (1U<<HASHBITS)

struct entry {
    char* word;
    int hits;
    int freq;
    struct list list_head;
};

struct entry hashtable[HASHSIZE];

struct entry* hashtable_search(const char* word, uint32_t h, bool add)
{
    size_t i = (size_t) h % HASHSIZE;
    struct entry* head = hashtable + i;
    struct list* list_head = &(head->list_head);

    if (head->word == NULL) {
        head->word = strdup(word);
        head->hits++;
        return head;
    }
    struct list* e;
    list_for_each(e, list_head) {
        struct entry* entry = list_entry(e, struct entry, list_head);
        if (strcmp(entry->word, word) == 0) {
            return entry;
        }
    }
    if (!add)
        return NULL;

    struct entry* entry = calloc(1, sizeof (struct entry));
    entry->word = strdup(word);
    list_insert(list_head, &entry->list_head);
    head->hits++;
    return entry;
}

void print(struct entry* hashtable, int len)
{
    for (unsigned i=0; i<len; ++i) {
        struct entry* p = hashtable+i;
        if (p->word) {
            printf("'%s'\t%d\n", p->word, p->freq);
            for (struct list* p= hashtable[i].list_head.next; p != &(hashtable[i].list_head); p = p->next) {
                struct entry* e = list_entry(p, struct entry, list_head);
                printf("'%s'\t%d\n", e->word, e->freq);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    struct entry* p;
    size_t len = HASHSIZE;
    uint32_t (*hfn)(const char*, size_t);

    for (p = hashtable, len=HASHSIZE; len--;p++) {
        p->word = NULL;
        p->hits = 0;
        p->freq = 0;
        list_init_head(&p->list_head);
    }

    hfn = h31_hash;
    if (argc>1) {
        switch (atoi(argv[1])) {
            case 1:
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

    FILE* fp = fopen("book.txt", "r");
    if (!fp) {
        perror("main");
        exit(-1);
    }
    char word[BUFSIZ]; /*big enough*/
    while(1)
    {
        if(fscanf(fp,"%s",word)!=1)
            break;
        uint32_t h = hfn(word, strlen(word));
        struct entry* e = hashtable_search(word, h, true);
        e->freq++;
    }


    fclose(fp);

    int n = 0;
    int collisions = 0;
    for (p = hashtable, len=HASHSIZE; len--; p++) {
        if (p->word) {
            ++n;
            collisions += p->hits;
        }
    }
    printf("N=%d, collisions=%d F=%.3g\n", n, collisions, collisions*1.0/n);
    print(hashtable, HASHSIZE);
}
