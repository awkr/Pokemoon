//
// Created by Hongjian Zhu on 2022/10/26.
//

#include "renderer/buffer.h"
#include "memory.h"
#include "renderer/command_buffer.h"

bool buffer_create(Context           *context,
                   u64                size,
                   VkBufferUsageFlags usage,
                   u32                memoryPropertyBits,
                   bool               bindOnCreate,
                   Buffer            *out) {
  out->size               = size;
  out->usage              = usage;
  out->memoryPropertyBits = memoryPropertyBits;

  VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.size               = size;
  bufferCreateInfo.usage              = usage;
  bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue

  VK_CHECK(
      vkCreateBuffer(context->device.handle, &bufferCreateInfo, context->allocator, &out->handle));

  // Gather memory requirements
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(context->device.handle, out->handle, &memoryRequirements);

  ASSERT(context->query_memory_type_index(
      memoryRequirements.memoryTypeBits, out->memoryPropertyBits, out->memoryTypeIndex));

  // Allocate memory
  VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocateInfo.allocationSize       = memoryRequirements.size;
  allocateInfo.memoryTypeIndex      = out->memoryTypeIndex;

  VK_CHECK(
      vkAllocateMemory(context->device.handle, &allocateInfo, context->allocator, &out->memory));

  if (bindOnCreate) { buffer_bind(context, out, 0); }

  return true;
}

void buffer_destroy(Context *context, Buffer *buffer) {
  vkFreeMemory(context->device.handle, buffer->memory, context->allocator);
  vkDestroyBuffer(context->device.handle, buffer->handle, context->allocator);
}

bool buffer_resize(
    Context *context, u64 size, Buffer *buffer, VkQueue queue, VkCommandPool commandPool) {
  // Create a new buffer
  VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.size               = size;
  bufferCreateInfo.usage              = buffer->usage;
  bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE; // Only used in one queue

  VkBuffer handle;
  VK_CHECK(vkCreateBuffer(context->device.handle, &bufferCreateInfo, context->allocator, &handle));

  // Gather memory requirements
  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(context->device.handle, handle, &memoryRequirements);

  VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocateInfo.allocationSize       = memoryRequirements.size;
  allocateInfo.memoryTypeIndex      = buffer->memoryTypeIndex;

  // Allocate the memory
  VkDeviceMemory memory;
  VK_CHECK(vkAllocateMemory(context->device.handle, &allocateInfo, context->allocator, &memory));

  // Bind the new buffer's memory
  VK_CHECK(vkBindBufferMemory(context->device.handle, handle, memory, 0));

  // Copy over the data
  buffer_copy_to(context, commandPool, nullptr, queue, buffer->handle, 0, handle, 0, buffer->size);

  vkDeviceWaitIdle(context->device.handle);

  // Destroy the old
  buffer_destroy(context, buffer);

  // Set new properties
  buffer->size   = size;
  buffer->memory = memory;
  buffer->handle = handle;

  return true;
}

void buffer_bind(Context *context, Buffer *buffer, u64 offset) {
  VK_CHECK(vkBindBufferMemory(context->device.handle, buffer->handle, buffer->memory, offset));
}

void *buffer_lock(Context *context, Buffer *buffer, u64 offset, u64 size, VkMemoryMapFlags flags) {
  void *data;
  VK_CHECK(vkMapMemory(context->device.handle, buffer->memory, offset, size, flags, &data));
  return data;
}

void buffer_unlock(Context *context, Buffer *buffer) {
  vkUnmapMemory(context->device.handle, buffer->memory);
}

void buffer_load(Context         *context,
                 Buffer          *buffer,
                 u64              offset,
                 u64              size,
                 VkMemoryMapFlags flags,
                 const void      *src) {
  auto dst = buffer_lock(context, buffer, offset, size, flags);
  memory_copy(dst, src, size);
  buffer_unlock(context, buffer);
}

void buffer_copy_to(Context      *context,
                    VkCommandPool commandPool,
                    VkFence       fence,
                    VkQueue       queue,
                    VkBuffer      src,
                    u64           srcOffset,
                    VkBuffer      dst,
                    u64           dstOffset,
                    u64           size) {
  vkQueueWaitIdle(queue);

  // Create a one-time-use command buffer
  CommandBuffer commandBuffer{};
  command_buffer_allocate_and_begin_single_use(context, commandPool, &commandBuffer);

  // Prepare the copy command and add it to the command buffer
  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size      = size;

  vkCmdCopyBuffer(commandBuffer.handle, src, dst, 1, &copyRegion);

  // Submit the buffer for execution and wait for it to complete
  command_buffer_end_single_use(context, commandPool, &commandBuffer, queue);
}
