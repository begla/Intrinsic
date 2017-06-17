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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
// Typedefs
typedef Dod::Ref DrawCallRef;
typedef _INTR_ARRAY(DrawCallRef) DrawCallRefArray;

struct DrawCallData : Dod::Resources::ResourceDataBase
{
  DrawCallData() : Dod::Resources::ResourceDataBase(_INTR_MAX_DRAW_CALL_COUNT)
  {
    descVertexCount.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descIndexCount.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descInstanceCount.resize(_INTR_MAX_DRAW_CALL_COUNT);

    descPipeline.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descBindInfos.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descVertexBuffers.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descIndexBuffer.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descMaterial.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descMaterialPass.resize(_INTR_MAX_DRAW_CALL_COUNT);
    descMeshComponent.resize(_INTR_MAX_DRAW_CALL_COUNT);

    dynamicOffsets.resize(_INTR_MAX_DRAW_CALL_COUNT);
    vertexBuffers.resize(_INTR_MAX_DRAW_CALL_COUNT);
    vkDescriptorSet.resize(_INTR_MAX_DRAW_CALL_COUNT);
    vertexBufferOffsets.resize(_INTR_MAX_DRAW_CALL_COUNT);
    indexBufferOffset.resize(_INTR_MAX_DRAW_CALL_COUNT);
    sortingHash.resize(_INTR_MAX_DRAW_CALL_COUNT);
  }

  // Description
  _INTR_ARRAY(uint32_t) descVertexCount;
  _INTR_ARRAY(uint32_t) descIndexCount;
  _INTR_ARRAY(uint32_t) descInstanceCount;

  _INTR_ARRAY(PipelineRef) descPipeline;
  _INTR_ARRAY(_INTR_ARRAY(BindingInfo)) descBindInfos;
  _INTR_ARRAY(_INTR_ARRAY(BufferRef)) descVertexBuffers;
  _INTR_ARRAY(BufferRef) descIndexBuffer;
  _INTR_ARRAY(Dod::Ref) descMaterial;
  _INTR_ARRAY(uint8_t) descMaterialPass;
  _INTR_ARRAY(Dod::Ref) descMeshComponent;

  // Resources
  _INTR_ARRAY(_INTR_ARRAY(uint32_t)) dynamicOffsets;
  _INTR_ARRAY(VkDescriptorSet) vkDescriptorSet;
  _INTR_ARRAY(_INTR_ARRAY(VkDeviceSize)) vertexBufferOffsets;
  _INTR_ARRAY(_INTR_ARRAY(VkBuffer)) vertexBuffers;
  _INTR_ARRAY(VkDeviceSize) indexBufferOffset;
  _INTR_ARRAY(uint32_t) sortingHash;
};

