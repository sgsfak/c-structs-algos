#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "trie.h"
 
#define ARRAY_SIZE(a) ((int) (sizeof(a)/sizeof(a[0])))
 
// Alphabet size (# of symbols)
#define ALPHABET_SIZE (26)
 
// Converts key current character into index
// use only 'a' through 'z' and lower case
#define CHAR_TO_INDEX(c) ((int)c - (int)'a')
 
// trie node
typedef struct trie_node {
    struct trie_node* children[ALPHABET_SIZE];
    struct trie_node *parent;
    void* data;
    int is_end_of_word;
    char character;
} trie_node_t;
 
struct trie_iterator_t {
    trie_node_t* current;
    int level;
};

struct trie_t {
    trie_node_t root;
    int count;

    trie_iterator_t iter;
};

static
struct trie_node* new_trie_node_(char character, struct trie_node* parent)
{
    struct trie_node* n = malloc(sizeof *n);
    for (int i=0; i<ALPHABET_SIZE; ++i)
        n->children[i] = NULL;
    n->parent = parent;
    n->data = NULL;
    n->is_end_of_word = 0;
    n->character = character;
    return n;
}

char* my_strdup(const char* src)
{
    int n = strlen(src);
    char* dst = malloc(n+1);
    memcpy(dst, src, n+1);
    return dst;
}

trie_t *trie_create(void)
{
    trie_t *t = calloc(1, sizeof(trie_t));
    return t;
}
 
int trie_count(trie_t *t)
{
    return t->count;
}

#define ISALPHA(c) (((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))
void* trie_insert(trie_t *t, const char *key, void* data)
{
    trie_node_t *node = &t->root;
    for (const char* s=key; *s; ++s) {
        assert(ISALPHA(*s));
        char k = tolower(*s);
        int index = CHAR_TO_INDEX(k);
        trie_node_t* child = node->children[index];
        if (!child) {
            child = new_trie_node_(k, node);
            node->children[index] = child;
            /* printf("Adding %p child at index '%c' of parent %p\n", child, k, node); */
        }
        node = child;
    }

    if (!node->is_end_of_word) {
        /* I.e. the node was a prefix and
         * now should be a word too
         */
        node->is_end_of_word = 1;
        node->data = data;
        t->count++;
    }
    return node->data;
}
 
trie_node_t* trie_search_(trie_node_t *root, const char *key)
{
    trie_node_t *node = root;
    for (const char* s=key; *s; ++s) {
        char k = tolower(*s);
        int index = CHAR_TO_INDEX(k);
        trie_node_t* child = node->children[index];
 
        if (!child)
            return NULL;
        node = child;
    }
    return node;
}
 
int trie_exists(trie_t *t, const char *key)
{
    trie_node_t* node = trie_search_(&t->root, key);
    return (node != NULL && node->is_end_of_word);
}

void* trie_find(trie_t *t, const char *key)
{
    trie_node_t* node = trie_search_(&t->root, key);
    if (!node || !node->is_end_of_word)
        return NULL;
    return node->data;
}

static
trie_node_t* trie_dfs_next_(trie_node_t* node, int* level)
{
    for (int i=0; i<ALPHABET_SIZE; ++i) {
        if (node->children[i]) {
            ++*level;
            return node->children[i];
        }
    }

    for(trie_node_t* p = node->parent; p; p = p->parent) {
        int index = CHAR_TO_INDEX(node->character);
        /* Find sibling */
        for (int j=index+1; j<ALPHABET_SIZE; ++j)
            if (p->children[j])
                return p->children[j];
        node = p;
        --*level;
    }

    return NULL;
}

static
int trie_has_children_(trie_node_t* node)
{
    for(int i=0; i<ALPHABET_SIZE; ++i) {
        if (node->children[i])
            return 1;
    }
    return 0;
}

void* trie_remove(trie_t *t, const char *key)
{
    void* data = NULL;
    trie_node_t* node = trie_search_(&t->root, key);

    if (node == NULL)
        return NULL;

    node->is_end_of_word = 0;
    data = node->data;
    t->count--;

    for(trie_node_t* n = node; n->parent && !trie_has_children_(n); ) {
        trie_node_t* p = n->parent;
        int index = CHAR_TO_INDEX(n->character);
        p->children[index] = NULL;
        free(n);
        n = p;
    }
    return data;
}

trie_iterator_t* trie_dfs_iterator(trie_t* trie)
{
    trie_iterator_t* it = &trie->iter;
    it->current = &trie->root;
    it->level = 0;
    return it;
}

void* trie_iterator_next(trie_iterator_t *it)
{

    if (!it->current)
        return NULL;

    trie_node_t* next = it->current;
    do {
        next = trie_dfs_next_(next, &it->level);
    }
    while (next && !next->is_end_of_word);

    it->current = next;
    if (!next) 
        return NULL;
    return next->data;
}

void trie_iterator_destroy(trie_iterator_t *it)
{
    it->current = NULL;
    it->level = 0;
}



#if 0

static
void trie_print_(trie_t* t)
{
    char str[100];
    int level = 0;
    for (trie_node_t* r=trie_dfs_next_(&t->root, &level); r; r = trie_dfs_next_(r, &level)) {
        str[level-1] = r->character;
        str[level] = '\0';
        for (int i=0; i<level; ++i)
            printf(" ");
        printf("%s%s\n", str, r->is_end_of_word ? " *" : "");
    }
}

// Driver
int main()
{
    // Input keys (use only 'a' through 'z' and lower case)
    char keys[][8] = {"the", "a", "THERE", "answer", "any",
                     "by", "bye", "their", "theRE"};
 
    char output[][32] = {"Not present in trie", "Present in trie"};
 
 
    trie_t *t = trie_create();
 
    // Construct trie
    int i;
    for (i = 0; i < ARRAY_SIZE(keys); i++) {
        void* data = trie_find(t, keys[i]);
        if (data != NULL) {
            printf("%s has already been added as '%s'\n", keys[i], data);
        }
        else
            trie_insert(t, keys[i], my_strdup( keys[i] ));
        /* printf("Added %s, Now the tree contains %d elements\n", keys[i], t->count); */
    }

    void*k = trie_remove(t, "their");
    if (k) free(k);


    printf("Now the tree contains %d elements:\n", t->count);
    trie_iterator_t it = trie_dfs_iterator(t);
    for (void* n = trie_iterator_next(&it); n; n = trie_iterator_next(&it)) {
        printf("\t%s\n", n);
    }

    printf("The complete trie is:\n");
    trie_print_(t);

    // Search for different keys
    printf("%s --- %s\n", "the", output[trie_exists(t, "the")] );
    printf("%s --- %s\n", "these", output[trie_exists(t, "these")] );
    printf("%s --- %s\n", "there", output[trie_exists(t, "there")] );
    printf("%s --- %s\n", "tHEIr", output[trie_exists(t, "tHEIr")] );
    printf("%s --- %s\n", "thaw", output[trie_exists(t, "thaw")] );


    printf("Size of Trie node : %zu\n", sizeof(trie_node_t));


    return 0;
}
#endif
