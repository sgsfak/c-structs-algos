#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <search.h>

struct Entry {
    char* word;
    int freq;
};


struct Entry* entry_new(const char* word)
{
    struct Entry* e = malloc(sizeof (struct Entry));
    if (e) {
        e->word = strdup(word);
        e->freq = 1;
    }
    return e;
}

void entry_free(void* k)
{
    struct Entry * e = k;
    if (e) {
        free(e->word);
        free(e);
    }
}

int entry_cmp(const void* k1, const void* k2)
{
    struct Entry const * e1 = k1;
    struct Entry const * e2 = k2;

    return strcmp(e1->word, e2->word);
}

char* max_word;
int max_freq;

void walk_entry(const void *mt_data,VISIT x,int level)
{
    struct Entry *m = *(struct Entry **)mt_data;
    printf("<%d>Walk on node %s %d %s  \n",
        level,
        x == preorder?"preorder":
        x == postorder?"postorder":
        x == endorder?"endorder":
        x == leaf?"leaf":
        "unknown",
        m->freq,m->word);
    if (m->freq > max_freq) {
        max_freq = m->freq;
        max_word = m->word;
    }
    return;
}

int main(int argc, char* argv[])
{
    void* rootp = NULL;


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
        struct Entry* e = entry_new(word);

        struct Entry** ee = (struct Entry**) tsearch(e, &rootp, entry_cmp);
        if (e && *ee != e) {
            entry_free(e);
            (*ee)->freq++;
        }
    }


    fclose(fp);
    twalk(rootp, walk_entry);
    printf("%s:%d\n", max_word, max_freq);
}
