//
// Created by Hongjian Zhu on 2022/10/22.
//

#pragma once

#include "renderer/types.h"

bool graphics_pipeline_create(Context                           *context,
                              RenderPass                        *renderPass,
                              u32                                attributeCount,
                              VkVertexInputAttributeDescription *attributes,
                              u32                                descriptorSetLayoutCount,
                              VkDescriptorSetLayout             *descriptorSetLayouts,
                              u32                                stageCount,
                              VkPipelineShaderStageCreateInfo   *stages,
                              VkViewport                         viewport,
                              VkRect2D                           scissor,
                              bool                               isWireframe,
                              Pipeline                          *out);

void pipeline_destroy(Context *context, Pipeline *pipeline);

void pipeline_bind(CommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint, Pipeline *pipeline);
