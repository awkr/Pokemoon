//
// Created by Hongjian Zhu on 2022/10/10.
//

#ifndef POKEMOON_FRONTEND_H
#define POKEMOON_FRONTEND_H

#include "defines.h"

struct RenderPacket {
  f32 deltaTime;
};

bool renderer_system_initialize(
    u64 *memorySize, void *pState, CString appName, u32 width, u32 height);
void renderer_system_shutdown();

bool renderer_draw_frame(const RenderPacket &packet);
void rendererOnResize(u16 width, u16 height);

#endif // POKEMOON_FRONTEND_H
