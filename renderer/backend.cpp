//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "renderer/backend.h"
#include "container/darray.h"
#include "logging.h"
#include "memory.h"
#include "renderer/command_buffer.h"
#include "renderer/device.h"
#include "renderer/fence.h"
#include "renderer/framebuffer.h"
#include "renderer/render_pass.h"
#include "renderer/shaders/ObjectShader.h"
#include "renderer/swapchain.h"
#include "renderer/types.h"

// ------- Vulkan implementations -------

static Context context{};

u32 cachedFramebufferWidth  = 0;
u32 cachedFramebufferHeight = 0;

bool initialize(RendererBackend *backend, const char *appName, u32 width, u32 height);
bool shutdown(RendererBackend *backend);
bool begin_frame(RendererBackend *backend, f32 deltaTime);
bool end_frame(RendererBackend *backend, f32 deltaTime);
void resize(RendererBackend *backend, u16 width, u16 height);
bool query_memory_type_index(u32 requiredType, VkMemoryPropertyFlags requiredProperty, u32 &index);
void create_framebuffers(RendererBackend *backend, Swapchain *swapchain, RenderPass *renderPass);
void create_command_buffers(RendererBackend *backend);
bool recreateSwapchain(RendererBackend *backend);

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
               void                                       *pUserData);

// ------- Public APIs -------

void renderer_backend_setup(RendererBackend *outBackend) {
  outBackend->initialize = initialize;
  outBackend->shutdown   = shutdown;
  outBackend->beginFrame = begin_frame;
  outBackend->endFrame   = end_frame;
  outBackend->resize     = resize;
}

void renderer_backend_cleanup(RendererBackend *backend) {
  memory_zero(backend, sizeof(RendererBackend));
}

// ------- Vulkan implementations -------

bool initialize(RendererBackend *backend, const char *appName, u32 width, u32 height) {
  context.query_memory_type_index = query_memory_type_index;

  cachedFramebufferWidth    = width;
  cachedFramebufferHeight   = height;
  context.framebufferWidth  = cachedFramebufferWidth;
  context.framebufferHeight = cachedFramebufferHeight;
  cachedFramebufferWidth    = 0;
  cachedFramebufferHeight   = 0;

  VkApplicationInfo applicationInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  applicationInfo.apiVersion         = VK_API_VERSION_1_3;
  applicationInfo.pApplicationName   = appName;
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName        = "Pokemoon";
  applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

  auto requiredExtensions = DARRAY_CREATE(CString);
  DARRAY_PUSH(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME); // Generic surface extension
  DARRAY_PUSH(requiredExtensions, &VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  platform_get_required_extension(requiredExtensions); // Platform-specific extension(s)
#ifdef DEBUG
  DARRAY_PUSH(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Debug utilities

  LOG_DEBUG("Required instance extensions:");
  for (u32 i = 0; i < darray_length(requiredExtensions); ++i) {
    LOG_DEBUG("  %s", requiredExtensions[i]);
  }
#endif

  CString *requiredLayers = nullptr;
#ifdef DEBUG
  requiredLayers = DARRAY_CREATE(CString);
  DARRAY_PUSH(requiredLayers, &"VK_LAYER_KHRONOS_validation");
#endif

  VkInstanceCreateInfo instanceCreateInfo    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceCreateInfo.pApplicationInfo        = &applicationInfo;
  instanceCreateInfo.enabledExtensionCount   = darray_length(requiredExtensions);
  instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions;
  instanceCreateInfo.enabledLayerCount       = requiredLayers ? darray_length(requiredLayers) : 0;
  instanceCreateInfo.ppEnabledLayerNames     = requiredLayers;

#ifdef DEBUG
  VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
  debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugMessengerCreateInfo.pfnUserCallback = debug_callback;

  instanceCreateInfo.pNext = &debugMessengerCreateInfo;
#endif

  // Create instance
  VK_CHECK(vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance));

  darray_destroy(requiredExtensions);
#ifdef DEBUG
  darray_destroy(requiredLayers);
#endif

#ifdef DEBUG
  // Create debugger
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
      context.instance, "vkCreateDebugUtilsMessengerEXT");
  VK_CHECK(func(
      context.instance, &debugMessengerCreateInfo, context.allocator, &context.debugMessenger));
#endif

  platform_create_surface(&context);

  device_create(&context);

  swapchain_create(
      &context, context.framebufferWidth, context.framebufferHeight, &context.swapchain);

  render_pass_create(&context,
                     {{0, 0}, {context.framebufferWidth, context.framebufferHeight}},
                     {0.5, 0.1, 0.2, 1},
                     1,
                     0,
                     &context.mainRenderPass);

  context.swapchain.framebuffers =
      DARRAY_RESERVE(Framebuffer, context.swapchain.imageCount, MemoryTag::Renderer);
  create_framebuffers(backend, &context.swapchain, &context.mainRenderPass);

  create_command_buffers(backend);

  // Create sync objects
  context.imageAcquiredSemaphores =
      DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight, MemoryTag::Semaphore);
  context.drawCompleteSemaphores =
      DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight, MemoryTag::Semaphore);
  context.inFlightFences =
      DARRAY_RESERVE(Fence, context.swapchain.maxFramesInFlight, MemoryTag::Fence);

  for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
    VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(
        context.device.handle, &createInfo, context.allocator, &context.imageAcquiredSemaphores[i]);
    vkCreateSemaphore(
        context.device.handle, &createInfo, context.allocator, &context.drawCompleteSemaphores[i]);

    fence_create(&context, true, &context.inFlightFences[i]); // Create the fence in signaled state
  }

  context.imagesInFlight =
      DARRAY_RESERVE(Fence *, context.swapchain.imageCount, MemoryTag::Renderer);

  // Create builtin shaders
  ASSERT(object_shader_create(&context, &context.objectShader));

  return true;
}

