//
// Created by Hongjian Zhu on 2022/10/20.
//

#pragma once

#include "defines.h"

class Allocator {};

class LinearAllocator : public Allocator {
public:
  explicit LinearAllocator(u64 size, void *memory = nullptr);

  ~LinearAllocator();

  void *alloc(u64 size);

  void reset();

private:
  u64   _size      = 0;
  u64   _allocated = 0;
  void *_memory    = nullptr;
  bool  _owner     = false;
};
