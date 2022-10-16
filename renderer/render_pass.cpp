//
// Created by Hongjian Zhu on 2022/10/13.
//

#include "render_pass.h"
#include "memory.h"
#include "renderer/types.h"
#include <vector>

void render_pass_create(Context        *context,
                        const VkRect2D &rendArea,
                        const Vec4     &clearColor,
                        f32             depth,
                        u32             stencil,
                        RenderPass     *outRenderPass) {
  outRenderPass->renderArea = rendArea;
  outRenderPass->clearColor = clearColor;
  outRenderPass->depth      = depth;
  outRenderPass->stencil    = stencil;

  // Main subpass
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Attachments
  std::vector<VkAttachmentDescription> attachmentDescriptions(2);

  // Color attachment
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format                  = context->swapchain.imageFormat.format;
  colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout =
      VK_IMAGE_LAYOUT_UNDEFINED; // Do not expect any particular layout before render pass starts
  colorAttachment.finalLayout =
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Transitioned to after the render pass

  attachmentDescriptions[0] = colorAttachment;

  VkAttachmentReference colorAttachmentReference = {};
  colorAttachmentReference.attachment            = 0; // Attachment description array index
  colorAttachmentReference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorAttachmentReference;

  // Depth attachment if there is one
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format                  = context->device.depthFormat;
  depthAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  attachmentDescriptions[1] = depthAttachment;

  VkAttachmentReference depthAttachmentReference = {};
  depthAttachmentReference.attachment            = 1;
  depthAttachmentReference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  subpass.pDepthStencilAttachment = &depthAttachmentReference;

  // Input from a shader
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments    = nullptr;

  // Attachments used for multisampling color attachments
  subpass.pResolveAttachments = nullptr;

  // Attachments not used in this subpass, but must be preserved for the next
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments    = nullptr;

  // Render pass dependencies
  VkSubpassDependency dependency = {};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask       = 0;
  dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  // Render pass creation
  VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassCreateInfo.attachmentCount        = attachmentDescriptions.size();
  renderPassCreateInfo.pAttachments           = attachmentDescriptions.data();
  renderPassCreateInfo.subpassCount           = 1;
  renderPassCreateInfo.pSubpasses             = &subpass;
  renderPassCreateInfo.dependencyCount        = 1;
  renderPassCreateInfo.pDependencies          = &dependency;

  VK_CHECK(vkCreateRenderPass(
      context->device.handle, &renderPassCreateInfo, context->allocator, &outRenderPass->handle));
}

void render_pass_destroy(Context *context, RenderPass *renderPass) {
  vkDestroyRenderPass(context->device.handle, renderPass->handle, context->allocator);
}

void render_pass_begin(CommandBuffer *commandBuffer,
                       RenderPass    *renderPass,
                       VkFramebuffer  framebuffer) {
  VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  beginInfo.renderPass            = renderPass->handle;
  beginInfo.framebuffer           = framebuffer;
  beginInfo.renderArea            = renderPass->renderArea;

  std::vector<VkClearValue> clearValues(2);
  memory_copy(clearValues[0].color.float32, &renderPass->clearColor[0], sizeof(Vec4));
  clearValues[1].depthStencil.depth   = renderPass->depth;
  clearValues[1].depthStencil.stencil = renderPass->stencil;

  beginInfo.clearValueCount = clearValues.size();
  beginInfo.pClearValues    = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
  commandBuffer->state = CommandBufferState::InRenderPass;
}

void render_pass_end(CommandBuffer *commandBuffer, RenderPass *renderPass) {
  vkCmdEndRenderPass(commandBuffer->handle);
  commandBuffer->state = CommandBufferState::Recording;
}
