//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "memory.h"
#include "logging.h"
#include "platform.h"
#include <cstdio>
#include <cstring>

struct MemoryStats {
  u64 totalAllocated;
  u64 taggedAllocations[(u16) MemoryTag::Max];
};

static CString memoryTags[(u16) MemoryTag::Max] = {
    "Unknown",
    "LINEAR_ALLOCATOR",
    "Array",
    "DArray",
    "String",
    "Texture",
    "Event",
    "Platform",
    "Renderer",
    "CommandBuffer",
    "Semaphore",
    "Fence",
    "Image",
    "ImageView",
};

static MemoryStats memoryStats{};

void memory_initialize() { platform_zero_memory(&memoryStats, sizeof(MemoryStats)); }

void memory_shutdown() {}

void *memory_allocate(u64 size, MemoryTag tag) {
  if (tag == MemoryTag::Unknown) { LOG_WARN("Allocating unknown memory"); }
  memoryStats.totalAllocated += size;
  memoryStats.taggedAllocations[(u16) tag] += size;
  void *block = platform_allocate(size);
  platform_zero_memory(block, size);
  return block;
}

void *memory_allocate(u64 stride, u32 n, MemoryTag tag) { return memory_allocate(stride * n, tag); }

void memory_free(void *block, u64 size, MemoryTag tag) {
  if (tag == MemoryTag::Unknown) { LOG_WARN("Freeing unknown memory"); }
  memoryStats.totalAllocated -= size;
  memoryStats.taggedAllocations[(u16) tag] -= size;
  platform_free(block);
}

void memory_free(void *block, u64 stride, u64 n, MemoryTag tag) {
  memory_free(block, stride * n, tag);
}

void *memory_zero(void *block, u64 size) { return platform_zero_memory(block, size); }

void *memory_copy(void *dst, const void *src, u64 size) {
  return platform_copy_memory(dst, src, size);
}

void *memory_set(void *dst, i32 value, u64 size) { return platform_set_memory(dst, value, size); }

char *memory_get_usage() {
  char buffer[2 * KiB] = "System memory usage:";
  u64  offset          = strlen(buffer);
  for (u16 i = 0; i < (u16) MemoryTag::Max; ++i) {
    char unit[4] = "XiB";
    f64  amount  = 0.0f;
    if (memoryStats.taggedAllocations[i] >= GiB) {
      unit[0] = 'G';
      amount  = (f64) memoryStats.taggedAllocations[i] / (f64) GiB;
    } else if (memoryStats.taggedAllocations[i] >= MiB) {
      unit[0] = 'M';
      amount  = (f64) memoryStats.taggedAllocations[i] / (f64) MiB;
    } else if (memoryStats.taggedAllocations[i] >= KiB) {
      unit[0] = 'K';
      amount  = (f64) memoryStats.taggedAllocations[i] / (f64) KiB;
    } else {
      unit[0] = 'B';
      unit[1] = 0;
      amount  = (f64) memoryStats.taggedAllocations[i];
    }
    auto length = snprintf(
        buffer + offset, 2 * KiB - offset, "\n%-16s: %6.2f %s", memoryTags[i], amount, unit);
    offset += length;
  }
  return strdup(buffer);
}
