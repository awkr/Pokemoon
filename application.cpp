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

void application_create(const ApplicationConfig &config) {
  ASSERT(!initialized);

  // Initialize subsystems
  logging_initialize();
  event_initialize();
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
  event_shutdown();
  logging_shutdown();

  LOG_INFO(memory_get_usage());
}

bool initialize() { return true; }

bool update() { return true; }

bool render() { return true; }

void resize() {}
