#pragma once

#include <stddef.h>

typedef union {
    long l;
    double d;
    void* p;
} vec_entry;

extern vec_entry* vec_create(size_t initialSize);
extern void vec_free(vec_entry** a);
extern size_t vec_size(vec_entry* a);
extern size_t vec_length(vec_entry* a);

extern vec_entry* vec_increase_size(vec_entry* a, size_t nel);

extern vec_entry* vec_ensure_capasity(vec_entry* a, size_t newSize);
extern vec_entry* vec_append(vec_entry* a, vec_entry elem);
