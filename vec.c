#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "vec.h"

typedef struct vec {
    size_t size;
    size_t length;
    vec_entry arr[];
} Vec;


#define _to_vec(_ptr_) ((Vec *)((void *)(_ptr_) - offsetof(Vec, arr)))

static Vec* _vec_resize(Vec* v, size_t newSize)
{
    size_t entry_size = sizeof(v->arr[0]);
    if ((SIZE_MAX - sizeof(Vec)) / entry_size < newSize) {
        errno = ENOMEM;
        return NULL;
    }
    Vec* t = realloc(v, sizeof(Vec) + newSize * entry_size);
    if (!t) {
        perror("_vec_resize");
        free(v);
        return NULL;
    }
    /* fprintf(stderr, "Reallocating to size=%zu and &v=%p\n", newSize, t); */
    v = t;
    v->size = newSize;
    return v;
}

vec_entry* vec_create(size_t initialSize)
{
    const size_t DEFAULT_INIT_SIZE = 10;
    size_t initial = initialSize > DEFAULT_INIT_SIZE ? initialSize : DEFAULT_INIT_SIZE;
    Vec* v = NULL;
    v = _vec_resize(v, initial);
    if (!v) {
        return NULL;
    }
    v->length = 0;
    return v->arr;
}


void vec_free(vec_entry** a, void (*dtor)(vec_entry))
{
    if (*a) {
        Vec* v = _to_vec(*a);
        /* fprintf(stderr, "Freeing %p\n", v); */
        *a = NULL;
        if (dtor) {
            for(size_t i=0; i < v->length; ++i)
                dtor(v->arr[i]);
        }
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

vec_entry* vec_resize(vec_entry** a, size_t newSize)
{
    Vec* v = _to_vec(*a);
    if (v->size != newSize) {
        v = _vec_resize(v, newSize);
        if (!v)
            return NULL;
        *a = v->arr;
    }
    return *a;
}

vec_entry* vec_append(vec_entry** a, vec_entry elem)
{
    Vec* v = _to_vec(*a);
    size_t l = v->length;
    if (v->size < l+1) {
        size_t sz = (v->size * 3) >> 1;
        v = _vec_resize(v, sz);
        if (!v)
            return NULL;
    }
    v->arr[ v->length++ ] = elem;
    *a = v->arr;
    return v->arr;
}

vec_entry* vec_remove(vec_entry** a, size_t pos)
{
    Vec* v = _to_vec(*a);
    assert(pos < v->length);
    if (pos < v->length - 1) {
        memmove(v->arr + pos, v->arr+pos+1, (v->length - pos - 1)*sizeof *v->arr);
    }
    v->length--;
    if (3 * v->length < v->size) {
        size_t sz = v->size >> 1;
        v = _vec_resize(v, sz);
        if (!v)
            return NULL;
        *a = v->arr;
    }
    return v->arr;
}


