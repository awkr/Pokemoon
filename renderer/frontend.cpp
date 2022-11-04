//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "frontend.h"
#include "backend.h"
#include "logging.h"
#include "math/types.h"
#include "memory.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

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
    const auto proj = glm::perspective(glm::radians(45.0f), 480.0f / 480.0f, 0.1f, 100.0f);

    static f32 z = -1.0f;
    z -= 0.005f;
    const auto view = glm::translate(glm::mat4(1.0f), {0, 0, z});

    state->backend.updateGlobalState(proj, view, VEC3_ZERO, VEC4_ONE, 0);

    static float angle = 0.0f;
    angle += 0.01f;
    auto quat  = glm::angleAxis(angle, glm::vec3(0, 0, 1));
    auto model = glm::mat4_cast(quat);
    state->backend.updateObject(model);

    if (!end_frame(packet.deltaTime)) { return false; }

    return true;
  }
  return false;
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
