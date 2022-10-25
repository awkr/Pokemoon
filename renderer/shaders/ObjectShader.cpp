//
// Created by Hongjian Zhu on 2022/10/22.
//

#include "ObjectShader.h"
#include "StringUtils.h"
#include "logging.h"
#include "math/types.h"
#include "memory.h"
#include "platform/filesystem.h"
#include "renderer/Pipeline.h"
#include <array>

bool create_shader_module(Context *context, CString name, CString type, ShaderStage *out);

bool object_shader_create(Context *context, ObjectShader *out) {
  // Shader module init per stage
  const char types[OBJECT_SHADER_STATE_COUNT][5] = {"vert", "frag"};
  for (u8 i = 0; i < OBJECT_SHADER_STATE_COUNT; ++i) {
    if (!create_shader_module(context, "Builtin.ObjectShader", types[i], &out->stages[i])) {
      return false;
    }
  }

  // Todo Descriptors

  // Pipeline creation
  VkViewport viewport{};
  viewport.x        = 0;
  viewport.y        = (f32) context->framebufferHeight;
  viewport.width    = (f32) context->framebufferWidth;
  viewport.height   = -(f32) context->framebufferHeight;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor{};
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width                = context->framebufferWidth;
  scissor.extent.height               = context->framebufferHeight;

  // Attributes
  const u32                         attributeCount = 1;
  VkVertexInputAttributeDescription attributeDescriptions[attributeCount];

  // Position
  VkFormat formats[attributeCount] = {VK_FORMAT_R32G32B32_SFLOAT};
  u32      offsets[attributeCount] = {offsetof(Vertex3D, position)};
  for (u32 i = 0; i < attributeCount; ++i) {
    attributeDescriptions[i].binding  = 0; // Binding index - should match binding desc
    attributeDescriptions[i].location = i; // Attribute location
    attributeDescriptions[i].format   = formats[i];
    attributeDescriptions[i].offset   = offsets[i];
  }

  // Todo Descriptor set layouts

  // Stages
  std::array<VkPipelineShaderStageCreateInfo, OBJECT_SHADER_STATE_COUNT> stageCreateInfos{};
  const VkShaderStageFlagBits stages[OBJECT_SHADER_STATE_COUNT] = {
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_SHADER_STAGE_FRAGMENT_BIT,
  };
  for (u32 i = 0; i < OBJECT_SHADER_STATE_COUNT; ++i) {
    stageCreateInfos[i].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfos[i].stage  = stages[i];
    stageCreateInfos[i].module = out->stages[i].module;
    stageCreateInfos[i].pName  = "main";
  }

  if (!graphics_pipeline_create(context,
                                &context->mainRenderPass,
                                attributeCount,
                                attributeDescriptions,
                                0,
                                nullptr,
                                OBJECT_SHADER_STATE_COUNT,
                                stageCreateInfos.data(),
                                viewport,
                                scissor,
                                false,
                                &out->pipeline)) {
    LOG_ERROR("Failed to create graphics pipeline for object shader");
    return false;
  }

  return true;
}

void object_shader_destroy(Context *context, ObjectShader *shader) {
  pipeline_destroy(context, &shader->pipeline);

  for (auto &stage : shader->stages) {
    vkDestroyShaderModule(context->device.handle, stage.module, context->allocator);
  }
}

void object_shader_use(Context *context, ObjectShader *shader) {}

bool create_shader_module(Context *context, CString name, CString type, ShaderStage *out) {
  // Build file path
  char filepath[512];
  utils::string_format(filepath, "assets/shaders/%s.%s.spv", name, type);

  FileHandle handle{};
  if (!filesystem_open(filepath, FILE_MODE_READ, true, &handle)) { return false; }

  // Read the entire file as binary
  u8 *buffer = nullptr;
  u64 size   = 0;
  if (!filesystem_read_all(&handle, &buffer, &size)) { return false; }

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  shaderModuleCreateInfo.codeSize                 = size;
  shaderModuleCreateInfo.pCode                    = (u32 *) buffer;

  VK_CHECK(vkCreateShaderModule(
      context->device.handle, &shaderModuleCreateInfo, context->allocator, &out->module));

  memory_free(buffer, sizeof(u8) * size, MemoryTag::STRING);

  filesystem_close(&handle);

  return true;
}
