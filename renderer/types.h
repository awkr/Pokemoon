//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_TYPES_H
#define POKEMOON_TYPES_H

#include "defines.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(expr) ASSERT(expr == VK_SUCCESS);

struct SwapchainSupport {
  VkSurfaceCapabilitiesKHR capabilities;
  u32                      formatCount;
  VkSurfaceFormatKHR      *formats;
  u32                      presentModeCount;
  VkPresentModeKHR        *presentModes;
};

struct Device {
  VkPhysicalDevice                 physicalDevice;
  VkPhysicalDeviceProperties       properties;
  VkPhysicalDeviceFeatures         features;
  VkPhysicalDeviceMemoryProperties memory;
  VkDevice                         handle; // Logical device
  u32                              graphicsQueueFamily;
  u32                              presentQueueFamily;
  u32                              computeQueueFamily;
  u32                              transferQueueFamily;
  SwapchainSupport                 swapchainSupport;

  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue transferQueue;
  VkQueue computeQueue;
};

struct Context {
  VkInstance             instance;
  VkAllocationCallbacks *allocator;
#ifdef DEBUG
  VkDebugUtilsMessengerEXT debugMessenger;
#endif
  VkSurfaceKHR surface;
  Device       device;
};

#endif // POKEMOON_TYPES_H
