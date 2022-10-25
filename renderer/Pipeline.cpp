//
// Created by Hongjian Zhu on 2022/10/22.
//

#include "Pipeline.h"
#include "logging.h"
#include "math/types.h"
#include <vector>

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
                              Pipeline                          *out) {
  VkPipelineViewportStateCreateInfo viewportState = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizationState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizationState.depthClampEnable        = VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.polygonMode     = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterizationState.lineWidth       = 1.0f;
  rasterizationState.cullMode        = VK_CULL_MODE_BACK_BIT;
  rasterizationState.frontFace       = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthBiasEnable = VK_FALSE;
  rasterizationState.depthBiasConstantFactor = 0.0f;
  rasterizationState.depthBiasClamp          = 0.0f;
  rasterizationState.depthBiasSlopeFactor    = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampleState = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampleState.sampleShadingEnable   = VK_FALSE;
  multisampleState.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
  multisampleState.minSampleShading      = 1.0f;
  multisampleState.pSampleMask           = nullptr;
  multisampleState.alphaToCoverageEnable = VK_FALSE;
  multisampleState.alphaToOneEnable      = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depthStencilState = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depthStencilState.depthTestEnable       = VK_TRUE;
  depthStencilState.depthWriteEnable      = VK_TRUE;
  depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS;
  depthStencilState.depthBoundsTestEnable = VK_FALSE;
  depthStencilState.stencilTestEnable     = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
  colorBlendAttachmentState.blendEnable         = VK_TRUE;
  colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendState = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlendState.logicOpEnable   = VK_FALSE;
  colorBlendState.logicOp         = VK_LOGIC_OP_COPY;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments    = &colorBlendAttachmentState;

  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_LINE_WIDTH,
  };

  VkPipelineDynamicStateCreateInfo dynamicState = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.dynamicStateCount = dynamicStates.size();
  dynamicState.pDynamicStates    = dynamicStates.data();

  // Vertex input
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0; // Binding index
  bindingDescription.stride  = sizeof(Vertex3D);
  bindingDescription.inputRate =
      VK_VERTEX_INPUT_RATE_VERTEX; // Move to next data entry for each vertex

  // Attributes
  VkPipelineVertexInputStateCreateInfo vertexInputState = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputState.vertexBindingDescriptionCount   = 1;
  vertexInputState.pVertexBindingDescriptions      = &bindingDescription;
  vertexInputState.vertexAttributeDescriptionCount = attributeCount;
  vertexInputState.pVertexAttributeDescriptions    = attributes;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssemblyState.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;

  // Pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  // Descriptor set layouts
  pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
  pipelineLayoutCreateInfo.pSetLayouts    = descriptorSetLayouts;

  // Create the pipeline layout
  VK_CHECK(vkCreatePipelineLayout(
      context->device.handle, &pipelineLayoutCreateInfo, context->allocator, &out->layout));

  // Create pipeline
  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineCreateInfo.stageCount          = stageCount;
  pipelineCreateInfo.pStages             = stages;
  pipelineCreateInfo.pVertexInputState   = &vertexInputState;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pViewportState      = &viewportState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pMultisampleState   = &multisampleState;
  pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
  pipelineCreateInfo.pColorBlendState    = &colorBlendState;
  pipelineCreateInfo.pDynamicState       = &dynamicState;
  pipelineCreateInfo.layout              = out->layout;
  pipelineCreateInfo.renderPass          = renderPass->handle;

  VK_CHECK(vkCreateGraphicsPipelines(context->device.handle,
                                     VK_NULL_HANDLE,
                                     1,
                                     &pipelineCreateInfo,
                                     context->allocator,
                                     &out->handle));

  LOG_DEBUG("Graphics pipeline created");

  return true;
}

void pipeline_destroy(Context *context, Pipeline *pipeline) {
  vkDestroyPipeline(context->device.handle, pipeline->handle, context->allocator);
  vkDestroyPipelineLayout(context->device.handle, pipeline->layout, context->allocator);
}

void pipeline_bind(CommandBuffer      *commandBuffer,
                   VkPipelineBindPoint bindPoint,
                   Pipeline           *pipeline) {
  vkCmdBindPipeline(commandBuffer->handle, bindPoint, pipeline->handle);
}
