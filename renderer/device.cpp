//
// Created by Hongjian Zhu on 2022/10/11.
//

#include "device.h"
#include "memory.h"

void select_physical_device(Context *context);

void device_create(Context *context) { select_physical_device(context); }

void device_destroy(Context *context) {
  MEMORY_FREE(context->device.swapchainSupport.formats,
              VkSurfaceFormatKHR,
              context->device.swapchainSupport.formatCount,
              MemoryTag::Renderer);
  MEMORY_FREE(context->device.swapchainSupport.presentModes,
              VkPresentModeKHR,
              context->device.swapchainSupport.presentModeCount,
              MemoryTag::Renderer);
}

void device_query_swapchain_support(VkPhysicalDevice  physicalDevice,
                                    VkSurfaceKHR      surface,
                                    SwapchainSupport &out) {
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &out.capabilities));

  VK_CHECK(
      vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &out.formatCount, nullptr));
  ASSERT(out.formatCount > 0);
  if (!out.formats) {
    out.formats = MEMORY_ALLOCATE(VkSurfaceFormatKHR, out.formatCount, MemoryTag::Renderer);
  }
  VK_CHECK(
      vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &out.formatCount, out.formats));

  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &out.presentModeCount, nullptr));
  ASSERT(out.presentModeCount > 0);
  if (!out.presentModes) {
    out.presentModes = MEMORY_ALLOCATE(VkPresentModeKHR, out.presentModeCount, MemoryTag::Renderer);
  }
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &out.presentModeCount, out.presentModes));
}

void select_physical_device(Context *context) {
  u32 physicalDeviceCount = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, nullptr));
  ASSERT(physicalDeviceCount > 0);

  VkPhysicalDevice physicalDevices[physicalDeviceCount];
  VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices));

  auto physicalDevice = physicalDevices[0]; // Just simply pick the first physical device

  // Retrieve 4 queue families for graphics, presentation, compute and transfer
  u32 graphicsQueueFamily = UINT32_MAX;
  u32 presentQueueFamily  = UINT32_MAX;
  u32 computeQueueFamily  = UINT32_MAX;
  u32 transferQueueFamily = UINT32_MAX;

  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  ASSERT(queueFamilyCount > 0);
  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
  for (u32 i = 0; i < queueFamilyCount; ++i) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && graphicsQueueFamily == UINT32_MAX) {
      graphicsQueueFamily = i;
      continue;
    }
    VkBool32 supportPresent = VK_FALSE;
    VK_CHECK(
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, context->surface, &supportPresent));
    if (supportPresent == VK_TRUE && presentQueueFamily == UINT32_MAX) {
      presentQueueFamily = i;
      continue;
    }
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && computeQueueFamily == UINT32_MAX) {
      computeQueueFamily = i;
      continue;
    }
    if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && transferQueueFamily == UINT32_MAX) {
      transferQueueFamily = i;
      continue;
    }
    if (graphicsQueueFamily != UINT32_MAX && presentQueueFamily != UINT32_MAX &&
        computeQueueFamily != UINT32_MAX && transferQueueFamily != UINT32_MAX) {
      break; // All queue families are fulfilled
    }
  }
  ASSERT(graphicsQueueFamily != UINT32_MAX && presentQueueFamily != UINT32_MAX &&
         computeQueueFamily != UINT32_MAX && transferQueueFamily != UINT32_MAX);

  context->device.physicalDevice = physicalDevice;

  vkGetPhysicalDeviceProperties(physicalDevice, &context->device.properties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &context->device.features);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &context->device.memory);

  context->device.graphicsQueueFamily = graphicsQueueFamily;
  context->device.presentQueueFamily  = presentQueueFamily;
  context->device.computeQueueFamily  = computeQueueFamily;
  context->device.transferQueueFamily = transferQueueFamily;

  device_query_swapchain_support(
      physicalDevice, context->surface, context->device.swapchainSupport);
}
