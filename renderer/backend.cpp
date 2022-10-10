//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "backend.h"
#include "memory.h"
#include "types.h"

// ------- Vulkan implementations -------

static Context context{};

bool initialize(RendererBackend *backend,
                PlatformState   *platformState,
                const char      *applicationName);
bool shutdown(RendererBackend *backend);
bool begin_frame(RendererBackend *backend, f32 deltaTime);
bool end_frame(RendererBackend *backend, f32 deltaTime);
void resize(RendererBackend *backend, u16 width, u16 height);

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
                const char      *applicationName) {
  VkApplicationInfo applicationInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  applicationInfo.apiVersion         = VK_API_VERSION_1_3;
  applicationInfo.pApplicationName   = applicationName;
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName        = "Pokemoon";
  applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instanceCreateInfo    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instanceCreateInfo.pApplicationInfo        = &applicationInfo;
  instanceCreateInfo.enabledExtensionCount   = 0;
  instanceCreateInfo.ppEnabledExtensionNames = nullptr;
  instanceCreateInfo.enabledLayerCount       = 0;
  instanceCreateInfo.ppEnabledLayerNames     = nullptr;

  VK_CHECK(vkCreateInstance(&instanceCreateInfo, context.allocator, &context.instance));

  return true;
}

bool shutdown(RendererBackend *backend) {
  vkDestroyInstance(context.instance, context.allocator);
  return true;
}

bool begin_frame(RendererBackend *backend, f32 deltaTime) { return true; }

bool end_frame(RendererBackend *backend, f32 deltaTime) { return true; }

void resize(RendererBackend *backend, u16 width, u16 height) {}
