//
// Created by Hongjian Zhu on 2022/10/20.
//

#include "LinearAllocator.h"
#include "logging.h"
#include "memory.h"

LinearAllocator::LinearAllocator(u64 size, void *memory) : _size(size) {
  if (memory) {
    _memory = memory;
  } else {
    _memory = memory_allocate(size, MemoryTag::LINEAR_ALLOCATOR);
    _owner  = true;
  }
}

LinearAllocator::~LinearAllocator() {
  if (_memory && _owner) { memory_free(_memory, _size, MemoryTag::LINEAR_ALLOCATOR); }
}

void *LinearAllocator::alloc(u64 size) {
  if (!_memory) {
    LOG_ERROR("[LinearAllocator] Not initialized.");
    return nullptr;
  }
  if (_allocated + size > _size) {
    auto remaining = _size - _allocated;
    LOG_ERROR("[LinearAllocator] Try to alloc %llu B, yet %llu B remaining.", size, remaining);
    return nullptr;
  }

  auto block = (void *) (uintptr_t(_memory) + _allocated);
  _allocated += size;
  return block;
}

void LinearAllocator::reset() {
  _allocated = 0;
  if (_memory) { memory_zero(_memory, _size); }
}
