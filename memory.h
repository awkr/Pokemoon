//
// Created by Hongjian Zhu on 2022/10/8.
//

#ifndef POKEMOON_MEMORY_H
#define POKEMOON_MEMORY_H

#include "defines.h"

enum class MemoryTag {
  Unknown = 0,
  Array,
  DArray,
  String,
  Texture,
  Event,
  Platform,
  Renderer,
  Max,
};

void memory_initialize();
void memory_shutdown();

void *memory_allocate(u64 size, MemoryTag tag);
void *memory_allocate(u64 stride, u32 n, MemoryTag tag);
void  memory_free(void *block, u64 size, MemoryTag tag);
void  memory_free(void *block, u64 stride, u64 n, MemoryTag tag);
void *memory_zero(void *block, u64 size);
void *memory_copy(void *dst, const void *src, u64 size);
void *memory_set(void *dst, i32 value, u64 size);

char *memory_get_usage();

#define MEMORY_ALLOCATE(type, n, tag)    (typeof(type) *) memory_allocate(sizeof(type) * (n), tag);
#define MEMORY_FREE(block, type, n, tag) memory_free(block, sizeof(type), n, tag);

#endif // POKEMOON_MEMORY_H
