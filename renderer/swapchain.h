//
// Created by Hongjian Zhu on 2022/10/12.
//

#ifndef POKEMOON_SWAPCHAIN_H
#define POKEMOON_SWAPCHAIN_H

#include "renderer/types.h"

void swapchain_create(Context *context, u32 width, u32 height, Swapchain *outSwapchain);

void swapchain_recreate(Context *context, u32 width, u32 height, Swapchain *swapchain);

void swapchain_destroy(Context *context, Swapchain *swapchain);

bool swapchain_acquire_next_image(Context    *context,
                                  Swapchain  *swapchain,
                                  u32         timeout, // In nanoseconds
                                  VkSemaphore imageAvailableSemaphore,
                                  VkFence     fence,
                                  u32        *outImageIndex);

void swapchain_present(Context    *context,
                       Swapchain  *swapchain,
                       VkQueue     graphicsQueue,
                       VkQueue     presentQueue,
                       VkSemaphore renderCompleteSemaphore,
                       u32         presentImageIndex);

#endif // POKEMOON_SWAPCHAIN_H