bool shutdown(RendererBackend *backend) {
  vkDeviceWaitIdle(context.device.handle);

  object_shader_destroy(&context, &context.objectShader);

  darray_destroy(context.imagesInFlight);
  for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
    fence_destroy(&context, &context.inFlightFences[i]);
    vkDestroySemaphore(context.device.handle, context.drawCompleteSemaphores[i], context.allocator);
    vkDestroySemaphore(
        context.device.handle, context.imageAcquiredSemaphores[i], context.allocator);
  }
  darray_destroy(context.inFlightFences);
  darray_destroy(context.drawCompleteSemaphores);
  darray_destroy(context.imageAcquiredSemaphores);

  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    command_buffer_free(
        &context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
  }
  darray_destroy(context.graphicsCommandBuffers);

  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }
  darray_destroy(context.swapchain.framebuffers);

  render_pass_destroy(&context, &context.mainRenderPass);
  swapchain_destroy(&context, &context.swapchain);
  device_destroy(&context);
  vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
#ifdef DEBUG
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
      context.instance, "vkDestroyDebugUtilsMessengerEXT");
  func(context.instance, context.debugMessenger, context.allocator);
#endif
  vkDestroyInstance(context.instance, context.allocator);
  return true;
}

bool begin_frame(RendererBackend *backend, f32 deltaTime) {
  auto device = context.device;
  if (context.recreatingSwapchain) {
    VK_CHECK(vkDeviceWaitIdle(device.handle));
    return false;
  }
  if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration) {
    VK_CHECK(vkDeviceWaitIdle(device.handle));
    if (!recreateSwapchain(backend)) { return false; }
    return false;
  }

  // Wait for the execution of the current frame to complete. The fence being free will allow us to
  // move on
  assert(fence_wait(&context, &context.inFlightFences[context.currentFrame]));

  // Acquire the next image from the swapchain. Pass along the semaphore that should be signaled
  // when completes. The same semaphore will later be waited on by the queue submission to ensure
  // the image is available
  assert(swapchain_acquire_next_image(&context,
                                      &context.swapchain,
                                      UINT64_MAX,
                                      context.imageAcquiredSemaphores[context.currentFrame],
                                      nullptr,
                                      &context.imageIndex));

  // Begin recording commands
  auto commandBuffer = &(context.graphicsCommandBuffers[context.imageIndex]);

  command_buffer_reset(commandBuffer);

  command_buffer_begin(commandBuffer, false, false, false);

  // Dynamic states
  VkViewport viewport{};
  viewport.x        = 0;
  viewport.y        = (f32) context.framebufferHeight;
  viewport.width    = (f32) context.framebufferWidth;
  viewport.height   = -(f32) context.framebufferHeight;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width                = context.framebufferWidth;
  scissor.extent.height               = context.framebufferHeight;

  vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

  context.mainRenderPass.renderArea = {{0, 0},
                                       {context.framebufferWidth, context.framebufferHeight}};

  render_pass_begin(commandBuffer,
                    &context.mainRenderPass,
                    context.swapchain.framebuffers[context.imageIndex].handle);

  return true;
}

