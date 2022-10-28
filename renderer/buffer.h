//
// Created by Hongjian Zhu on 2022/10/26.
//

#pragma once

#include "renderer/types.h"

bool buffer_create(Context           *context,
                   u64                size,
                   VkBufferUsageFlags usage,
                   u32                memoryPropertyBits,
                   bool               bindOnCreate,
                   Buffer            *out);

void buffer_destroy(Context *context, Buffer *buffer);

bool buffer_resize(
    Context *context, u64 size, Buffer *buffer, VkQueue queue, VkCommandPool commandPool);

void buffer_bind(Context *context, Buffer *buffer, u64 offset);

void *buffer_lock(Context *context, Buffer *buffer, u64 offset, u64 size, VkMemoryMapFlags flags);

void buffer_unlock(Context *context, Buffer *buffer);

void buffer_load(Context         *context,
                 Buffer          *buffer,
                 u64              offset,
                 u64              size,
                 VkMemoryMapFlags flags,
                 const void      *src);

void buffer_copy_to(Context      *context,
                    VkCommandPool commandPool,
                    VkFence       fence,
                    VkQueue       queue,
                    VkBuffer      src,
                    u64           srcOffset,
                    VkBuffer      dst,
                    u64           dstOffset,
                    u64           size);
