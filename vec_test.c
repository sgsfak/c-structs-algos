#include <stdio.h>
#include "vec.h"

int main()
{
    vec_entry* v = vec_create(0);

    const int max = 20;

    vec_entry e;
    for (int i=0; i<max; ++i) {
        e.l = i;
        v = vec_append(v, e);
    }

    for (int i = 0; i<vec_length(v); ++i)
        printf("array[%d]=%ld\n", i, v[i].l);

    vec_free(&v);
}

