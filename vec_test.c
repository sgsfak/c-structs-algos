#include <stdio.h>
#include "vec.h"

int main()
{
    vec_entry* v = vec_create(0);

    const int max = 20;

    vec_entry e;
    for (int i=0; i<max; ++i) {
        e.l = i;
        vec_append(&v, e);
    }
    printf("array size=%zu and len=%zu\n", vec_size(v), vec_length(v));

    for (int i=0; i<=2*max/3; ++i) {
        vec_remove(&v, 0);
    }
    printf("array size=%zu and len=%zu\n", vec_size(v), vec_length(v));

    for (int i = 0; i<vec_length(v); ++i)
        printf("array[%d]=%ld\n", i, v[i].l);

    vec_free(&v, NULL);
}

