#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef union {
        int i;
        long l;
        long long ll;
        double d;
        void* p;
    } vec_entry;

    vec_entry* vec_create(size_t initialSize);
    void vec_free(vec_entry** a, void (*dtor_fn)(vec_entry));
    size_t vec_size(vec_entry* a);
    size_t vec_length(vec_entry* a);

    vec_entry* vec_resize(vec_entry** a, size_t newSize);
    vec_entry* vec_append(vec_entry** a, vec_entry elem);
    vec_entry* vec_remove(vec_entry** a, size_t pos);

#ifdef __cplusplus
}
#endif
