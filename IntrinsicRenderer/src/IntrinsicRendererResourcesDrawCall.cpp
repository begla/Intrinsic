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

using namespace CResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Resources
{
// Static members
_INTR_ARRAY(_INTR_ARRAY(DrawCallRef))
DrawCallManager::_drawCallsPerMaterialPass;

// <-

void DrawCallManager::createResources(const DrawCallRefArray& p_DrawCalls)
{
  for (uint32_t dcIdx = 0u; dcIdx < p_DrawCalls.size(); ++dcIdx)
  {
    DrawCallRef drawCallRef = p_DrawCalls[dcIdx];

    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(drawCallRef);
    PipelineRef pipelineRef = _descPipeline(drawCallRef);
    PipelineLayoutRef pipelineLayout =
        Resources::PipelineManager::_descPipelineLayout(pipelineRef);
    _INTR_ARRAY(BufferRef)& descVtxBuffers = _descVertexBuffers(drawCallRef);
    _INTR_ARRAY(VkBuffer)& vtxBuffers = _vertexBuffers(drawCallRef);

    VkDescriptorSet& descSet = _vkDescriptorSet(drawCallRef);
    _INTR_ASSERT(descSet == VK_NULL_HANDLE);

    descSet = Resources::PipelineLayoutManager::allocateAndWriteDescriptorSet(
        pipelineLayout, bindInfos);

    // Defaults for now
    _indexBufferOffset(drawCallRef) = 0ull;
    _vertexBufferOffsets(drawCallRef).resize(descVtxBuffers.size());
    _vertexBuffers(drawCallRef).resize(descVtxBuffers.size());

    for (uint32_t i = 0u; i < descVtxBuffers.size(); ++i)
    {
      vtxBuffers[i] = BufferManager::_vkBuffer(descVtxBuffers[i]);
    }

    uint32_t dynamicOffsetCount = 0u;
    for (uint32_t bindIdx = 0u; bindIdx < (uint32_t)bindInfos.size(); ++bindIdx)
    {
      BindingInfo& bindInfo = bindInfos[bindIdx];

      if (bindInfo.bindingType == BindingType::kUniformBufferDynamic)
      {
        ++dynamicOffsetCount;
      }
    }

    _dynamicOffsets(drawCallRef).resize(dynamicOffsetCount);

    // "Sort" to per material pass array
    uint8_t materialPass = _descMaterialPass(drawCallRef);
    if (materialPass + 1u > _drawCallsPerMaterialPass.size())
    {
      _drawCallsPerMaterialPass.resize(materialPass + 1u);
    }
    _drawCallsPerMaterialPass[materialPass].push_back(drawCallRef);
  }
}

// <-

void DrawCallManager::bindImage(DrawCallRef p_DrawCallRef, const Name& p_Name,
                                uint8_t p_ShaderStage, Dod::Ref p_ImageRef,
                                uint8_t p_SamplerIdx, uint8_t p_BindingFlags,
                                uint8_t p_ArrayLayerIdx, uint8_t p_MipLevelIdx)
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

void DrawCallManager::bindBuffer(DrawCallRef p_DrawCallRef, const Name& p_Name,
                                 uint8_t p_ShaderStage, Dod::Ref p_BufferRef,
                                 uint8_t p_UboType, uint32_t p_RangeInBytes,
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

// <-

DrawCallRef DrawCallManager::createDrawCallForMesh(
    const Name& p_Name, Dod::Ref p_Mesh, Dod::Ref p_Material,
    uint8_t p_MaterialPass, uint32_t p_PerInstanceDataVertexSize,
    uint32_t p_PerInstanceDataFragmentSize, uint32_t p_SubMeshIdx)
{
  if (!p_Mesh.isValid())
  {
    return Dod::Ref();
  }

  DrawCallRef drawCallMesh = createDrawCall(p_Name);
  resetToDefault(drawCallMesh);
  addResourceFlags(drawCallMesh,
                   Dod::Resources::ResourceFlags::kResourceVolatile);

  {
    _descPipeline(drawCallMesh) = MaterialManager::_materialPassPipelines
        [MaterialManager::_materialPasses[p_MaterialPass].pipelineIdx];

    _INTR_ASSERT(PipelineManager::_vkPipeline(_descPipeline(drawCallMesh)));

    _descVertexBuffers(drawCallMesh) =
        MeshManager::_vertexBuffersPerSubMesh(p_Mesh)[p_SubMeshIdx];
    _descIndexBuffer(drawCallMesh) =
        MeshManager::_indexBufferPerSubMesh(p_Mesh)[p_SubMeshIdx];
    _descVertexCount(drawCallMesh) =
        (uint32_t)MeshManager::_descPositionsPerSubMesh(p_Mesh)[p_SubMeshIdx]
            .size();
    _descIndexCount(drawCallMesh) =
        (uint32_t)MeshManager::_descIndicesPerSubMesh(p_Mesh)[p_SubMeshIdx]
            .size();
    _descMaterial(drawCallMesh) = p_Material;
    _descMaterialPass(drawCallMesh) = p_MaterialPass;

    MaterialPass::BoundResources& boundResources =
        MaterialManager::_materialPassBoundResources
            [MaterialManager::_materialPasses[p_MaterialPass].boundResoucesIdx];

    for (uint32_t i = 0u; i < boundResources.boundResourceEntries.size(); ++i)
    {
      MaterialPass::BoundResourceEntry& entry =
          boundResources.boundResourceEntries[i];

      if (entry.type == MaterialPass::BoundResourceType::kImage)
      {
        Name resourceName = entry.resourceName;
        auto functionMapping =
            MaterialManager::_materialResourceFunctionMapping.find(
                entry.resourceName);
        if (functionMapping !=
            MaterialManager::_materialResourceFunctionMapping.end())
        {
          resourceName = functionMapping->second(p_Material);
        }
        ImageRef imageRef = ImageManager::getResourceByName(resourceName);
        DrawCallManager::bindImage(drawCallMesh, entry.slotName,
                                   entry.shaderStage, imageRef,
                                   Samplers::kLinearRepeat);
      }
      else if (entry.type == MaterialPass::BoundResourceType::kBuffer)
      {
        if (entry.resourceName == _N(PerMaterial))
        {
          DrawCallManager::bindBuffer(
              drawCallMesh, entry.slotName, entry.shaderStage,
              UniformManager::_perMaterialUniformBuffer,
              entry.shaderStage == GpuProgramType::kFragment
                  ? UboType::kPerMaterialFragment
                  : UboType::kPerMaterialVertex,
              entry.shaderStage == GpuProgramType::kFragment
                  ? sizeof(PerMaterialDataFragment)
                  : sizeof(PerMaterialDataVertex));
        }
        else if (entry.resourceName == _N(PerInstance))
        {
          DrawCallManager::bindBuffer(
              drawCallMesh, entry.slotName, entry.shaderStage,
              UniformManager::_perInstanceUniformBuffer,
              entry.shaderStage == GpuProgramType::kFragment
                  ? UboType::kPerInstanceFragment
                  : UboType::kPerInstanceVertex,
              entry.shaderStage == GpuProgramType::kFragment
                  ? p_PerInstanceDataFragmentSize
                  : p_PerInstanceDataVertexSize);
        }
        else if (entry.resourceName == _N(PerFrame))
        {
          DrawCallManager::bindBuffer(
              drawCallMesh, entry.slotName, entry.shaderStage,
              UniformManager::_perFrameUniformBuffer,
              entry.shaderStage == GpuProgramType::kFragment
                  ? UboType::kPerFrameFragment
                  : UboType::kPerFrameVertex,
              entry.shaderStage == GpuProgramType::kFragment
                  ? sizeof(RenderProcess::PerFrameDataFrament)
                  : sizeof(RenderProcess::PerFrameDataVertex));
        }
        else
        {
          _INTR_ASSERT(false && "Resource type not found");
        }
      }
    }

    return drawCallMesh;
  }
}
}
}
}
