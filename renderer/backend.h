//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_BACKEND_H
#define POKEMOON_BACKEND_H

#include "defines.h"

struct PlatformState;

struct RendererBackend {
  bool (*initialize)(RendererBackend *backend,
                     PlatformState   *platformState,
                     const char      *applicationName);
  bool (*shutdown)(RendererBackend *backend);
  void (*resize)(RendererBackend *backend, u16 width, u16 height);
  bool (*beginFrame)(RendererBackend *backend, f32 deltaTime);
  bool (*endFrame)(RendererBackend *backend, f32 deltaTime);
};

void renderer_backend_setup(RendererBackend *outBackend);
void renderer_backend_cleanup(RendererBackend *backend);

void platform_get_required_extension(CString *&extensions);

#endif // POKEMOON_BACKEND_H
