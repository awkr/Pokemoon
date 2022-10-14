//
// Created by Hongjian Zhu on 2022/10/14.
//

#ifndef POKEMOON_COMMAND_BUFFER_H
#define POKEMOON_COMMAND_BUFFER_H

#include "renderer/types.h"

void command_buffer_allocate(Context       *context,
                             VkCommandPool  commandPool,
                             bool           isPrimary,
                             CommandBuffer *outCommandBuffer);

void command_buffer_free(Context *context, VkCommandPool commandPool, CommandBuffer *commandBuffer);

void command_buffer_begin(CommandBuffer *commandBuffer,
                          bool           isSingleUse,
                          bool           isRenderPassContinue,
                          bool           isSimultaneousUse);

void command_buffer_end(CommandBuffer *commandBuffer);

void command_buffer_update_submitted(CommandBuffer *commandBuffer);

void command_buffer_reset(CommandBuffer *commandBuffer);

// Allocate and begin recording to the command buffer
void command_buffer_allocate_and_begin_single_use(Context       *context,
                                                  VkCommandPool  commandPool,
                                                  CommandBuffer *outCommandBuffer);

// End recording, submit to and wait for queue operation and free the provided command buffer
void command_buffer_end_single_use(Context       *context,
                                   VkCommandPool  commandPool,
                                   CommandBuffer *commandBuffer,
                                   VkQueue        queue);

#endif // POKEMOON_COMMAND_BUFFER_H
