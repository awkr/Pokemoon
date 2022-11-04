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
#include "renderer/buffer.h"
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

  // Descriptors

  { // Global descriptors
    VkDescriptorSetLayoutBinding binding{};
    binding.binding         = 0;
    binding.descriptorCount = 1;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layout = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout.bindingCount                    = 1;
    layout.pBindings                       = &binding;
    VK_CHECK(vkCreateDescriptorSetLayout(
        context->device.handle, &layout, context->allocator, &out->globalDescriptorSetLayout));

    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = context->swapchain.imageCount;

    VkDescriptorPoolCreateInfo pool = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool.poolSizeCount              = 1;
    pool.pPoolSizes                 = &poolSize;
    pool.maxSets                    = context->swapchain.imageCount;
    VK_CHECK(vkCreateDescriptorPool(
        context->device.handle, &pool, context->allocator, &out->globalDescriptorPool));
  }

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
  const u32                         attributeCount = 2;
  VkVertexInputAttributeDescription attributeDescriptions[attributeCount];

  // Position and color
  VkFormat formats[attributeCount] = {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT};
  u32      offsets[attributeCount] = {offsetof(Vertex3D, position), offsetof(Vertex3D, color)};
  for (u32 i = 0; i < attributeCount; ++i) {
    attributeDescriptions[i].binding  = 0; // Binding index - should match binding desc
    attributeDescriptions[i].location = i; // Attribute location
    attributeDescriptions[i].format   = formats[i];
    attributeDescriptions[i].offset   = offsets[i];
  }

  // Descriptor set layouts
  const u32             descriptorSetLayoutCount          = 1;
  VkDescriptorSetLayout layouts[descriptorSetLayoutCount] = {
      out->globalDescriptorSetLayout,
  };

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
                                descriptorSetLayoutCount,
                                layouts,
                                OBJECT_SHADER_STATE_COUNT,
                                stageCreateInfos.data(),
                                viewport,
                                scissor,
                                false,
                                &out->pipeline)) {
    LOG_ERROR("Failed to create graphics pipeline for object shader");
    return false;
  }

  // Create uniform buffer
  if (!buffer_create(context,
                     sizeof(GlobalUniformObject),
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     true,
                     &out->globalUniformBuffer)) {
    return false;
  }

  // Allocate global descriptor sets

  VkDescriptorSetLayout globalLayouts[3 /* Todo */] = {
      out->globalDescriptorSetLayout,
      out->globalDescriptorSetLayout,
      out->globalDescriptorSetLayout,
  };

  VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  allocateInfo.descriptorPool              = out->globalDescriptorPool;
  allocateInfo.descriptorSetCount          = 3; /* Todo */
  allocateInfo.pSetLayouts                 = globalLayouts;
  VK_CHECK(
      vkAllocateDescriptorSets(context->device.handle, &allocateInfo, out->globalDescriptorSets));

  return true;
}

void object_shader_destroy(Context *context, ObjectShader *shader) {
  auto device = context->device.handle;

  buffer_destroy(context, &shader->globalUniformBuffer);

  pipeline_destroy(context, &shader->pipeline);

  vkDestroyDescriptorPool(device, shader->globalDescriptorPool, context->allocator);
  vkDestroyDescriptorSetLayout(device, shader->globalDescriptorSetLayout, context->allocator);

  for (auto &stage : shader->stages) {
    vkDestroyShaderModule(device, stage.module, context->allocator);
  }
}

void object_shader_use(Context *context, ObjectShader *shader) {
  pipeline_bind(&context->graphicsCommandBuffers[context->imageIndex],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                &shader->pipeline);
}

void object_shader_update_global_state(Context *context, ObjectShader *shader) {
  auto imageIndex    = context->imageIndex;
  auto commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;
  auto descriptorSet = shader->globalDescriptorSets[imageIndex];

  // Bind the descriptor set to be updated
  vkCmdBindDescriptorSets(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          shader->pipeline.layout,
                          0,
                          1,
                          &descriptorSet,
                          0,
                          nullptr);

  // Configure the descriptors for the given index
  u64 offset = 0;
  u32 range  = sizeof(GlobalUniformObject);

  // Copy data to buffer
  buffer_load(context, &shader->globalUniformBuffer, offset, range, 0, &shader->globalUBO);

  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = shader->globalUniformBuffer.handle;
  bufferInfo.offset = offset;
  bufferInfo.range  = range;

  // Update descriptor sets
  VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptorWrite.dstSet               = shader->globalDescriptorSets[imageIndex];
  descriptorWrite.dstBinding           = 0;
  descriptorWrite.dstArrayElement      = 0;
  descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount      = 1;
  descriptorWrite.pBufferInfo          = &bufferInfo;

  vkUpdateDescriptorSets(context->device.handle, 1, &descriptorWrite, 0, nullptr);
}

void object_shader_update_object(Context *context, ObjectShader *shader, const glm::mat4 &model) {
  auto commandBuffer = context->graphicsCommandBuffers[context->imageIndex].handle;
  vkCmdPushConstants(commandBuffer,
                     shader->pipeline.layout,
                     VK_SHADER_STAGE_VERTEX_BIT,
                     0,
                     sizeof(glm::mat4),
                     &model);
}

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
