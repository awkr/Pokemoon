//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "platform.h"
#include "logging.h"
#include "memory.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

struct PlatformInternalState {
  GLFWwindow *window;
};

static void glfw_error_callback(int error, const char *description) {
  LOG_ERROR("GLFW error: %s", description);
}

static void glfw_close_callback(GLFWwindow *window) {}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height) {}

static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {}

static void glfw_cursor_position_callback(GLFWwindow *window, f64 xPos, f64 yPos) {}

static void glfw_scroll_callback(GLFWwindow *window, f64 xOffset, f64 yOffset) {}

static void glfw_refresh_callback(GLFWwindow *window) {}

void platform_startup(PlatformState *state, const char *name, u32 width, u32 height) {
  state->internalState = memory_allocate(sizeof(PlatformInternalState), MemoryTag::Platform);
  auto internalState   = (PlatformInternalState *) state->internalState;

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

  internalState->window = window;
}

void platform_shutdown(PlatformState *state) {
  auto internalState = (PlatformInternalState *) state->internalState;
  glfwDestroyWindow(internalState->window);
  glfwTerminate();
  memory_free(internalState, sizeof(PlatformInternalState), MemoryTag::Platform);
}

void platform_poll_events(PlatformState *state) { glfwPollEvents(); }

void *platform_allocate(u64 size, bool aligned) { return malloc(size); }

void platform_free(void *block, bool aligned) { free(block); }

void *platform_zero_memory(void *block, u64 size) { return platform_set_memory(block, 0, size); }

void *platform_copy_memory(void *dst, const void *src, u64 size) { return memcpy(dst, src, size); }

void *platform_set_memory(void *dst, i32 value, u64 size) { return memset(dst, value, size); }

void platform_console_write(const char *message) { fprintf(stdout, "%s", message); }

void platform_console_write_error(const char *message) { fprintf(stderr, "%s", message); }

f64 platform_get_absolute_time() { return glfwGetTime(); }

void platform_sleep(u64 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
