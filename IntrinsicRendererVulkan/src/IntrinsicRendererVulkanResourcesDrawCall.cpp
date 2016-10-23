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
namespace
{
_INTR_INLINE void initBindInfosGBuffer(DrawCallRef p_DrawCallRef,
                                       MaterialRef p_Material,
                                       uint32_t p_PerInstanceDataVertexSize,
                                       uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      p_PerInstanceDataFragmentSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerMaterial), GpuProgramType::kFragment,
      UniformManager::_perMaterialUniformBuffer, UboType::kPerMaterialFragment,
      sizeof(PerMaterialDataFragment));
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedoTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descNormalTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(roughnessTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descPbrTextureName(p_Material)),
      Samplers::kLinearRepeat);
}

// <-

_INTR_INLINE void
initBindInfosPerPixelPicking(DrawCallRef p_DrawCallRef, MaterialRef p_Material,
                             uint32_t p_PerInstanceDataVertexSize,
                             uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      p_PerInstanceDataFragmentSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerMaterial), GpuProgramType::kFragment,
      UniformManager::_perMaterialUniformBuffer, UboType::kPerMaterialFragment,
      sizeof(PerMaterialDataFragment));
}

// <-

_INTR_INLINE void
initBindInfosGBufferLayered(DrawCallRef p_DrawCallRef, MaterialRef p_Material,
                            uint32_t p_PerInstanceDataVertexSize,
                            uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      p_PerInstanceDataFragmentSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerMaterial), GpuProgramType::kFragment,
      UniformManager::_perMaterialUniformBuffer, UboType::kPerMaterialFragment,
      sizeof(PerMaterialDataFragment));
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex0), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedoTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex0), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descNormalTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(roughnessTex0), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descPbrTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex1), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedo1TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex1), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descNormal1TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(roughnessTex1), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descPbr1TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex2), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedo2TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex2), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descNormal2TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(roughnessTex2), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descPbr2TextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(blendMaskTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descBlendMaskTextureName(p_Material)),
      Samplers::kLinearRepeat);
}

// <-

_INTR_INLINE void
initBindInfosGBufferWater(DrawCallRef p_DrawCallRef, MaterialRef p_Material,
                          uint32_t p_PerInstanceDataVertexSize,
                          uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      p_PerInstanceDataFragmentSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerMaterial), GpuProgramType::kFragment,
      UniformManager::_perMaterialUniformBuffer, UboType::kPerMaterialFragment,
      sizeof(PerMaterialDataFragment));
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedoTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descNormalTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(roughnessTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descPbrTextureName(p_Material)),
      Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(gbufferDepthTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(_N(GBufferDepth)),
      Samplers::kNearestClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(foamTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descFoamTextureName(p_Material)),
      Samplers::kLinearRepeat);
}

// <-

_INTR_INLINE void
initBindInfosShadowFoliage(DrawCallRef p_DrawCallRef, MaterialRef p_Material,
                           uint32_t p_PerInstanceDataVertexSize,
                           uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      p_PerInstanceDataFragmentSize);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerMaterial), GpuProgramType::kFragment,
      UniformManager::_perMaterialUniformBuffer, UboType::kPerMaterialFragment,
      sizeof(PerMaterialDataFragment));
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex), GpuProgramType::kFragment,
      Resources::ImageManager::getResourceByName(
          MaterialManager::_descAlbedoTextureName(p_Material)),
      Samplers::kLinearRepeat);
}

// <-

_INTR_INLINE void initBindInfosShadow(DrawCallRef p_DrawCallRef,
                                      MaterialRef p_Material,
                                      uint32_t p_PerInstanceDataVertexSize,
                                      uint32_t p_PerInstanceDataFragmentSize)
{
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kVertex,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
      p_PerInstanceDataVertexSize);
}
}

// Static members
_INTR_ARRAY(DrawCallRef)
DrawCallManager::_drawCallsPerMaterialPass[MaterialPass::kCount];

// <-

