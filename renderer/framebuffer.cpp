//
// Created by Hongjian Zhu on 2022/10/15.
//

#include "framebuffer.h"
#include "memory.h"

void framebuffer_create(Context     *context,
                        RenderPass  *renderPass,
                        u32          width,
                        u32          height,
                        u32          attachmentCount,
                        VkImageView *attachments,
                        Framebuffer *outFramebuffer) {
  outFramebuffer->attachmentCount = attachmentCount;
  const auto size                 = sizeof(VkImageView) * attachmentCount;
  outFramebuffer->attachments     = (VkImageView *) memory_allocate(size, MemoryTag::Renderer);
  memory_copy(outFramebuffer->attachments, attachments, size);
  outFramebuffer->renderPass = renderPass;

  VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
  createInfo.renderPass              = renderPass->handle;
  createInfo.attachmentCount         = attachmentCount;
  createInfo.pAttachments            = attachments;
  createInfo.width                   = width;
  createInfo.height                  = height;
  createInfo.layers                  = 1;

  VK_CHECK(vkCreateFramebuffer(
      context->device.handle, &createInfo, context->allocator, &outFramebuffer->handle));
}

void framebuffer_destroy(Context *context, Framebuffer *framebuffer) {
  vkDestroyFramebuffer(context->device.handle, framebuffer->handle, context->allocator);
  MEMORY_FREE(
      framebuffer->attachments, VkImageView, framebuffer->attachmentCount, MemoryTag::Renderer);
}
