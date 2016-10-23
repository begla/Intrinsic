// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
void ComputeCallManager::createResources(
    const ComputeCallRefArray& p_ComputeCalls)
{
  for (uint32_t ccIdx = 0u; ccIdx < p_ComputeCalls.size(); ++ccIdx)
  {
    ComputeCallRef computeCallRef = p_ComputeCalls[ccIdx];

    _INTR_ARRAY(BindingInfo)& bindInfs = _descBindInfos(computeCallRef);
    PipelineRef pipelineRef = _descPipeline(computeCallRef);
    PipelineLayoutRef pipelineLayout =
        Resources::PipelineManager::_descPipelineLayout(pipelineRef);

    VkDescriptorSet& descSet = _vkDescriptorSet(computeCallRef);
    _INTR_ASSERT(descSet == VK_NULL_HANDLE);

    // Allocate and init. descriptor set
    descSet = Resources::PipelineLayoutManager::allocateAndWriteDescriptorSet(
        pipelineLayout, bindInfs);

    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(computeCallRef);

    uint32_t dynamicOffsetCount = 0u;
    for (uint32_t bindIdx = 0u; bindIdx < (uint32_t)bindInfos.size(); ++bindIdx)
    {
      BindingInfo& bindInfo = bindInfos[bindIdx];

      if (bindInfo.bindingType == BindingType::kUniformBufferDynamic)
      {
        ++dynamicOffsetCount;
      }
    }

    _dynamicOffsets(computeCallRef).resize(dynamicOffsetCount);
  }
}

// <-

void ComputeCallManager::updateUniformMemory(
    const ComputeCallRefArray& p_ComputeCalls, void* p_PerInstanceDataCompute,
    uint32_t p_PerInstanceDataComputeSize)
{
  for (uint32_t ccIdx = 0u; ccIdx < p_ComputeCalls.size(); ++ccIdx)
  {
    Resources::ComputeCallRef computeCallRef = p_ComputeCalls[ccIdx];
    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(computeCallRef);

    uint32_t dynamicOffsetIndex = 0u;
    for (uint32_t bIdx = 0u; bIdx < bindInfos.size(); ++bIdx)
    {
      BindingInfo& bindInfo = bindInfos[bIdx];

      if (bindInfo.bindingType == BindingType::kUniformBufferDynamic)
      {
        if (bindInfo.bufferData.uboType == UboType::kPerInstanceCompute)
        {
          UniformManager::allocatePerInstanceDataMemory(
              bindInfo.bufferData.rangeInBytes,
              _dynamicOffsets(computeCallRef)[dynamicOffsetIndex]);
          const uint32_t dynamicOffset =
              _dynamicOffsets(computeCallRef)[dynamicOffsetIndex];

          uint8_t* gpuMem =
              &UniformManager::_mappedPerInstanceMemory[dynamicOffset];
          memcpy(gpuMem, p_PerInstanceDataCompute,
                 p_PerInstanceDataComputeSize);
        }

        ++dynamicOffsetIndex;
      }
    }
  }
}

// <-

void ComputeCallManager::bindImage(ComputeCallRef p_DrawCallRef,
                                   const Name& p_Name, uint8_t p_ShaderStage,
                                   Dod::Ref p_ImageRef, uint8_t p_SamplerIdx,
                                   uint8_t p_BindingFlags,
                                   uint8_t p_ArrayLayerIdx,
                                   uint8_t p_MipLevelIdx)
{
  PipelineLayoutRef pipelineLayout =
      PipelineManager::_descPipelineLayout(_descPipeline(p_DrawCallRef));

  _INTR_ARRAY(BindingDescription)& bindingDescs =
      PipelineLayoutManager::_descBindingDescs(pipelineLayout);
  _INTR_ARRAY(BindingInfo)& bindingInfos = _descBindInfos(p_DrawCallRef);

  bool found = false;
  for (uint32_t i = 0u; i < bindingDescs.size(); ++i)
  {
    BindingDescription& desc = bindingDescs[i];

    if (desc.name == p_Name && desc.shaderStage == p_ShaderStage)
    {
      if (bindingInfos.size() <= desc.binding)
      {
        bindingInfos.resize(desc.binding + 1u);
      }

      BindingInfo bindingInfo;
      bindingInfo.binding = desc.binding;
      bindingInfo.bindingType = desc.bindingType;
      bindingInfo.resource = p_ImageRef;
      bindingInfo.imageData.arrayLayerIdx = p_ArrayLayerIdx;
      bindingInfo.imageData.mipLevelIdx = p_MipLevelIdx;
      bindingInfo.imageData.samplerIdx = p_SamplerIdx;
      bindingInfo.bindingFlags = p_BindingFlags;

      bindingInfos[desc.binding] = bindingInfo;

      found = true;
      break;
    }
  }

  _INTR_ASSERT(found);
}

// <-

void ComputeCallManager::bindBuffer(ComputeCallRef p_DrawCallRef,
                                    const Name& p_Name, uint8_t p_ShaderStage,
                                    Dod::Ref p_BufferRef, uint8_t p_UboType,
                                    uint32_t p_RangeInBytes,
                                    uint32_t p_OffsetInBytes)
{
  PipelineLayoutRef pipelineLayout =
      PipelineManager::_descPipelineLayout(_descPipeline(p_DrawCallRef));

  _INTR_ARRAY(BindingDescription)& bindingDescs =
      PipelineLayoutManager::_descBindingDescs(pipelineLayout);
  _INTR_ARRAY(BindingInfo)& bindingInfos = _descBindInfos(p_DrawCallRef);

  bool found = false;
  for (uint32_t i = 0u; i < bindingDescs.size(); ++i)
  {
    BindingDescription& desc = bindingDescs[i];

    if (desc.name == p_Name && desc.shaderStage == p_ShaderStage)
    {
      if (bindingInfos.size() <= desc.binding)
      {
        bindingInfos.resize(desc.binding + 1u);
      }

      BindingInfo bindingInfo;
      bindingInfo.binding = desc.binding;
      bindingInfo.bindingType = desc.bindingType;
      bindingInfo.resource = p_BufferRef;
      bindingInfo.bufferData.offsetInBytes = p_OffsetInBytes;
      bindingInfo.bufferData.rangeInBytes = p_RangeInBytes;
      bindingInfo.bufferData.uboType = p_UboType;

      bindingInfos[desc.binding] = bindingInfo;

      found = true;
      break;
    }
  }

  _INTR_ASSERT(found);
}
}
}
}
}
