// Copyright 2016 Benjamin Glatzel
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
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
_INTR_ARRAY(VkPipelineDepthStencilStateCreateInfo)
RenderStates::depthStencilStates;
_INTR_ARRAY(VkPipelineInputAssemblyStateCreateInfo)
RenderStates::inputAssemblyStates;
_INTR_ARRAY(VkPipelineRasterizationStateCreateInfo)
RenderStates::rasterizationStates;
_INTR_ARRAY(VkPipelineColorBlendAttachmentState) RenderStates::blendStates;

void initDepthStencilStates()
{
  {
    VkPipelineDepthStencilStateCreateInfo& ds =
        RenderStates::depthStencilStates[DepthStencilStates::kDefault];
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = nullptr;
    ds.flags = 0u;
    ds.depthTestEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthWriteEnable = VK_TRUE;
  }

  {
    VkPipelineDepthStencilStateCreateInfo& ds = RenderStates::depthStencilStates
        [DepthStencilStates::kDefaultNoDepthTestAndWrite];
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = nullptr;
    ds.flags = 0u;
  }

  {
    VkPipelineDepthStencilStateCreateInfo& ds =
        RenderStates::depthStencilStates[DepthStencilStates::kDefaultNoWrite];
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = nullptr;
    ds.flags = 0u;
    ds.depthTestEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  }

  {
    VkPipelineDepthStencilStateCreateInfo& ds = RenderStates::depthStencilStates
        [DepthStencilStates::kDefaultNoDepthTest];
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = nullptr;
    ds.flags = 0u;
    ds.depthTestEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    ds.depthWriteEnable = VK_TRUE;
  }
}

void initInputAssemblyStates()
{
  {
    VkPipelineInputAssemblyStateCreateInfo& ia =
        RenderStates::inputAssemblyStates[InputAssemblyStates::kTriangleList];
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.pNext = nullptr;
    ia.flags = 0u;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }

  {
    VkPipelineInputAssemblyStateCreateInfo& ia =
        RenderStates::inputAssemblyStates[InputAssemblyStates::kLineList];
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.pNext = nullptr;
    ia.flags = 0u;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
  }
}

void initRasterizationStates()
{
  {
    VkPipelineRasterizationStateCreateInfo& rs =
        RenderStates::rasterizationStates[RasterizationStates::kDefault];
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext = nullptr;
    rs.flags = 0;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0.0f;
    rs.depthBiasClamp = 0.0f;
    rs.depthBiasSlopeFactor = 0.0f;
    rs.lineWidth = 1.0f;
  }

  {
    VkPipelineRasterizationStateCreateInfo& rs = RenderStates::
        rasterizationStates[RasterizationStates::kInvertedCulling];
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext = nullptr;
    rs.flags = 0;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_FRONT_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0.0f;
    rs.depthBiasClamp = 0.0f;
    rs.depthBiasSlopeFactor = 0.0f;
    rs.lineWidth = 1.0f;
  }

  {
    VkPipelineRasterizationStateCreateInfo& rs =
        RenderStates::rasterizationStates[RasterizationStates::kDoubleSided];
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext = nullptr;
    rs.flags = 0;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0.0f;
    rs.depthBiasClamp = 0.0f;
    rs.depthBiasSlopeFactor = 0.0f;
    rs.lineWidth = 1.0f;
  }
}

void initBlendStates()
{
  {
    VkPipelineColorBlendAttachmentState& bs =
        RenderStates::blendStates[BlendStates::kDefault];
    bs.colorWriteMask = 0xf;
    bs.blendEnable = VK_FALSE;
    bs.alphaBlendOp = VK_BLEND_OP_ADD;
    bs.colorBlendOp = VK_BLEND_OP_ADD;
    bs.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    bs.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  }
}

void RenderStates::init()
{
  {
    depthStencilStates.resize(DepthStencilStates::kCount);
    initDepthStencilStates();

    inputAssemblyStates.resize(InputAssemblyStates::kCount);
    initInputAssemblyStates();

    rasterizationStates.resize(RasterizationStates::kCount);
    initRasterizationStates();

    blendStates.resize(BlendStates::kCount);
    initBlendStates();
  }
}
}
}
}
