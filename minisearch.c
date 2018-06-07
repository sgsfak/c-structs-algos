#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "trie.h"


typedef struct {
    char* line;
    int line_nbr;
    int pos;
} word_pos_t;

typedef struct {
    char* word;
    int word_len;
    int occurences;
    word_pos_t* positions;
} word_info_t;

void free_word_info(word_info_t* info)
{
    free(info->word);
    free(info->positions);
    memset(info, 0, sizeof *info);
}

#define ISALPHA(c) (((c)>='a' && (c)<='z') || ((c)>='A' && (c)<='Z'))

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <docfile> <K>\n", argv[0]);
        return -1;
    }
    char* docfile = argv[1];
    /* int K = atoi(argv[2]); */

    FILE* fp = strcmp(docfile, "-") == 0 ? stdin : fopen(docfile, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open '%s' for reading\n", docfile);
        return -1;
    }
    trie_t* dic = trie_create();
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    int lines_size = 10;
    char** lines = calloc(lines_size, sizeof *lines);
    int total_lines = 0;
    for (int ln = 0; (linelen = getline(&line, &linecap, fp)) > 0; ++ln) {

        if (ln == lines_size) {
            lines_size = 3 * lines_size / 2;
            lines = realloc(lines, lines_size * sizeof *lines);
        }
        lines[ln] = strdup(line);
        ++total_lines;

        char* word = line;
        int word_len = 0;
        for (int i=0; i<linelen; ++i) {
            if (ISALPHA(line[i])) {
                ++word_len;
                continue;
            }

            if (word_len > 1) {
                line[i] = '\0';
                word_pos_t pos;
                pos.line = lines[ln];
                pos.line_nbr = ln+1;
                pos.pos = (int) (word - line);

                word_info_t* info = trie_find(dic, word);
                if (!info) {
                    info = calloc(1, sizeof *info);
                    info->word = strdup(word);
                    info->word_len = word_len;
                    trie_insert(dic, word, info);
                }
                ++info->occurences;
                info->positions = realloc(info->positions, info->occurences * sizeof *info->positions);
                info->positions[info->occurences-1] = pos;
            }
            word = line + i + 1;
            word_len = 0;
        }
    }

    free(line);

    if (fp != stdin)
        fclose(fp);

    printf("Found %d words:\n", trie_count(dic));
    trie_iterator_t* it = trie_dfs_iterator(dic);
    for (word_info_t* n = trie_iterator_next(it); n; n = trie_iterator_next(it)) {
        printf("\n---------------------------------------------------------------------------------\n");
        printf("%s [%d occurrences]\n", n->word, n->occurences);
        char prefix[30];
        for (int k=0; k<n->occurences; ++k) {
            word_pos_t pos = n->positions[k];
            snprintf(prefix, sizeof prefix - 1, "%d: ", pos.line_nbr);
            printf("%s%s", prefix, pos.line);
            int spaces = pos.pos + strlen(prefix);
            for (int j=0; j<spaces; ++j) printf(" ");
            for (int j=0; j<n->word_len; ++j) printf("^");
            printf("\n");
        }

        free_word_info(n);
    }
    trie_iterator_destroy(it);

    for (int i=0; i<total_lines; ++i)
        free(lines[i]);
    free(lines);
}
