//
// Created by Hongjian Zhu on 2022/10/9.
//

#ifndef POKEMOON_DARRAY_H
#define POKEMOON_DARRAY_H

#include "defines.h"

// Memory layout:
// u64   capacity = number of elements that can be held
// u64   length   = number of elements currently contained
// u64   stride   = size of each element in bytes
// void *elements

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR    2

#include "memory.h"

enum DArrayField { Capacity = 0, Length, Stride, Max };

void *darray_create(u64 capacity, u64 stride, MemoryTag tag = MemoryTag::DArray);
void  darray_destroy(void *array, MemoryTag tag = MemoryTag::DArray);

u64  darray_field_get(void *array, DArrayField field);
void darray_field_set(void *array, DArrayField field, u64 value);

void *darray_resize(void *array);

void *darray_push(void *array, const void *src);
void  darray_pop(void *array, void *dst);

void *darray_insert_at(void *array, u64 index, const void *src);
void *darray_pop_at(void *array, u64 index, void *dst = nullptr);

void darray_clear(void *array);

u64 darray_capacity(void *array);
u64 darray_length(void *array);
u64 darray_stride(void *array);

void darray_length_set(void *array, u64 n);

#define DARRAY_CREATE(type) (typeof(type) *) darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define DARRAY_CREATE_TAG(type, tag)                                                               \
  (typeof(type) *) darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type), tag)

#define DARRAY_RESERVE(type, capacity) darray_create(capacity, sizeof(type))

#define DARRAY_PUSH(array, value) array = (typeof(value) *) darray_push(array, &value);

#define DARRAY_INSERT_AT(array, index, value)                                                      \
  array = (typeof(value)) darray_insert_at(array, index, &value);

#endif // POKEMOON_DARRAY_H
