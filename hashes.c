#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "lch_hmap.h"
#include "hfn.h"

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

char** parseFile(const char* fn)
{
    FILE* fp = fopen(fn, "r");
    if (!fp) {
        perror("main");
        exit(-1);
    }
    int s = 1000;
    char** lines = malloc(s*sizeof(*lines));
    int k = 0;
    float startTime = (float)clock()/CLOCKS_PER_SEC;
    char *word = NULL; 
    size_t linecap = 0;
    ssize_t len;

    while ((len = getline(&word, &linecap, fp)) != -1) {
        word[len-1] = '\0';
        const char* sep = " \t\n\x0B\f\r\"'";
        for (char* str = strtok(word, sep); str ; str = strtok(NULL, sep)) {
            if (k == s) {
                s += 1000;
                lines = realloc(lines, s * sizeof(*lines));
            }
            lines[k++] = strdup(str);
        }
    }
    free(word);
    if (k == s) {
        s += 1;
        lines = realloc(lines, s * sizeof(*lines));
    }
    lines[k] = NULL;

    float endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Read %d words in %.3f ms..\n", k, 1000*(endTime - startTime));
    fclose(fp);
    return lines;
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
            default:
                break;
        }
    }

    char** lines = parseFile("book.txt");

    lch_hmap_t* ht = ht_create(100, hfn);
    float startTime = (float)clock()/CLOCKS_PER_SEC;
    int k;
    for(k = 0; lines[k]; ++k) {
        char* word = lines[k];
        lch_value_t* e = ht_put(ht, word);
        e->l++;
    }
    float endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Hashed %d words in %.3f ms..\n", k, 1000*(endTime - startTime));

    for(int j = 0; lines[j]; ++j) {
        free(lines[j]);
    }
    free(lines);

    lch_hmap_stats_t stats = ht_stats(ht);
    printf("Gen. %llu Size: %u (total: %u) Load: %4.2f max bucket size=%d\n", 
            stats.generation,
            stats.nbr_elems, stats.capacity,
            ht_load_factor(ht),
            stats.max_bucket_size);

    struct max_freq tt = {0};
    ht_traverse(ht, find_max, &tt);
    printf("Max frequency: %ld for word: '%s'\n", tt.freq, tt.key);
    ht_destroy(ht, NULL);

}
