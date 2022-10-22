//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "platform.h"
#include "container/darray.h"
#include "event.h"
#include "input.h"
#include "logging.h"
#include "memory.h"
#include "renderer/types.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>

struct PlatformSystemState {
  GLFWwindow *window = nullptr;
};

static PlatformSystemState *state = nullptr;

static void glfw_error_callback(int error, const char *description) {
  LOG_ERROR("GLFW error: %s", description);
}

static void glfw_close_callback(GLFWwindow *window) {
  event_fire(EventCode::ApplicationQuit, nullptr, {});
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  bool pressed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
  input_process_key((Key) key, pressed);
}

static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  EventContext context{};
  context.u32[0] = width;
  context.u32[1] = height;
  event_fire(EventCode::WindowResized, nullptr, context);
}

static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  bool pressed = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
  input_process_mouse_button((MouseButton) button, pressed);
}

static void glfw_cursor_position_callback(GLFWwindow *window, f64 xPos, f64 yPos) {
  input_process_mouse_move((f32) xPos, (f32) yPos);
}

static void glfw_scroll_callback(GLFWwindow *window, f64 xOffset, f64 yOffset) {
  input_process_mouse_wheel((f32) xOffset, (f32) yOffset);
}

static void glfw_refresh_callback(GLFWwindow *window) {}

void platform_system_startup(u64 *memorySize, void *pState, CString name, u32 width, u32 height) {
  *memorySize = sizeof(PlatformSystemState);
  if (!pState) { return; }
  state = (PlatformSystemState *) pState;

  glfwSetErrorCallback(glfw_error_callback);
  ASSERT(glfwInit() == GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto window = glfwCreateWindow((int) width, (int) height, name, nullptr, nullptr);
  ASSERT(window);
  glfwSetWindowCloseCallback(window, glfw_close_callback);
  glfwSetKeyCallback(window, glfw_key_callback);
  glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
  glfwSetScrollCallback(window, glfw_scroll_callback);
  glfwSetWindowRefreshCallback(window, glfw_refresh_callback);

  state->window = window;
}

void platform_system_shutdown() {
  glfwDestroyWindow(state->window);
  glfwTerminate();
  glfwSetErrorCallback(nullptr);
  state = nullptr;
}

void platform_poll_events() { glfwPollEvents(); }

void *platform_allocate(u64 size, bool aligned) { return malloc(size); }

void platform_free(void *block, bool aligned) { free(block); }

void *platform_zero_memory(void *block, u64 size) { return platform_set_memory(block, 0, size); }

void *platform_copy_memory(void *dst, const void *src, u64 size) { return memcpy(dst, src, size); }

void *platform_set_memory(void *dst, i32 value, u64 size) { return memset(dst, value, size); }

void platform_console_write(CString message) { fprintf(stdout, "%s", message); }

void platform_console_write_error(CString message) { fprintf(stderr, "%s", message); }

f64 platform_get_absolute_time() { return glfwGetTime(); }

void platform_sleep(u64 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void platform_get_framebuffer_size(u32 &width, u32 &height) {
  glfwGetFramebufferSize(state->window, (int *) (&width), (int *) (&height));
}

void platform_get_required_extension(CString *&extensions) {
  DARRAY_PUSH(extensions, &VK_EXT_METAL_SURFACE_EXTENSION_NAME);
}

void platform_create_surface(Context *context) {
  auto error = glfwCreateWindowSurface(
      context->instance, state->window, context->allocator, &context->surface);
  VK_CHECK(error);
}
