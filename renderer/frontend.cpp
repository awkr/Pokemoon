//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "frontend.h"
#include "backend.h"
#include "logging.h"
#include "memory.h"

struct RenderSystemState {
  RendererBackend backend{};
};

static RenderSystemState *state = nullptr;

bool begin_frame(f32 deltaTime);
bool end_frame(f32 deltaTime);

bool renderer_system_initialize(
    u64 *memorySize, void *pState, CString appName, u32 width, u32 height) {
  *memorySize = sizeof(RenderSystemState);
  if (!pState) { return true; }
  state = (RenderSystemState *) pState;
  renderer_backend_setup(&state->backend);
  return state->backend.initialize(&state->backend, appName, width, height);
}

void renderer_system_shutdown() {
  state->backend.shutdown(&state->backend);
  renderer_backend_cleanup(&state->backend);
}

bool renderer_draw_frame(const RenderPacket &packet) {
  if (begin_frame(packet.deltaTime)) {
    if (!end_frame(packet.deltaTime)) { return false; }
  }
  return true;
}

void rendererOnResize(u16 width, u16 height) {
  if (state) {
    state->backend.resize(&state->backend, width, height);
  } else {
    LOG_WARN("Renderer backend does not exist to accept size: %i %i", width, height);
  }
}

bool begin_frame(f32 deltaTime) { return state->backend.beginFrame(&state->backend, deltaTime); }

bool end_frame(f32 deltaTime) { return state->backend.endFrame(&state->backend, deltaTime); }
