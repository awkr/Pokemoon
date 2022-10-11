//
// Created by Hongjian Zhu on 2022/10/11.
//

#ifndef POKEMOON_DEVICE_H
#define POKEMOON_DEVICE_H

#include "renderer/types.h"

void device_create(Context *context);
void device_destroy(Context *context);

void device_query_swapchain_support(VkPhysicalDevice  physicalDevice,
                                    VkSurfaceKHR      surface,
                                    SwapchainSupport &out);

#endif // POKEMOON_DEVICE_H
