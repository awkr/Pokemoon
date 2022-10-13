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

  VkFormat depthFormat;
};

struct Image {
  VkImage        handle;
  VkDeviceMemory memory;
  VkImageView    view;
  u32            width;
  u32            height;
};

struct Swapchain {
  VkSurfaceFormatKHR imageFormat;
  u8                 maxFramesInFlight;
  VkSwapchainKHR     handle;
  u32                imageCount;
  VkImage           *images;
  VkImageView       *views;
  Image              depthAttachment;
};

struct Context {
  VkInstance             instance;
  VkAllocationCallbacks *allocator;
#ifdef DEBUG
  VkDebugUtilsMessengerEXT debugMessenger;
#endif
  VkSurfaceKHR surface;
  Device       device;

  u32 framebufferWidth;
  u32 framebufferHeight;

  Swapchain swapchain;
  u32       imageIndex;
  u64       currentFrame;
  bool      recreatingSwapchain;

  bool (*query_memory_type_index)(u32                   requiredType,
                                  VkMemoryPropertyFlags requiredProperty,
                                  u32                  &index);
};

#endif // POKEMOON_TYPES_H
