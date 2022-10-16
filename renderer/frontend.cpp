//
// Created by Hongjian Zhu on 2022/10/10.
//

#include "frontend.h"
#include "backend.h"
#include "logging.h"
#include "memory.h"

static RendererBackend *backend = nullptr;

bool begin_frame(f32 deltaTime);
bool end_frame(f32 deltaTime);

bool renderer_initialize(struct PlatformState *platformState,
                         CString               applicationName,
                         u32                   width,
                         u32                   height) {
  backend = (RendererBackend *) memory_allocate(sizeof(RendererBackend), MemoryTag::Renderer);
  renderer_backend_setup(backend);
  return backend->initialize(backend, platformState, applicationName, width, height);
}

void renderer_shutdown() {
  backend->shutdown(backend);
  renderer_backend_cleanup(backend);
  memory_free(backend, sizeof(RendererBackend), MemoryTag::Renderer);
}

bool renderer_draw_frame(const RenderPacket &packet) {
  if (begin_frame(packet.deltaTime)) {
    if (!end_frame(packet.deltaTime)) { return false; }
  }
  return true;
}

void rendererOnResize(u16 width, u16 height) {
  if (backend) {
    backend->resize(backend, width, height);
  } else {
    LOG_WARN("Renderer backend does not exist to accept size: %i %i", width, height);
  }
}

bool begin_frame(f32 deltaTime) { return backend->beginFrame(backend, deltaTime); }

bool end_frame(f32 deltaTime) { return backend->endFrame(backend, deltaTime); }
