//
// Created by Hongjian Zhu on 2022/10/14.
//

#include "command_buffer.h"
#include "renderer/types.h"

void command_buffer_allocate(Context       *context,
                             VkCommandPool  commandPool,
                             bool           isPrimary,
                             CommandBuffer *outCommandBuffer) {
  VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocateInfo.commandPool                 = commandPool;
  allocateInfo.level =
      isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  allocateInfo.commandBufferCount = 1;

  VK_CHECK(
      vkAllocateCommandBuffers(context->device.handle, &allocateInfo, &outCommandBuffer->handle));

  outCommandBuffer->state = CommandBufferState::Ready;
}

void command_buffer_free(Context       *context,
                         VkCommandPool  commandPool,
                         CommandBuffer *commandBuffer) {
  vkFreeCommandBuffers(context->device.handle, commandPool, 1, &commandBuffer->handle);
  commandBuffer->state = CommandBufferState::NotAllocated;
}

void command_buffer_begin(CommandBuffer *commandBuffer,
                          bool           isSingleUse,
                          bool           isRenderPassContinue,
                          bool           isSimultaneousUse) {
  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (isSingleUse) { beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; }
  if (isRenderPassContinue) { beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; }
  if (isSimultaneousUse) { beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; }

  VK_CHECK(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo));
  commandBuffer->state = CommandBufferState::Recording;
}

void command_buffer_end(CommandBuffer *commandBuffer) {
  VK_CHECK(vkEndCommandBuffer(commandBuffer->handle));
  commandBuffer->state = CommandBufferState::RecordingEnded;
}

void command_buffer_update_submitted(CommandBuffer *commandBuffer) {
  commandBuffer->state = CommandBufferState::Submitted;
}

void command_buffer_reset(CommandBuffer *commandBuffer) {
  commandBuffer->state = CommandBufferState::Ready;
}

void command_buffer_allocate_and_begin_single_use(Context       *context,
                                                  VkCommandPool  commandPool,
                                                  CommandBuffer *outCommandBuffer) {
  command_buffer_allocate(context, commandPool, true, outCommandBuffer);
  command_buffer_begin(outCommandBuffer, true, false, false);
}

void command_buffer_end_single_use(Context       *context,
                                   VkCommandPool  commandPool,
                                   CommandBuffer *commandBuffer,
                                   VkQueue        queue) {
  command_buffer_end(commandBuffer);

  VkSubmitInfo submitInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;
  VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, nullptr));

  VK_CHECK(vkQueueWaitIdle(queue));

  command_buffer_free(context, commandPool, commandBuffer);
}