void DrawCallManager::createResources(const DrawCallRefArray& p_DrawCalls)
{
  for (uint32_t dcIdx = 0u; dcIdx < p_DrawCalls.size(); ++dcIdx)
  {
    DrawCallRef drawCallRef = p_DrawCalls[dcIdx];

    _INTR_ARRAY(BindingInfo)& bindInfs = _descBindInfos(drawCallRef);
    PipelineRef pipelineRef = _descPipeline(drawCallRef);
    PipelineLayoutRef pipelineLayout =
        Resources::PipelineManager::_descPipelineLayout(pipelineRef);
    _INTR_ARRAY(BufferRef)& descVtxBuffers = _descVertexBuffers(drawCallRef);
    _INTR_ARRAY(VkBuffer)& vtxBuffers = _vertexBuffers(drawCallRef);

    VkDescriptorSet& descSet = _vkDescriptorSet(drawCallRef);
    _INTR_ASSERT(descSet == VK_NULL_HANDLE);

    descSet = Resources::PipelineLayoutManager::allocateAndWriteDescriptorSet(
        pipelineLayout, bindInfs);

    // Defaults for now
    _indexBufferOffset(drawCallRef) = 0ull;
    _vertexBufferOffsets(drawCallRef).resize(descVtxBuffers.size());
    _vertexBuffers(drawCallRef).resize(descVtxBuffers.size());

    for (uint32_t i = 0u; i < descVtxBuffers.size(); ++i)
    {
      vtxBuffers[i] = BufferManager::_vkBuffer(descVtxBuffers[i]);
    }

    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(drawCallRef);

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
    if (materialPass != MaterialPass::kNone)
    {
      _drawCallsPerMaterialPass[materialPass].push_back(drawCallRef);
    }
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
    uint32_t p_PerInstanceDataFragmentSize)
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
    _descPipeline(drawCallMesh) = MaterialManager::_pipelines[p_MaterialPass];
    _descVertexBuffers(drawCallMesh) =
        Core::Resources::MeshManager::_vertexBuffersPerSubMesh(p_Mesh)[0];
    _descIndexBuffer(drawCallMesh) =
        Core::Resources::MeshManager::_indexBufferPerSubMesh(p_Mesh)[0];
    _descVertexCount(drawCallMesh) =
        (uint32_t)Core::Resources::MeshManager::_descPositionsPerSubMesh(
            p_Mesh)[0]
            .size();
    _descIndexCount(drawCallMesh) =
        (uint32_t)Core::Resources::MeshManager::_descIndicesPerSubMesh(
            p_Mesh)[0]
            .size();
    _descMaterial(drawCallMesh) = p_Material;
    _descMaterialPass(drawCallMesh) = p_MaterialPass;

    if (p_MaterialPass == MaterialPass::kSurface ||
        p_MaterialPass == MaterialPass::kSky ||
        p_MaterialPass == MaterialPass::kFoliage ||
        p_MaterialPass == MaterialPass::kDebugGizmo ||
        p_MaterialPass == MaterialPass::kDebugGrid)
    {
      initBindInfosGBuffer(drawCallMesh, p_Material,
                           p_PerInstanceDataVertexSize,
                           p_PerInstanceDataFragmentSize);
    }
    else if (p_MaterialPass == MaterialPass::kSurfaceLayered)
    {
      initBindInfosGBufferLayered(drawCallMesh, p_Material,
                                  p_PerInstanceDataVertexSize,
                                  p_PerInstanceDataFragmentSize);
    }
    else if (p_MaterialPass == MaterialPass::kSurfaceWater)
    {
      initBindInfosGBufferWater(drawCallMesh, p_Material,
                                p_PerInstanceDataVertexSize,
                                p_PerInstanceDataFragmentSize);
    }
    else if (p_MaterialPass == MaterialPass::kShadow)
    {
      initBindInfosShadow(drawCallMesh, p_Material, p_PerInstanceDataVertexSize,
                          p_PerInstanceDataFragmentSize);
    }
    else if (p_MaterialPass == MaterialPass::kShadowFoliage)
    {
      initBindInfosShadowFoliage(drawCallMesh, p_Material,
                                 p_PerInstanceDataVertexSize,
                                 p_PerInstanceDataFragmentSize);
    }
    else if (p_MaterialPass == MaterialPass::kPerPixelPicking)
    {
      initBindInfosPerPixelPicking(drawCallMesh, p_Material,
                                   p_PerInstanceDataVertexSize,
                                   p_PerInstanceDataFragmentSize);
    }
  }

  return drawCallMesh;
}
}
}
}
}
