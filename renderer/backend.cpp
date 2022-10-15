//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "renderer/backend.h"
#include "container/darray.h"
#include "logging.h"
#include "memory.h"
#include "platform.h"
#include "renderer/command_buffer.h"
#include "renderer/device.h"
#include "renderer/fence.h"
#include "renderer/framebuffer.h"
#include "renderer/render_pass.h"
#include "renderer/swapchain.h"
#include "renderer/types.h"

// ------- Vulkan implementations -------

static Context context{};

u32 cachedFramebufferWidth  = 0;
u32 cachedFramebufferHeight = 0;

bool initialize(RendererBackend *backend,
                PlatformState   *platformState,
                const char      *applicationName,
                u32              width,
                u32              height);
bool shutdown(RendererBackend *backend);
bool begin_frame(RendererBackend *backend, f32 deltaTime);
bool end_frame(RendererBackend *backend, f32 deltaTime);
void resize(RendererBackend *backend, u16 width, u16 height);
bool query_memory_type_index(u32 requiredType, VkMemoryPropertyFlags requiredProperty, u32 &index);
void create_framebuffers(RendererBackend *backend, Swapchain *swapchain, RenderPass *renderPass);
void create_command_buffers(RendererBackend *backend);

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

bool initialize(RendererBackend *backend,
                PlatformState   *platformState,
                const char      *applicationName,
                u32              width,
                u32              height) {
  context.query_memory_type_index = query_memory_type_index;

  cachedFramebufferWidth    = width;
  cachedFramebufferHeight   = height;
  context.framebufferWidth  = cachedFramebufferWidth;
  context.framebufferHeight = cachedFramebufferHeight;
  cachedFramebufferWidth = cachedFramebufferHeight = 0;

  VkApplicationInfo applicationInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  applicationInfo.apiVersion         = VK_API_VERSION_1_3;
  applicationInfo.pApplicationName   = applicationName;
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

  platform_create_surface(platformState, &context);

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
      DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight, MemoryTag::Renderer);
  context.drawCompleteSemaphores =
      DARRAY_RESERVE(VkSemaphore, context.swapchain.maxFramesInFlight, MemoryTag::Renderer);
  context.inFlightFences =
      DARRAY_RESERVE(Fence, context.swapchain.maxFramesInFlight, MemoryTag::Renderer);

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

  return true;
}

bool shutdown(RendererBackend *backend) {
  vkDeviceWaitIdle(context.device.handle);

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

bool begin_frame(RendererBackend *backend, f32 deltaTime) { return true; }

bool end_frame(RendererBackend *backend, f32 deltaTime) { return true; }

void resize(RendererBackend *backend, u16 width, u16 height) {
  cachedFramebufferWidth  = width;
  cachedFramebufferHeight = height;
  // Todo Recreate swapchain and related objects
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
  context.graphicsCommandBuffers =
      DARRAY_RESERVE(CommandBuffer, context.swapchain.imageCount, MemoryTag::Renderer);
  for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
    command_buffer_allocate(
        &context, context.device.graphicsCommandPool, true, &context.graphicsCommandBuffers[i]);
  }
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
