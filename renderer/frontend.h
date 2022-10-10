//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_FRONTEND_H
#define POKEMOON_FRONTEND_H

#include "defines.h"

struct RenderPacket {
  f32 deltaTime;
};

bool renderer_initialize(struct PlatformState *platformState, const char *applicationName);
void renderer_shutdown();

bool renderer_draw_frame(const RenderPacket &packet);
void renderer_resize(u16 width, u16 height);

#endif // POKEMOON_FRONTEND_H
