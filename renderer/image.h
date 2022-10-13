//
// Created by Hongjian Zhu on 2022/10/13.
//

#ifndef POKEMOON_IMAGE_H
#define POKEMOON_IMAGE_H

#include "renderer/types.h"

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
                  Image                *outImage);

void image_destroy(Context *context, Image *image);

void image_view_create(Context *context, VkFormat format, Image *image, VkImageAspectFlags aspect);
void image_view_create(
    Context *context, VkFormat format, VkImage image, VkImageAspectFlags aspect, VkImageView *view);

#endif // POKEMOON_IMAGE_H