bool end_frame(RendererBackend *backend, f32 deltaTime) {
  auto commandBuffer = &(context.graphicsCommandBuffers[context.imageIndex]);

  render_pass_end(commandBuffer, &context.mainRenderPass);

  command_buffer_end(commandBuffer);

  // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
  if (context.imagesInFlight[context.imageIndex]) {
    fence_wait(&context, context.imagesInFlight[context.imageIndex]);
  }

  // Mark the image fence as in-use by this frame
  context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

  fence_reset(&context, &context.inFlightFences[context.currentFrame]);

  // Submit the queue and wait for the operation to complete
  VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

  // Command buffer(s) to be executed
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer->handle;

  // The semaphore(s) to be signaled when the queue is complete
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores    = &context.drawCompleteSemaphores[context.currentFrame];

  // Wait semaphore ensures that the operation cannot begin until the image is available
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores    = &context.imageAcquiredSemaphores[context.currentFrame];

  // Each semaphore waits on a corresponding pipeline stage to complete.
  // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent color attachment writes from
  // executing until the semaphore signals (i.e. one frame is presented at a time)
  VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask  = flags;

  VK_CHECK(vkQueueSubmit(context.device.graphicsQueue,
                         1,
                         &submitInfo,
                         context.inFlightFences[context.currentFrame].handle));

  command_buffer_update_submitted(commandBuffer);

  // Give the image back to the swapchain
  swapchain_present(&context,
                    &context.swapchain,
                    context.device.graphicsQueue,
                    context.device.presentQueue,
                    context.drawCompleteSemaphores[context.currentFrame],
                    context.imageIndex);

  return true;
}

void resize(RendererBackend *backend, u16 width, u16 height) {
  cachedFramebufferWidth  = width;
  cachedFramebufferHeight = height;
  context.framebufferSizeGeneration++;
}

bool query_memory_type_index(u32 requiredType, VkMemoryPropertyFlags requiredProperty, u32 &index) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);
  for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    if ((requiredType & (1 << i)) &&
        ((memoryProperties.memoryTypes[i].propertyFlags & requiredProperty) == requiredProperty)) {
      index = i;
      return true;
    }
  }
  return false;
}

void create_framebuffers(RendererBackend *backend, Swapchain *swapchain, RenderPass *renderPass) {
  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    u32         attachmentCount = 2;
    VkImageView attachments[]   = {
        swapchain->views[i],
        swapchain->depthAttachment.view,
    };
    framebuffer_create(&context,
                       renderPass,
                       context.framebufferWidth,
                       context.framebufferHeight,
                       attachmentCount,
                       attachments,
                       &context.swapchain.framebuffers[i]);
  }
}

void create_command_buffers(RendererBackend *backend) {
  if (!context.graphicsCommandBuffers) {
    context.graphicsCommandBuffers =
        DARRAY_RESERVE(CommandBuffer, context.swapchain.imageCount, MemoryTag::CommandBuffer);
  }
  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    command_buffer_allocate(
        &context, context.device.graphicsCommandPool, true, &context.graphicsCommandBuffers[i]);
  }
}

bool recreateSwapchain(RendererBackend *backend) {
  if (context.recreatingSwapchain) { return false; }
  if (cachedFramebufferWidth == 0 || cachedFramebufferHeight == 0) { return false; }
  vkDeviceWaitIdle(context.device.handle);
  context.recreatingSwapchain = true; // Set flag
  // Cleanup
  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    context.imagesInFlight[i] = nullptr;
    command_buffer_free(
        &context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }
  // Recreate
  swapchain_recreate(&context, cachedFramebufferWidth, cachedFramebufferHeight, &context.swapchain);
  // Sync the framebuffer size with the cached size
  context.framebufferWidth              = cachedFramebufferWidth;
  context.framebufferHeight             = cachedFramebufferHeight;
  cachedFramebufferWidth                = 0;
  cachedFramebufferHeight               = 0;
  context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;
  context.mainRenderPass.renderArea     = {{0, 0},
                                       {context.framebufferWidth, context.framebufferHeight}};
  create_framebuffers(backend, &context.swapchain, &context.mainRenderPass);
  create_command_buffers(backend);
  context.recreatingSwapchain = false; // Clear flag
  return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
               void                                       *pUserData) {
  switch (messageSeverity) {
  default:
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    LOG_ERROR(pCallbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    LOG_WARN(pCallbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    LOG_INFO(pCallbackData->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    LOG_DEBUG(pCallbackData->pMessage);
    break;
  }
  return VK_FALSE;
}
