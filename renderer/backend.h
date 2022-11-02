//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_BACKEND_H
#define POKEMOON_BACKEND_H

#include "defines.h"
#include "glm/glm.hpp"

struct Context;

struct RendererBackend {
  bool (*initialize)(RendererBackend *backend, const char *appName, u32 width, u32 height);
  bool (*shutdown)(RendererBackend *backend);
  void (*resize)(RendererBackend *backend, u16 width, u16 height);
  bool (*beginFrame)(RendererBackend *backend, f32 deltaTime);
  void (*updateGlobalState)(const glm::mat4 &proj,
                            const glm::mat4 &view,
                            const glm::vec3 &eye,
                            const glm::vec4 &ambient,
                            u32              mode);
  bool (*endFrame)(RendererBackend *backend, f32 deltaTime);
};

void renderer_backend_setup(RendererBackend *backend);
void renderer_backend_cleanup(RendererBackend *backend);

void platform_get_required_extension(CString *&extensions);
void platform_create_surface(Context *context);

#endif // POKEMOON_BACKEND_H
