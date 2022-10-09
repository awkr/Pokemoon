//
// Created by Hongjian Zhu on 2022/10/8.
//

#include "application.h"
#include "event.h"
#include "input.h"
#include "logging.h"
#include "memory.h"
#include "platform.h"

struct ApplicationState {
  bool          isRunning;
  bool          isSuspended;
  PlatformState platformState;
  u16           width;
  u16           height;
};

static bool             initialized = false;
static ApplicationState applicationState{};

bool initialize();
bool update();
bool render();
void resize();

// Event handles
bool application_on_event(EventCode           code,
                          void               *sender,
                          void               *listener,
                          const EventContext &context);
bool application_on_key(EventCode code, void *sender, void *listener, const EventContext &context);

void application_create(const ApplicationConfig &config) {
  ASSERT(!initialized);

  // Initialize subsystems
  logging_initialize();
  event_initialize();
  event_register(EventCode::ApplicationQuit, nullptr, application_on_event);
  event_register(EventCode::KeyReleased, nullptr, application_on_key);
  event_register(EventCode::KeyPressed, nullptr, application_on_key);
  input_initialize();
  platform_startup(&applicationState.platformState, config.name, config.width, config.height);

  ASSERT(initialize());

  applicationState.isRunning = true;

  LOG_DEBUG("Hello, Pokemoon.");

  initialized = true;
}

void application_run() {
  LOG_INFO(memory_get_usage());

  while (applicationState.isRunning) {
    platform_poll_events(&applicationState.platformState);
    if (!applicationState.isSuspended) {
      if (!update()) {
        applicationState.isRunning = false;
        break;
      }
      if (!render()) {
        applicationState.isRunning = false;
        break;
      }
      // Input update/state copying should always be handled after any input should be recorded
      input_update();
    }
  }

  applicationState.isRunning = false;

  platform_shutdown(&applicationState.platformState);
  input_shutdown();
  event_deregister(EventCode::KeyPressed, nullptr, application_on_key);
  event_deregister(EventCode::KeyReleased, nullptr, application_on_key);
  event_deregister(EventCode::ApplicationQuit, nullptr, application_on_event);
  event_shutdown();
  logging_shutdown();

  LOG_INFO(memory_get_usage());
}

bool initialize() { return true; }

bool update() { return true; }

bool render() { return true; }

void resize() {}

bool application_on_event(EventCode           code,
                          void               *sender,
                          void               *listener,
                          const EventContext &context) {
  switch (code) {
  case EventCode::ApplicationQuit:
    applicationState.isRunning = false;
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
      return true;
    }
    LOG_DEBUG("Key %d released", key);
  }
  return false;
}
