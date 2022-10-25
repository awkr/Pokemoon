//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_TYPES_H
#define POKEMOON_TYPES_H

#include "defines.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#define VK_CHECK(expr) ASSERT(expr == VK_SUCCESS);

using Vec4 = glm::vec4;

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

  VkCommandPool graphicsCommandPool;

  VkFormat depthFormat;
};

struct Image {
  VkImage        handle;
  VkDeviceMemory memory;
  VkImageView    view;
  u32            width;
  u32            height;
};

enum class RenderPassState {
  NotAllocated,
  Ready,
  Recording,
  InRenderPass,
  RecordingEnded,
  Submitted,
};

struct RenderPass {
  VkRenderPass    handle;
  VkRect2D        renderArea;
  Vec4            clearColor;
  f32             depth;
  u32             stencil;
  RenderPassState state;
};

struct Framebuffer {
  VkFramebuffer handle;
  u32           attachmentCount;
  VkImageView  *attachments;
  RenderPass   *renderPass;
};

struct Swapchain {
  VkSurfaceFormatKHR imageFormat;
  u8                 maxFramesInFlight;
  VkSwapchainKHR     handle;
  u32                imageCount;
  VkImage           *images;
  VkImageView       *views;
  Image              depthAttachment;
  Framebuffer       *framebuffers; // Framebuffers used for on-screen rendering
};

enum class CommandBufferState { // Used for dedicated state tracking
  NotAllocated,
  Ready,
  Recording,
  InRenderPass,
  RecordingEnded,
  Submitted,
};

struct CommandBuffer {
  VkCommandBuffer    handle;
  CommandBufferState state;
};

struct Fence {
  VkFence handle;
  bool    isSignaled;
};

struct ShaderStage {
  VkShaderModule module;
};

struct Pipeline {
  VkPipeline       handle;
  VkPipelineLayout layout;
};

#define OBJECT_SHADER_STATE_COUNT 2 // Vertex, fragment

struct ObjectShader {
  ShaderStage stages[OBJECT_SHADER_STATE_COUNT];
  Pipeline    pipeline;
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
  // Current generation of framebuffer size. If it does not match `framebufferSizeLastGeneration`, a
  // new one should be generated
  u64 framebufferSizeGeneration;
  // The generation of the framebuffer when it was last created. Set to `framebufferSizeGeneration`
  // when updated
  u64 framebufferSizeLastGeneration;

  Swapchain swapchain;
  u32       imageIndex;
  u64       currentFrame; // [0, Swapchain::maxFramesInFlight - 1]
  bool      recreatingSwapchain;

  ObjectShader objectShader;

  RenderPass mainRenderPass;

  CommandBuffer *graphicsCommandBuffers; // DArray

  // [0, Swapchain::maxFramesInFlight - 1]
  VkSemaphore *imageAcquiredSemaphores; // When an image is done presenting, it can be acquired to
                                        // rendered to
  VkSemaphore *drawCompleteSemaphores;  // Means image is ready to be presented
  u32          inFlightFenceCount;
  Fence       *inFlightFences;

  Fence **imagesInFlight; // [0, Swapchain::imageCount - 1]

  bool (*query_memory_type_index)(u32                   requiredType,
                                  VkMemoryPropertyFlags requiredProperty,
                                  u32                  &index);
};

#endif // POKEMOON_TYPES_H
