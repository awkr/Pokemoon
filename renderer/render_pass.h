//
// Created by Hongjian Zhu on 2022/10/13.
//

#ifndef POKEMOON_RENDER_PASS_H
#define POKEMOON_RENDER_PASS_H

#include "renderer/types.h"

void render_pass_create(Context        *context,
                        const VkRect2D &rendArea,
                        const Vec4     &clearColor,
                        f32             depth,
                        u32             stencil,
                        RenderPass     *outRenderPass);

void render_pass_destroy(Context *context, RenderPass *renderPass);

void render_pass_begin(CommandBuffer *commandBuffer,
                       RenderPass    *renderPass,
                       VkFramebuffer  framebuffer);

void render_pass_end(CommandBuffer *commandBuffer, RenderPass *renderPass);

#endif // POKEMOON_RENDER_PASS_H
