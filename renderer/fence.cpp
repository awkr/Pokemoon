//
// Created by Hongjian Zhu on 2022/10/15.
//

#include "fence.h"

void fence_create(Context *context, bool isSignaled, Fence *outFence) {
  VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  if (isSignaled) { createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; }

  VK_CHECK(
      vkCreateFence(context->device.handle, &createInfo, context->allocator, &outFence->handle));

  outFence->isSignaled = isSignaled;
}

void fence_destroy(Context *context, Fence *fence) {
  vkDestroyFence(context->device.handle, fence->handle, context->allocator);
  fence->isSignaled = false;
}

bool fence_wait(Context *context, Fence *fence, u64 timeoutNs) {
  if (fence->isSignaled) { return true; }
  auto result = vkWaitForFences(context->device.handle, 1, &fence->handle, true, timeoutNs);
  if (result == VK_SUCCESS) {
    fence->isSignaled = true;
    return true;
  }
  return false;
}

void fence_reset(Context *context, Fence *fence) {
  if (fence->isSignaled) {
    VK_CHECK(vkResetFences(context->device.handle, 1, &fence->handle));
    fence->isSignaled = false;
  }
}
