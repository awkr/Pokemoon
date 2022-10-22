//
// Created by Hongjian Zhu on 2022/10/22.
//

#include "ObjectShader.h"
#include "StringUtils.h"
#include "memory.h"
#include "platform/filesystem.h"

bool create_shader_module(
    Context *context, CString name, CString type, VkShaderStageFlagBits stage, ShaderStage *out);

bool object_shader_create(Context *context, ObjectShader *out) {
  // Shader module init per stage
  const char types[OBJECT_SHADER_STATE_COUNT][5] = {
      "vert",
      "frag",
  };
  const VkShaderStageFlagBits stages[OBJECT_SHADER_STATE_COUNT] = {
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  for (u8 i = 0; i < OBJECT_SHADER_STATE_COUNT; ++i) {
    if (!create_shader_module(
            context, "Builtin.ObjectShader", types[i], stages[i], &out->stages[i])) {
      return false;
    }
  }

  return true;
}

void object_shader_destroy(Context *context, ObjectShader *shader) {
  for (u8 i = 0; i < OBJECT_SHADER_STATE_COUNT; ++i) {
    vkDestroyShaderModule(context->device.handle, shader->stages[i].module, context->allocator);
  }
}

void object_shader_use(Context *context, ObjectShader *shader) {}

bool create_shader_module(
    Context *context, CString name, CString type, VkShaderStageFlagBits stage, ShaderStage *out) {
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

  // Shader stage info

  VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  shaderStageCreateInfo.stage  = stage;
  shaderStageCreateInfo.module = out->module;
  shaderStageCreateInfo.pName  = "main";

  out->shaderStageCreateInfo = shaderStageCreateInfo;

  return true;
}
