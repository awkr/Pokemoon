//
// Created by Hongjian Zhu on 2022/10/15.
//

#ifndef POKEMOON_FRAMEBUFFER_H
#define POKEMOON_FRAMEBUFFER_H

#include "renderer/types.h"

void framebuffer_create(Context     *context,
                        RenderPass  *renderPass,
                        u32          width,
                        u32          height,
                        u32          attachmentCount,
                        VkImageView *attachments,
                        Framebuffer *outFramebuffer);

void framebuffer_destroy(Context *context, Framebuffer *framebuffer);

#endif // POKEMOON_FRAMEBUFFER_H
