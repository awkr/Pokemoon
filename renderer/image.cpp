//
// Created by Hongjian Zhu on 2022/10/13.
//

#include "image.h"

void image_create(Context              *context,
                  VkImageType           imageType,
                  u32                   width,
                  u32                   height,
                  VkFormat              format,
                  VkImageTiling         tiling,
                  VkImageUsageFlags     usage,
                  VkMemoryPropertyFlags memoryProperty,
                  bool                  createView,
                  VkImageAspectFlags    aspect,
                  Image                *outImage) {
  outImage->width  = width;
  outImage->height = height;

  VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageCreateInfo.imageType         = imageType;
  imageCreateInfo.extent.width      = width;
  imageCreateInfo.extent.height     = height;
  imageCreateInfo.extent.depth      = 1;
  imageCreateInfo.mipLevels         = 4;
  imageCreateInfo.arrayLayers       = 1;
  imageCreateInfo.format            = format;
  imageCreateInfo.tiling            = tiling;
  imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.usage             = usage;
  imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateImage(
      context->device.handle, &imageCreateInfo, context->allocator, &outImage->handle));

  // Query memory requirements
  VkMemoryRequirements memoryRequirements;
  vkGetImageMemoryRequirements(context->device.handle, outImage->handle, &memoryRequirements);
  u32 index;
  ASSERT(
      context->query_memory_type_index(memoryRequirements.memoryTypeBits, memoryProperty, index));

  // Allocate memory
  VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocateInfo.allocationSize       = memoryRequirements.size;
  allocateInfo.memoryTypeIndex      = index;
  VK_CHECK(vkAllocateMemory(
      context->device.handle, &allocateInfo, context->allocator, &outImage->memory));

  // Bind the memory
  VK_CHECK(vkBindImageMemory(context->device.handle, outImage->handle, outImage->memory, 0));

  if (createView) { image_view_create(context, format, outImage, aspect); }
}

void image_destroy(Context *context, Image *image) {
  vkDestroyImageView(context->device.handle, image->view, context->allocator);
  vkFreeMemory(context->device.handle, image->memory, context->allocator);
  vkDestroyImage(context->device.handle, image->handle, context->allocator);
}

void image_view_create(Context *context, VkFormat format, Image *image, VkImageAspectFlags aspect) {
  image_view_create(context, format, image->handle, aspect, &image->view);
}

void image_view_create(Context           *context,
                       VkFormat           format,
                       VkImage            image,
                       VkImageAspectFlags aspect,
                       VkImageView       *view) {
  VkImageViewCreateInfo viewCreateInfo           = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewCreateInfo.image                           = image;
  viewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format                          = format;
  viewCreateInfo.subresourceRange.aspectMask     = aspect;
  viewCreateInfo.subresourceRange.baseMipLevel   = 0;
  viewCreateInfo.subresourceRange.levelCount     = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;
  viewCreateInfo.subresourceRange.layerCount     = 1;

  VK_CHECK(vkCreateImageView(context->device.handle, &viewCreateInfo, context->allocator, view));
}
