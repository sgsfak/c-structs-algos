#ifndef TRIE_H
#define TRIE_H
 

typedef struct trie_t trie_t;
typedef struct trie_iterator_t trie_iterator_t;

trie_t *trie_create(void);
int trie_count(trie_t *t);

/* this function inserts a `key` in the Trie with the given `data`
 * payload. If there's already an entry in the Trie with the given
 * `key`, it returns the exising `data` (i.e. it does not overwrite
 * it). Otherwise, it creates a new entry, assigns its payload to `data`
 * and returns the passed `data`
 */
void* trie_insert(trie_t *root, const char *key, void* data);

/* trie_find searches for the given key in the Trie. If it is found
 * it returns its `data` pointer else returns NULL. Please note that
 * if you have inserted the `key` with a NULL `data` in the first place, 
 * there's no way to discriminate whether it's indeed in the Trie or not.
 */ 
void* trie_find(trie_t *root, const char *key);

/* checks if the given `key` exists in the trie */
int trie_exists(trie_t *root, const char *key);

/* removes (deletes) the given `key` from the trie
 * and returns its `data`
 */
void* trie_remove(trie_t *root, const char *key);

/* Get an "iterator" that provides access to the contents
 * of the Trie in pre-order depth first traversal
 */
trie_iterator_t* trie_dfs_iterator(trie_t* trie);

/* Get the next item from the iterator. If there 
 * no other items, it returns NULL
 */
void* trie_iterator_next(trie_iterator_t* it);

/* Destroy the iterator when you don't need
 * it any more
 */
void trie_iterator_destroy(trie_iterator_t* it);

#endif
