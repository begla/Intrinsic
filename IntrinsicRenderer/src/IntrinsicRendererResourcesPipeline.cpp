// Copyright 2017 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Resources
{
void createGraphicsPipeline(PipelineRef p_PipelineRef)
{
  _INTR_ARRAY(uint8_t)& blendStates =
      PipelineManager::_descBlendStates(p_PipelineRef);
  VkPipeline& pipeline = PipelineManager::_vkPipeline(p_PipelineRef);

  _INTR_ARRAY(VkPipelineColorBlendAttachmentState) blendAttachmentStates;
  for (uint32_t i = 0u; i < (uint32_t)blendStates.size(); ++i)
  {
    blendAttachmentStates.push_back(RenderStates::blendStates[blendStates[i]]);
  }

  VkPipelineColorBlendStateCreateInfo cb = {};
  {
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.flags = 0;
    cb.pNext = nullptr;
    cb.attachmentCount = (uint32_t)blendAttachmentStates.size();
    cb.pAttachments = blendAttachmentStates.data();
    cb.logicOpEnable = VK_FALSE;
    cb.logicOp = VK_LOGIC_OP_NO_OP;
    cb.blendConstants[0] = 1.0f;
    cb.blendConstants[1] = 1.0f;
    cb.blendConstants[2] = 1.0f;
    cb.blendConstants[3] = 1.0f;
  }

  glm::uvec2& dimScissor =
      PipelineManager::_descAbsoluteScissorDimensions(p_PipelineRef);
  if (PipelineManager::_descScissorRenderSize(p_PipelineRef) !=
      RenderSize::kCustom)
  {
    dimScissor = RenderSystem::getAbsoluteRenderSize(
        (RenderSize::Enum)PipelineManager::_descScissorRenderSize(
            p_PipelineRef));
  }

  glm::uvec2& dimViewport =
      PipelineManager::_descAbsoluteViewportDimensions(p_PipelineRef);
  if (PipelineManager::_descViewportRenderSize(p_PipelineRef) !=
      RenderSize::kCustom)
  {
    dimViewport = RenderSystem::getAbsoluteRenderSize(
        (RenderSize::Enum)PipelineManager::_descScissorRenderSize(
            p_PipelineRef));
  }

  VkViewport viewport = {};
  {
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)dimViewport.x;
    viewport.height = (float)dimViewport.y;
  }

  VkRect2D scissor = {};
  {
    scissor.extent.width = dimScissor.x;
    scissor.extent.height = dimScissor.y;
    scissor.offset.x = 0u;
    scissor.offset.y = 0u;
  }

  VkPipelineViewportStateCreateInfo viewportStateCreateinfo = {};
  {
    viewportStateCreateinfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateinfo.pNext = nullptr;
    viewportStateCreateinfo.flags = 0u;
    viewportStateCreateinfo.pScissors = &scissor;
    viewportStateCreateinfo.pViewports = &viewport;
    viewportStateCreateinfo.viewportCount = 1u;
    viewportStateCreateinfo.scissorCount = 1u;
  }

  VkPipelineMultisampleStateCreateInfo ms = {};
  {
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pNext = nullptr;
    ms.flags = 0;
    ms.pSampleMask = nullptr;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms.sampleShadingEnable = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable = VK_FALSE;
    ms.minSampleShading = 0.0;
  }

  VertexLayoutRef vtxLayout = PipelineManager::_descVertexLayout(p_PipelineRef);
  PipelineLayoutRef pipLayout =
      PipelineManager::_descPipelineLayout(p_PipelineRef);
  RenderPassRef rp = PipelineManager::_descRenderPass(p_PipelineRef);
  GpuProgramRef vp = PipelineManager::_descVertexProgram(p_PipelineRef);
  GpuProgramRef fp = PipelineManager::_descFragmentProgram(p_PipelineRef);
  GpuProgramRef gp = PipelineManager::_descGeometryProgram(p_PipelineRef);

  uint8_t dss = PipelineManager::_descDepthStencilState(p_PipelineRef);
  uint8_t ias = PipelineManager::_descInputAssemblyState(p_PipelineRef);
  uint8_t rs = PipelineManager::_descRasterizationState(p_PipelineRef);

  uint32_t shaderStageCount = 0u;
  VkPipelineShaderStageCreateInfo shaderStages[3];

  if (vp.isValid())
  {
    shaderStages[shaderStageCount++] =
        GpuProgramManager::_vkPipelineShaderStageCreateInfo(vp);
  }
  if (fp.isValid())
  {
    shaderStages[shaderStageCount++] =
        GpuProgramManager::_vkPipelineShaderStageCreateInfo(fp);
  }
  if (gp.isValid())
  {
    shaderStages[shaderStageCount++] =
        GpuProgramManager::_vkPipelineShaderStageCreateInfo(gp);
  }

  VkPipelineVertexInputStateCreateInfo vtxInputState = {};
  if (vtxLayout.isValid())
  {
    vtxInputState =
        VertexLayoutManager::_vkPipelineVertexInputStateCreateInfo(vtxLayout);
  }
  else
  {
    vtxInputState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vtxInputState.pNext = nullptr;
    vtxInputState.flags = 0u;
    vtxInputState.vertexBindingDescriptionCount = 0u;
    vtxInputState.pVertexBindingDescriptions = nullptr;
    vtxInputState.vertexAttributeDescriptionCount = 0u;
    vtxInputState.pVertexAttributeDescriptions = nullptr;
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  {
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.layout =
        PipelineLayoutManager::_vkPipelineLayout(pipLayout);
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = 0u;
    pipelineCreateInfo.flags = 0u;
    pipelineCreateInfo.pVertexInputState = &vtxInputState;
    pipelineCreateInfo.pInputAssemblyState =
        &RenderStates::inputAssemblyStates[ias];
    pipelineCreateInfo.pRasterizationState =
        &RenderStates::rasterizationStates[rs];
    pipelineCreateInfo.pColorBlendState = &cb;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pMultisampleState = &ms;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportStateCreateinfo;
    pipelineCreateInfo.pDepthStencilState =
        &RenderStates::depthStencilStates[dss];
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.stageCount = shaderStageCount;
    pipelineCreateInfo.renderPass = RenderPassManager::_vkRenderPass(rp);
    pipelineCreateInfo.subpass = 0u;
  }

  VkResult result = vkCreateGraphicsPipelines(
      RenderSystem::_vkDevice, RenderSystem::_vkPipelineCache, 1u,
      &pipelineCreateInfo, nullptr, &pipeline);
  _INTR_VK_CHECK_RESULT(result);
}

// <-

void createComputePipeline(PipelineRef p_PipelineRef)
{
  VkPipeline& pipeline = PipelineManager::_vkPipeline(p_PipelineRef);

  PipelineLayoutRef pipLayout =
      PipelineManager::_descPipelineLayout(p_PipelineRef);
  GpuProgramRef cp = PipelineManager::_descComputeProgram(p_PipelineRef);

  VkComputePipelineCreateInfo pipelineCreateInfo = {};
  {
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.layout =
        PipelineLayoutManager::_vkPipelineLayout(pipLayout);
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = 0u;
    pipelineCreateInfo.flags = 0u;
    pipelineCreateInfo.stage =
        GpuProgramManager::_vkPipelineShaderStageCreateInfo(cp);
  }

  VkResult result = vkCreateComputePipelines(
      RenderSystem::_vkDevice, RenderSystem::_vkPipelineCache, 1u,
      &pipelineCreateInfo, nullptr, &pipeline);
  _INTR_VK_CHECK_RESULT(result);
}

void PipelineManager::createResources(const PipelineRefArray& p_Pipelines)
{
  for (uint32_t i = 0u; i < p_Pipelines.size(); ++i)
  {
    RenderPassRef pipelineRef = p_Pipelines[i];

    if (_descComputeProgram(pipelineRef).isValid())
    {
      createComputePipeline(pipelineRef);
    }
    else
    {
      createGraphicsPipeline(pipelineRef);
    }
  }
}
}
}
}
