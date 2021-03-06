#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "lch_hmap.h"
#include "hfn.h"

#include "vec.h"

#include <sys/resource.h>
struct max_freq {
    lch_key_t key;
    long freq;
};

int find_max(lch_key_t key, lch_value_t e, void * arg)
{
    struct max_freq* a = arg;
    if (e.l > a->freq) {
        a->key = key;
        a->freq = e.l;
    }
    return 0;
}

/*
*  Suppress nuisance compiler warnings. Yes, each compiler can already 
*  do this, each differently! VC9 has its UNREFERENCED_PARAMETER(),
*  which is almost the same as the SUPPRESS_UNUSED_WARNING() below.
*
*  We append _UNUSED to the variable name, because the dumb gcc compiler
*  doesn't bother to tell you if you erroneously _use_ something flagged
*  with __attribute__((unused)). So we are forced to *mangle* the name.
*/
#if defined(__cplusplus)
#define UNUSED(x)       // = nothing
#elif defined(__GNUC__)
#define UNUSED(x)       x##_UNUSED __attribute__((unused))
#else
#define UNUSED(x)       x##_UNUSED
#endif

int print_entries(lch_key_t key, lch_value_t e, void* UNUSED(arg))
{
    printf("%s\t%ld\n", key, e.l);
    return 0;
}
int print_entries_head(lch_key_t key, lch_value_t e, void * arg)
{
    int * n = arg;
    printf("WORD %d: %s\t%ld\n", *n, key, e.l);
    return (++*n > 100) ? -1 : 0;
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

vec_entry* parseFile(const char* fn)
{
    FILE* fp = fopen(fn, "r");
    if (!fp) {
        perror("parseFile");
        exit(-1);
    }
    int s = 1000;
    vec_entry* lines = vec_create(s);
    float startTime = (float)clock()/CLOCKS_PER_SEC;
    char *word = NULL; 
    size_t linecap = 0;
    ssize_t len;

    while ((len = getline(&word, &linecap, fp)) != -1) {
        word[len-1] = '\0';
        /* const char* sep = " \t\n\x0B\f\r\"'.,();!-:?&|^&"; */
        const char* sep = " \t\n\x0B\f\r";
        for (char* str = strtok(word, sep); str ; str = strtok(NULL, sep)) {
            vec_entry e;
            e.p = strdup(str);
            vec_append(&lines, e);
        }
    }
    free(word);

    float endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Read %zu words in %.3f ms..\n", vec_length(lines), 1000*(endTime - startTime));
    fclose(fp);
    return lines;
}


void free_entry(vec_entry v)
{
    free(v.p);
}


int main(int argc, char* argv[])
{
    lch_hfn hfn = fnv32_hash;
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
            case 8:
                hfn = berkeley_hash;
                break;
            default:
                break;
        }
    }

    vec_entry* lines = parseFile("book.txt");

    lch_hmap_t* ht = ht_create(701, hfn);
    float startTime = (float)clock()/CLOCKS_PER_SEC;
    int k, n = vec_length(lines);
    for(k = 0; k<n; ++k) {
        char* word = lines[k].p;
        lch_value_t* e = ht_put(ht, word);
        e->l++;
    }
    float endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Hashed %d words in %.3f ms..\n", k, 1000*(endTime - startTime));

    vec_free(&lines, free_entry);

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    lch_hmap_stats_t stats = ht_stats(ht);
    printf("Gen. %llu Size: %u (total: %u) Load: %4.2f max bucket size=%d RSS=%.3lf MB\n", 
            stats.generation,
            stats.nbr_elems, stats.capacity,
            ht_load_factor(ht),
            stats.max_bucket_size,
            (double) usage.ru_maxrss/(1024.0 * 1024.0));

    struct max_freq tt = {};
    startTime = (float)clock()/CLOCKS_PER_SEC;
    ht_traverse(ht, find_max, &tt);
    endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Max frequency: %ld for word: '%s' (latency: %.3f ms)\n", tt.freq, tt.key, 1000*(endTime-startTime));
    char word[] = "The";
    startTime = (float)clock()/CLOCKS_PER_SEC;
    lch_value_t* v = ht_get(ht, word);
    endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("checking for existence of '%s' latency: %.3f ms Found %ld\n", word, 1000*(endTime-startTime), v->l);


    /*
        int nn = 0;
        ht_traverse_ordered(ht, print_entries_head, &nn);
    */

    ht_destroy(ht, NULL);

}
