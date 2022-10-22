//
// Created by Hongjian Zhu on 2022/10/8.
//

#ifndef POKEMOON_MEMORY_H
#define POKEMOON_MEMORY_H

#include "defines.h"

static constexpr u16 MEMORY_TAG_COUNT = 14;
enum class MemoryTag : u16 {
  Unknown = 0,
  LINEAR_ALLOCATOR,
  Array,
  DArray,
  String,
  Texture,
  Event,
  Platform,
  Renderer,
  CommandBuffer,
  Semaphore,
  Fence,
  Image,
  ImageView,
  // When adding more variables, make sure to update MEMORY_TAG_COUNT
};

void memory_system_initialize();
void memory_system_shutdown();

void *memory_allocate(u64 size, MemoryTag tag);
void *memory_allocate(u64 stride, u32 n, MemoryTag tag);
void  memory_free(void *block, u64 size, MemoryTag tag);
void  memory_free(void *block, u64 stride, u64 n, MemoryTag tag);
void *memory_zero(void *block, u64 size);
void *memory_copy(void *dst, const void *src, u64 size);
void *memory_set(void *dst, i32 value, u64 size);

char *memory_get_usage();
u64 memory_get_alloc_count();

#define MEMORY_ALLOCATE(type, n, tag) (typeof(type) *) memory_allocate(sizeof(type) * (n), tag);
#define MEMORY_FREE(block, type, n, tag)                                                           \
  {                                                                                                \
    memory_free(block, sizeof(type), n, tag);                                                      \
    block = nullptr;                                                                               \
  }

#endif // POKEMOON_MEMORY_H
