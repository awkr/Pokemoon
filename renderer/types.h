//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_TYPES_H
#define POKEMOON_TYPES_H

#include <vulkan/vulkan.h>

#define VK_CHECK(expr) ASSERT(expr == VK_SUCCESS);

struct Context {
  VkInstance             instance;
  VkAllocationCallbacks *allocator;
};

#endif // POKEMOON_TYPES_H
