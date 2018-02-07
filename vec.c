#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include "vec.h"

typedef struct vec {
    size_t size;
    size_t length;
    vec_entry arr[];
} Vec;


void debug(Vec* v)
{
    fprintf(stderr, 
            "Size: %zu, &v=%p &v->arr=%p diff=%ld offset=%ld\n",
            sizeof(Vec), v, v->arr, (intptr_t) v->arr - (intptr_t) v,
            offsetof(Vec, arr));
}

vec_entry* vec_create(size_t initialSize)
{
    const size_t DEFAULT_INIT_SIZE = 10;
    size_t initial = initialSize > DEFAULT_INIT_SIZE ? initialSize : DEFAULT_INIT_SIZE;
    Vec* v = malloc(sizeof(*v) + initial * sizeof(v->arr[0]));
    if (!v) {
        perror("vec_append");
        return NULL;
    }
    v->length = 0;
    v->size = initial;

    return v->arr;
}

#define _to_vec(_ptr_) ((Vec *)((void *)(_ptr_) - offsetof(Vec, arr)))

void vec_free(vec_entry** a)
{
    if (*a) {
        Vec* v = _to_vec(*a);
        /* fprintf(stderr, "Freeing %p\n", v); */
        *a = NULL;
        free(v);
    }
}

size_t vec_size(vec_entry* a)
{
    Vec* v = _to_vec(a);
    return v->size;
}

size_t vec_length(vec_entry* a)
{
    Vec* v = _to_vec(a);
    return v->length;
}

Vec* _vec_enlarge(Vec* v, size_t newSize)
{
    if (v->size < newSize) {
        size_t sz = (v->size * 3) >> 1;
        if (sz < newSize)
            sz = newSize;
        Vec* t = realloc(v, sizeof(*v) + sz * sizeof(v->arr[0]));
        if (!t) {
            perror("vec_enlarge");
            free(v);
            return NULL;
        }
        /* fprintf(stderr, "Reallocating to size=%zu and &v=%p\n", sz, t); */
        v = t;
        v->size = sz;
    }
    return v;
}

vec_entry* vec_increase_size(vec_entry* a, size_t nel)
{
    Vec* v = _to_vec(a);
    size_t newSize = v->size + nel;
    v = _vec_enlarge(v, newSize);
    if (!v)
        return NULL;
    return v->arr;
}

vec_entry* vec_ensure_capasity(vec_entry* a, size_t newSize)
{
    Vec* v = _to_vec(a);
    v = _vec_enlarge(v, newSize);
    if (!v)
        return NULL;
    return v->arr;
}

vec_entry* vec_append(vec_entry* a, vec_entry elem)
{
    Vec* v = _to_vec(a);
    size_t l = v->length;
    v = _vec_enlarge(v, l+1);
    if (!v)
        return NULL;
    v->arr[ v->length++ ] = elem;
    return v->arr;
}
        


