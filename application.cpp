//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "application.h"
#include "core/Clock.h"
#include "event.h"
#include "input.h"
#include "logging.h"
#include "memory.h"
#include "memory/LinearAllocator.h"
#include "platform.h"
#include "renderer/frontend.h"

struct ApplicationState {
  bool  isRunning   = false;
  bool  isSuspended = false;
  u16   width;
  u16   height;
  Clock clock;
  f64   lastTime;

  LinearAllocator *systemsAllocator;

  u64   loggingSystemMemorySize;
  void *loggingSystemState;
  u64   eventSystemMemorySize;
  void *eventSystemState;
  u64   inputSystemMemorySize;
  void *inputSystemState;
  u64   platformSystemMemorySize;
  void *platformSystemState;
  u64   rendererSystemMemorySize;
  void *rendererSystemState;
};

static ApplicationState *state = nullptr;

bool initialize();
bool update(f32 deltaTime);
bool render();

// Event handles
bool application_on_event(EventCode           code,
                          void               *sender,
                          void               *listener,
                          const EventContext &context);
bool application_on_key(EventCode code, void *sender, void *listener, const EventContext &context);
bool applicationOnResize(EventCode code, void *sender, void *listener, const EventContext &context);

void application_create(const ApplicationConfig &config) {
  ASSERT(!state);

  memory_system_initialize();

  state                   = new ApplicationState();
  state->systemsAllocator = new LinearAllocator(64 * MiB);

  // Initialize subsystems

  logging_system_initialize(&state->loggingSystemMemorySize, nullptr);
  state->loggingSystemState = state->systemsAllocator->alloc(state->loggingSystemMemorySize);
  logging_system_initialize(&state->loggingSystemMemorySize, state->loggingSystemState);

  event_system_initialize(&state->eventSystemMemorySize, nullptr);
  state->eventSystemState = state->systemsAllocator->alloc(state->eventSystemMemorySize);
  event_system_initialize(&state->eventSystemMemorySize, state->eventSystemState);

  event_register(EventCode::ApplicationQuit, nullptr, application_on_event);
  event_register(EventCode::KeyReleased, nullptr, application_on_key);
  event_register(EventCode::KeyPressed, nullptr, application_on_key);
  event_register(EventCode::WindowResized, nullptr, applicationOnResize);

  input_system_initialize(&state->inputSystemMemorySize, nullptr);
  state->inputSystemState = state->systemsAllocator->alloc(state->inputSystemMemorySize);
  input_system_initialize(&state->inputSystemMemorySize, state->inputSystemState);

  platform_system_startup(
      &state->platformSystemMemorySize, nullptr, config.name, config.width, config.height);
  state->platformSystemState = state->systemsAllocator->alloc(state->platformSystemMemorySize);
  platform_system_startup(&state->platformSystemMemorySize,
                          state->platformSystemState,
                          config.name,
                          config.width,
                          config.height);

  u32 width = 0, height = 0;
  platform_get_framebuffer_size(width, height);

  renderer_system_initialize(&state->rendererSystemMemorySize, nullptr, config.name, width, height);
  state->rendererSystemState = state->systemsAllocator->alloc(state->rendererSystemMemorySize);
  renderer_system_initialize(
      &state->rendererSystemMemorySize, state->rendererSystemState, config.name, width, height);

  ASSERT(initialize());

  LOG_DEBUG("Hello, Pokemoon.");
}

void application_run() {
  ASSERT(state);

  state->isRunning = true;

  state->clock.start();
  state->clock.tick();
  state->lastTime              = state->clock.elapsed();
  const f64 targetFrameSeconds = 1.0f / 60;

  while (state->isRunning) {
    platform_poll_events();
    if (!state->isSuspended) {
      state->clock.tick();
      f64 currentTime    = state->clock.elapsed();
      f64 deltaTime      = currentTime - state->lastTime;
      state->lastTime    = currentTime;
      f64 frameStartTime = platform_get_absolute_time();

      if (!update((f32) deltaTime)) {
        state->isRunning = false;
        break;
      }
      if (!render()) {
        state->isRunning = false;
        break;
      }

      RenderPacket renderPacket = {};
      renderPacket.deltaTime    = (f32) deltaTime;
      renderer_draw_frame(renderPacket);

      f64 frameEndTime     = platform_get_absolute_time();
      f64 frameElapsedTime = frameEndTime - frameStartTime;
      if (f64 remaining = targetFrameSeconds - frameElapsedTime; remaining > 0) {
        platform_sleep((u64) (remaining * 1000)); // If there is time left, give it back to the OS
      }

      // Input update/state copying should always be handled after any input should be recorded
      input_update();
    }
  }

  state->isRunning = false;

  renderer_system_shutdown();
  platform_system_shutdown();
  input_system_shutdown();
  event_deregister(EventCode::WindowResized, nullptr, applicationOnResize);
  event_deregister(EventCode::KeyPressed, nullptr, application_on_key);
  event_deregister(EventCode::KeyReleased, nullptr, application_on_key);
  event_deregister(EventCode::ApplicationQuit, nullptr, application_on_event);
  event_system_shutdown();
  logging_system_shutdown();

  delete state->systemsAllocator;
  delete state;

  memory_system_shutdown();

  LOG_INFO(memory_get_usage());
}

bool initialize() { return true; }

bool update(f32 deltaTime) { return true; }

bool render() { return true; }

void resize() {}

bool application_on_event(EventCode           code,
                          void               *sender,
                          void               *listener,
                          const EventContext &context) {
  switch (code) {
  case EventCode::ApplicationQuit:
    state->isRunning = false;
    return true;
  default:
    break;
  }
  return false;
}

bool application_on_key(EventCode code, void *sender, void *listener, const EventContext &context) {
  if (code == EventCode::KeyPressed) {
    auto key = context.u16[0];
    LOG_DEBUG("Key %d pressed", key);
  } else if ((EventCode) code == EventCode::KeyReleased) {
    auto key = (Key) context.u16[0];
    if (key == Key::ESC) {
      event_fire(EventCode::ApplicationQuit, nullptr, {});
      return true;
    } else if (key == Key::M) {
      LOG_INFO(memory_get_usage());
      LOG_INFO("Alloc count: %d", memory_get_alloc_count());
      return true;
    }
    LOG_DEBUG("Key %d released", key);
  }
  return false;
}

bool applicationOnResize(EventCode           code,
                         void               *sender,
                         void               *listener,
                         const EventContext &context) {
  auto width  = context.u32[0];
  auto height = context.u32[1];

  state->width  = width;
  state->height = height;

  if (width == 0 || height == 0) { // Handle minimization
    state->isSuspended = true;
    return true;
  }

  if (state->isSuspended) { state->isSuspended = false; } // Resume

  // LOG_DEBUG("Window resized: %i, %i", width, height);

  rendererOnResize(width, height);

  return false; // Event purposely not handled to allow other listeners to get this
}
