//
// Created by Hongjian Zhu on 2022/10/12.
//

#include "renderer/swapchain.h"
#include "defines.h"
#include "logging.h"
#include "memory.h"
#include "renderer/device.h"
#include "renderer/image.h"
#include "renderer/types.h"

void create(Context *context, u32 width, u32 height, Swapchain *out);
void destroy(Context *context, Swapchain *swapchain);

void swapchain_create(Context *context, u32 width, u32 height, Swapchain *outSwapchain) {
  create(context, width, height, outSwapchain);
}

void swapchain_recreate(Context *context, u32 width, u32 height, Swapchain *swapchain) {
  destroy(context, swapchain);
  create(context, width, height, swapchain);
}

void swapchain_destroy(Context *context, Swapchain *swapchain) { destroy(context, swapchain); }

bool swapchain_acquire_next_image(Context    *context,
                                  Swapchain  *swapchain,
                                  u32         timeout,
                                  VkSemaphore imageAvailableSemaphore,
                                  VkFence     fence,
                                  u32        *outImageIndex) {
  auto result = vkAcquireNextImageKHR(context->device.handle,
                                      swapchain->handle,
                                      timeout,
                                      imageAvailableSemaphore,
                                      fence,
                                      outImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    swapchain_recreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
    return false; // Boot out of the render loop
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    LOG_ERROR("Failed to acquire swapchain image");
    return false;
  }
  return true;
}

void swapchain_present(Context    *context,
                       Swapchain  *swapchain,
                       VkQueue     graphicsQueue,
                       VkQueue     presentQueue,
                       VkSemaphore renderCompleteSemaphore,
                       u32         presentImageIndex) {
  // Return the image to the swapchain for presentation
  VkPresentInfoKHR presentInfo   = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores    = &renderCompleteSemaphore;
  presentInfo.swapchainCount     = 1;
  presentInfo.pSwapchains        = &swapchain->handle;
  presentInfo.pImageIndices      = &presentImageIndex;
  presentInfo.pResults           = nullptr;

  auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    swapchain_recreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
  } else if (result != VK_SUCCESS) {
    LOG_ERROR("Failed to present swapchain image");
  }
}

void create(Context *context, u32 width, u32 height, Swapchain *out) {
  device_query_swapchain_support(
      context->device.physicalDevice, context->surface, context->device.swapchainSupport);

  const auto &swapchainSupport = context->device.swapchainSupport;

  // Choose a surface format
  bool found = false;
  for (u32 i = 0; i < swapchainSupport.formatCount; ++i) {
    const auto &format = swapchainSupport.formats[i];
    // Preferred format
    if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      out->imageFormat = format;
      found            = true;
      break;
    }
  }
  if (!found) { out->imageFormat = swapchainSupport.formats[0]; }

  VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (u32 i = 0; i < swapchainSupport.presentModeCount; ++i) {
    auto mode = swapchainSupport.presentModes[i];
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      presentMode = mode;
      break;
    }
  }

  VkExtent2D imageExtent = {width, height};
  if (swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
    imageExtent = swapchainSupport.capabilities.currentExtent;
  }
  const auto min     = swapchainSupport.capabilities.minImageExtent;
  const auto max     = swapchainSupport.capabilities.maxImageExtent;
  imageExtent.width  = CLAMP(imageExtent.width, min.width, max.width);
  imageExtent.height = CLAMP(imageExtent.height, min.height, max.height);

  u32 imageCount = swapchainSupport.capabilities.minImageCount + 1;
  if (swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  // if equals 2, means triple buffer: at a time, one is presenting, two can be written to
  out->maxFramesInFlight = imageCount - 1;

  // Swapchain create info
  VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  swapchainCreateInfo.surface                  = context->surface;
  swapchainCreateInfo.minImageCount            = imageCount;
  swapchainCreateInfo.imageFormat              = out->imageFormat.format;
  swapchainCreateInfo.imageColorSpace          = out->imageFormat.colorSpace;
  swapchainCreateInfo.imageExtent              = imageExtent;
  swapchainCreateInfo.imageArrayLayers         = 1;
  swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;

  u32 queueFamilyIndices[4] = {
      context->device.graphicsQueueFamily,
      context->device.presentQueueFamily,
      context->device.computeQueueFamily,
      context->device.transferQueueFamily,
  };
  swapchainCreateInfo.queueFamilyIndexCount = 4;
  swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;

  swapchainCreateInfo.preTransform   = swapchainSupport.capabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode    = presentMode;
  swapchainCreateInfo.clipped        = VK_TRUE;
  swapchainCreateInfo.oldSwapchain   = nullptr;

  // Create swapchain
  VK_CHECK(vkCreateSwapchainKHR(
      context->device.handle, &swapchainCreateInfo, context->allocator, &out->handle));

  LOG_DEBUG("Swapchain created");

  context->currentFrame = 0; // Start with a zero frame index

  // Images
  VK_CHECK(vkGetSwapchainImagesKHR(context->device.handle, out->handle, &out->imageCount, nullptr));
  if (!out->images) {
    out->images = MEMORY_ALLOCATE(VkImage, out->imageCount, MemoryTag::Renderer);
  }
  VK_CHECK(
      vkGetSwapchainImagesKHR(context->device.handle, out->handle, &out->imageCount, out->images));

  // Views
  if (!out->views) {
    out->views = MEMORY_ALLOCATE(VkImageView, out->imageCount, MemoryTag::Renderer);
  }
  for (u32 i = 0; i < out->imageCount; ++i) {
    image_view_create(context,
                      out->imageFormat.format,
                      out->images[i],
                      VK_IMAGE_ASPECT_COLOR_BIT,
                      &out->views[i]);
  }

  // Depth resources
  ASSERT(device_detect_depth_format(&context->device));
  // Create depth image and its view
  image_create(context,
               VK_IMAGE_TYPE_2D,
               imageExtent.width,
               imageExtent.height,
               context->device.depthFormat,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               true,
               VK_IMAGE_ASPECT_DEPTH_BIT,
               &out->depthAttachment);
}

void destroy(Context *context, Swapchain *swapchain) {
  image_destroy(context, &swapchain->depthAttachment);
  for (u32 i = 0; i < swapchain->imageCount; ++i) {
    vkDestroyImageView(context->device.handle, swapchain->views[i], context->allocator);
  }
  MEMORY_FREE(swapchain->views, VkImageView, swapchain->imageCount, MemoryTag::Renderer);
  MEMORY_FREE(swapchain->images, VkImage, swapchain->imageCount, MemoryTag::Renderer);
  vkDestroySwapchainKHR(context->device.handle, swapchain->handle, context->allocator);
}