struct DrawCallManager
    : Dod::Resources::ResourceManagerBase<DrawCallData,
                                          _INTR_MAX_DRAW_CALL_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Draw Call Manager...");

    Dod::Resources::ResourceManagerBase<
        DrawCallData, _INTR_MAX_DRAW_CALL_COUNT>::_initResourceManager();
  }

  // <-

  _INTR_INLINE static DrawCallRef createDrawCall(const Name& p_Name)
  {
    DrawCallRef ref = Dod::Resources::ResourceManagerBase<
        DrawCallData, _INTR_MAX_DRAW_CALL_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void allocateUniformMemory(DrawCallRef p_DrawCall)
  {
    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(p_DrawCall);
    static const uint32_t perFrameRangeSizeInBytes =
        sizeof(RenderProcess::PerFrameDataVertex) +
        sizeof(RenderProcess::PerFrameDataFrament);

    uint32_t dynamicOffsetIndex = 0u;
    for (uint32_t bIdx = 0u; bIdx < bindInfos.size(); ++bIdx)
    {
      BindingInfo& bindInfo = bindInfos[bIdx];

      if (bindInfo.bindingType == BindingType::kUniformBufferDynamic)
      {
        if (bindInfo.bufferData.uboType == UboType::kPerInstanceVertex)
        {
          UniformManager::allocatePerInstanceDataMemory(
              bindInfo.bufferData.rangeInBytes,
              _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex]);
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerInstanceFragment)
        {
          UniformManager::allocatePerInstanceDataMemory(
              bindInfo.bufferData.rangeInBytes,
              _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex]);
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerMaterialVertex)
        {
          _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex] =
              MaterialManager::_perMaterialDataVertexOffset(
                  _descMaterial(p_DrawCall));
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerMaterialFragment)
        {
          _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex] =
              MaterialManager::_perMaterialDataFragmentOffset(
                  _descMaterial(p_DrawCall));
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerFrameVertex)
        {
          _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex] = RenderProcess::
              UniformManager::getDynamicOffsetForPerFrameDataVertex();
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerFrameFragment)
        {
          _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex] = RenderProcess::
              UniformManager::getDynamicOffsetForPerFrameDataFragment();
        }

        ++dynamicOffsetIndex;
      }
    }
  }

  // <-

  _INTR_INLINE static void
  allocateUniformMemory(const DrawCallRefArray& p_DrawCalls,
                        uint32_t p_FirstIdx, uint32_t p_Count)
  {
    for (uint32_t dcIdx = p_FirstIdx; dcIdx < p_Count; ++dcIdx)
    {
      Resources::DrawCallRef dcRef = p_DrawCalls[dcIdx];
      allocateUniformMemory(dcRef);
    }
  }

  // <-

  _INTR_INLINE static void
  updateUniformMemory(DrawCallRef p_DrawCall, void* p_PerInstanceDataVertex,
                      uint32_t p_PerInstanceDataVertexSize,
                      void* p_PerInstanceDataFragment,
                      uint32_t p_PerInstanceDataFragmentSize)
  {
    _INTR_ARRAY(BindingInfo)& bindInfos = _descBindInfos(p_DrawCall);

    uint32_t dynamicOffsetIndex = 0u;
    for (uint32_t bIdx = 0u; bIdx < bindInfos.size(); ++bIdx)
    {
      BindingInfo& bindInfo = bindInfos[bIdx];

      if (bindInfo.bindingType == BindingType::kUniformBufferDynamic)
      {
        const uint32_t dynamicOffset =
            _dynamicOffsets(p_DrawCall)[dynamicOffsetIndex];

        if (bindInfo.bufferData.uboType == UboType::kPerInstanceVertex)
        {
          uint8_t* gpuMem = &UniformManager::_perInstanceMemory[dynamicOffset];
          memcpy(gpuMem, p_PerInstanceDataVertex, p_PerInstanceDataVertexSize);
        }
        else if (bindInfo.bufferData.uboType == UboType::kPerInstanceFragment)
        {
          uint8_t* gpuMem = &UniformManager::_perInstanceMemory[dynamicOffset];
          memcpy(gpuMem, p_PerInstanceDataFragment,
                 p_PerInstanceDataFragmentSize);
        }

        ++dynamicOffsetIndex;
      }
    }
  }

  // <-

  _INTR_INLINE static void
  updateUniformMemory(const DrawCallRefArray& p_DrawCalls, uint32_t p_FirstIdx,
                      uint32_t p_Count, void* p_PerInstanceDataVertex,
                      uint32_t p_PerInstanceDataVertexSize,
                      void* p_PerInstanceDataFragment,
                      uint32_t p_PerInstanceDataFragmentSize)
  {
    for (uint32_t dcIdx = p_FirstIdx; dcIdx < p_Count; ++dcIdx)
    {
      Resources::DrawCallRef dcRef = p_DrawCalls[dcIdx];
      updateUniformMemory(
          dcRef, p_PerInstanceDataVertex, p_PerInstanceDataVertexSize,
          p_PerInstanceDataFragment, p_PerInstanceDataFragmentSize);
    }
  }

  // <-

  _INTR_INLINE static void updateSortingHash(DrawCallRef p_DrawCall,
                                             float p_DistToCamera2)
  {
    _sortingHash(p_DrawCall) =
        (_descMaterialPass(p_DrawCall) << 16u) | (uint16_t)p_DistToCamera2;
  }

  // <-

  static void allocateAndUpdateUniformMemory(
      const DrawCallRefArray& p_DrawCalls, void* p_PerInstanceDataVertex,
      uint32_t p_PerInstanceDataVertexSize, void* p_PerInstanceDataFragment,
      uint32_t p_PerInstanceDataFragmentSize)
  {
    allocateUniformMemory(p_DrawCalls, 0u, (uint32_t)p_DrawCalls.size());
    updateUniformMemory(p_DrawCalls, 0u, (uint32_t)p_DrawCalls.size(),
                        p_PerInstanceDataVertex, p_PerInstanceDataVertexSize,
                        p_PerInstanceDataFragment,
                        p_PerInstanceDataFragmentSize);
  }

  // <-

  static DrawCallRef createDrawCallForMesh(
      const Name& p_Name, Dod::Ref p_Mesh, Dod::Ref p_Material,
      uint8_t p_MaterialPass, uint32_t p_PerInstanceDataVertexSize,
      uint32_t p_PerInstanceDataFragmentSize, uint32_t p_SubMeshIdx = 0u);

  // <-

  _INTR_INLINE static void resetToDefault(BufferRef p_Ref)
  {
    _descVertexCount(p_Ref) = 0u;
    _descIndexCount(p_Ref) = 0u;
    _descInstanceCount(p_Ref) = 1u;
    _descPipeline(p_Ref) = PipelineRef();
    _descBindInfos(p_Ref).clear();
    _descVertexBuffers(p_Ref).clear();
    _descIndexBuffer(p_Ref) = BufferRef();
    _descMaterial(p_Ref) = Dod::Ref();
    _descMaterialPass(p_Ref) = 0u;
    _descMeshComponent(p_Ref) = Dod::Ref();
  }

  _INTR_INLINE static void destroyDrawCall(DrawCallRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        DrawCallData, _INTR_MAX_DRAW_CALL_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(DrawCallRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        DrawCallData,
        _INTR_MAX_DRAW_CALL_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                       p_Properties,
                                                       p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(DrawCallRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        DrawCallData,
        _INTR_MAX_DRAW_CALL_COUNT>::_initFromDescriptor(p_Ref, p_GenerateDesc,
                                                        p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        DrawCallData,
        _INTR_MAX_DRAW_CALL_COUNT>::_saveToSingleFile(p_FileName,
                                                      compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        DrawCallData,
        _INTR_MAX_DRAW_CALL_COUNT>::_loadFromSingleFile(p_FileName,
                                                        initFromDescriptor,
                                                        resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  static void createResources(const DrawCallRefArray& p_DrawCalls);

  // <-

  _INTR_INLINE static void destroyResources(const DrawCallRefArray& p_DrawCalls)
  {
    for (uint32_t i = 0u; i < p_DrawCalls.size(); ++i)
    {
      DrawCallRef drawCallRef = p_DrawCalls[i];

      VkDescriptorSet& vkDescSet = _vkDescriptorSet(drawCallRef);
      PipelineRef pipeline = _descPipeline(drawCallRef);
      _INTR_ASSERT(pipeline.isValid());
      DrawCallRef pipelineLayout =
          PipelineManager::_descPipelineLayout(pipeline);
      _INTR_ASSERT(pipelineLayout.isValid());

      if (vkDescSet != VK_NULL_HANDLE)
      {
        VkDescriptorPool vkDescPool =
            PipelineLayoutManager::_vkDescriptorPool(pipelineLayout);
        _INTR_ASSERT(vkDescPool != VK_NULL_HANDLE);

        RenderSystem::releaseResource(_N(VkDescriptorSet), (void*)vkDescSet,
                                      (void*)vkDescPool);
        vkDescSet = VK_NULL_HANDLE;
      }

      _vertexBufferOffsets(drawCallRef).clear();
      _indexBufferOffset(drawCallRef) = 0ull;
      _dynamicOffsets(drawCallRef).clear();

      // Remove from per material pass array
      uint8_t materialPass = _descMaterialPass(drawCallRef);
      {
        uint32_t dcCount =
            (uint32_t)_drawCallsPerMaterialPass[materialPass].size();
        for (uint32_t i = 0u; i < dcCount; ++i)
        {
          if (_drawCallsPerMaterialPass[materialPass][i] == drawCallRef)
          {
            // Erase and swap
            _drawCallsPerMaterialPass[materialPass][i] =
                _drawCallsPerMaterialPass[materialPass][dcCount - 1u];
            _drawCallsPerMaterialPass[materialPass].resize(dcCount - 1u);
            break;
          }
        }
      }
    }
  }

  // <-

  _INTR_INLINE static void
  destroyDrawCallsAndResources(const DrawCallRefArray& p_DrawCalls)
  {
    destroyResources(p_DrawCalls);

    for (uint32_t i = 0u; i < p_DrawCalls.size(); ++i)
    {
      destroyDrawCall(p_DrawCalls[i]);
    }
  }

  // <-

  _INTR_INLINE static void
  sortDrawCallsFrontToBack(DrawCallRefArray& p_RefArray)
  {
    _INTR_PROFILE_CPU("General", "Sort Draw Calls");

    struct Comparator
    {
      bool operator()(const Dod::Ref& a, const Dod::Ref& b) const
      {
        return _sortingHash(a) < _sortingHash(b);
      }
    } comp;

    Algorithm::parallelSort<Dod::Ref, Comparator>(p_RefArray, comp);
  }

  // <-

  _INTR_INLINE static void
  sortDrawCallsBackToFront(DrawCallRefArray& p_RefArray)
  {
    _INTR_PROFILE_CPU("General", "Sort Draw Calls");

    struct Comparator
    {
      bool operator()(const Dod::Ref& a, const Dod::Ref& b) const
      {
        return _sortingHash(a) > _sortingHash(b);
      }
    } comp;

    Algorithm::parallelSort<Dod::Ref, Comparator>(p_RefArray, comp);
  }

  // <-

  static void bindImage(DrawCallRef p_DrawCallRef, const Name& p_Name,
                        uint8_t p_ShaderStage, Dod::Ref p_ImageRef,
                        uint8_t p_SamplerIdx, uint8_t p_BindingFlags = 0u,
                        uint8_t p_ArrayLayerIdx = 0u,
                        uint8_t p_MipLevelIdx = 0u);
  static void bindBuffer(DrawCallRef p_DrawCallRef, const Name& p_Name,
                         uint8_t p_ShaderStage, Dod::Ref p_BufferRef,
                         uint8_t p_UboType, uint32_t p_RangeInBytes,
                         uint32_t p_OffsetInBytes = 0u);

  // Description
  _INTR_INLINE static uint32_t& _descVertexCount(DrawCallRef p_Ref)
  {
    return _data.descVertexCount[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descIndexCount(DrawCallRef p_Ref)
  {
    return _data.descIndexCount[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descInstanceCount(DrawCallRef p_Ref)
  {
    return _data.descInstanceCount[p_Ref._id];
  }

  _INTR_INLINE static PipelineRef& _descPipeline(DrawCallRef p_Ref)
  {
    return _data.descPipeline[p_Ref._id];
  }

  _INTR_INLINE static _INTR_ARRAY(BindingInfo) &
      _descBindInfos(DrawCallRef p_Ref)
  {
    return _data.descBindInfos[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(BufferRef) &
      _descVertexBuffers(DrawCallRef p_Ref)
  {
    return _data.descVertexBuffers[p_Ref._id];
  }
  _INTR_INLINE static BufferRef& _descIndexBuffer(DrawCallRef p_Ref)
  {
    return _data.descIndexBuffer[p_Ref._id];
  }

  _INTR_INLINE static Dod::Ref& _descMaterial(DrawCallRef p_Ref)
  {
    return _data.descMaterial[p_Ref._id];
  }
  _INTR_INLINE static Dod::Ref& _descMeshComponent(DrawCallRef p_Ref)
  {
    return _data.descMeshComponent[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descMaterialPass(DrawCallRef p_Ref)
  {
    return _data.descMaterialPass[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static uint32_t& _sortingHash(DrawCallRef p_Ref)
  {
    return _data.sortingHash[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(uint32_t) & _dynamicOffsets(DrawCallRef p_Ref)
  {
    return _data.dynamicOffsets[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(VkDeviceSize) &
      _vertexBufferOffsets(DrawCallRef p_Ref)
  {
    return _data.vertexBufferOffsets[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(VkBuffer) & _vertexBuffers(DrawCallRef p_Ref)
  {
    return _data.vertexBuffers[p_Ref._id];
  }
  _INTR_INLINE static VkDeviceSize& _indexBufferOffset(DrawCallRef p_Ref)
  {
    return _data.indexBufferOffset[p_Ref._id];
  }
  _INTR_INLINE static VkDescriptorSet& _vkDescriptorSet(DrawCallRef p_Ref)
  {
    return _data.vkDescriptorSet[p_Ref._id];
  }

  // Static members
  static _INTR_ARRAY(_INTR_ARRAY(DrawCallRef)) _drawCallsPerMaterialPass;
};
}
}
}
}
