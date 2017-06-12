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
typedef Dod::Ref ComputeCallRef;
typedef _INTR_ARRAY(ComputeCallRef) ComputeCallRefArray;

struct ComputeCallData : Dod::Resources::ResourceDataBase
{
  ComputeCallData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_COMPUTE_CALL_COUNT)
  {
    descPipeline.resize(_INTR_MAX_COMPUTE_CALL_COUNT);
    descBindInfos.resize(_INTR_MAX_COMPUTE_CALL_COUNT);
    descDimensions.resize(_INTR_MAX_COMPUTE_CALL_COUNT);

    dynamicOffsets.resize(_INTR_MAX_COMPUTE_CALL_COUNT);
    vkDescriptorSet.resize(_INTR_MAX_COMPUTE_CALL_COUNT);
  }

  _INTR_ARRAY(PipelineRef) descPipeline;
  _INTR_ARRAY(_INTR_ARRAY(BindingInfo)) descBindInfos;
  _INTR_ARRAY(glm::uvec3) descDimensions;

  // GPU resources
  _INTR_ARRAY(_INTR_ARRAY(uint32_t)) dynamicOffsets;
  _INTR_ARRAY(VkDescriptorSet) vkDescriptorSet;
};

struct ComputeCallManager
    : Dod::Resources::ResourceManagerBase<ComputeCallData,
                                          _INTR_MAX_COMPUTE_CALL_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Compute Call Manager...");

    Dod::Resources::ResourceManagerBase<
        ComputeCallData, _INTR_MAX_COMPUTE_CALL_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static ComputeCallRef createComputeCall(const Name& p_Name)
  {
    ComputeCallRef ref = Dod::Resources::ResourceManagerBase<
        ComputeCallData, _INTR_MAX_COMPUTE_CALL_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(BufferRef p_Ref)
  {
    _descPipeline(p_Ref) = PipelineRef();
    _descBindInfos(p_Ref).clear();
    _descDimensions(p_Ref) = glm::uvec3(1u, 1u, 1u);
  }

  _INTR_INLINE static void destroyComputeCall(ComputeCallRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        ComputeCallData, _INTR_MAX_COMPUTE_CALL_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(ComputeCallRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        ComputeCallData,
        _INTR_MAX_COMPUTE_CALL_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                          p_Properties,
                                                          p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(ComputeCallRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        ComputeCallData,
        _INTR_MAX_COMPUTE_CALL_COUNT>::_initFromDescriptor(p_Ref, p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        ComputeCallData,
        _INTR_MAX_COMPUTE_CALL_COUNT>::_saveToSingleFile(p_FileName,
                                                         compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        ComputeCallData,
        _INTR_MAX_COMPUTE_CALL_COUNT>::_loadFromSingleFile(p_FileName,
                                                           initFromDescriptor,
                                                           resetToDefault);
  }

  // <-

  static void updateUniformMemory(const ComputeCallRefArray& p_ComputeCalls,
                                  void* p_PerInstanceDataCompute,
                                  uint32_t p_PerInstanceDataComputeSize);

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const ComputeCallRefArray& p_ComputeCalls);

  _INTR_INLINE static void
  destroyResources(const ComputeCallRefArray& p_ComputeCalls)
  {
    for (uint32_t i = 0u; i < p_ComputeCalls.size(); ++i)
    {
      ComputeCallRef ComputeCallRef = p_ComputeCalls[i];

      VkDescriptorSet& vkDescSet = _vkDescriptorSet(ComputeCallRef);
      PipelineRef pipeline = _descPipeline(ComputeCallRef);
      _INTR_ASSERT(pipeline.isValid());
      PipelineLayoutRef pipelineLayout =
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

      _dynamicOffsets(ComputeCallRef).clear();
    }
  }

  // <-

  _INTR_INLINE static void
  destroyComputeCallsAndResources(const DrawCallRefArray& p_ComputeCalls)
  {
    destroyResources(p_ComputeCalls);

    for (uint32_t i = 0u; i < p_ComputeCalls.size(); ++i)
    {
      destroyComputeCall(p_ComputeCalls[i]);
    }
  }

  // <-

  static void bindImage(ComputeCallRef p_DrawCallRef, const Name& p_Name,
                        uint8_t p_ShaderStage, Dod::Ref p_ImageRef,
                        uint8_t p_SamplerIdx, uint8_t p_BindingFlags = 0u,
                        uint8_t p_ArrayLayerIdx = 0u,
                        uint8_t p_MipLevelIdx = 0u);
  static void bindBuffer(ComputeCallRef p_DrawCallRef, const Name& p_Name,
                         uint8_t p_ShaderStage, Dod::Ref p_BufferRef,
                         uint8_t p_UboType, uint32_t p_RangeInBytes,
                         uint32_t p_OffsetInBytes = 0u);

  // Members refs.
  // ->

  _INTR_INLINE static PipelineRef& _descPipeline(PipelineLayoutRef p_Ref)
  {
    return _data.descPipeline[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(BindingInfo) &
      _descBindInfos(PipelineLayoutRef p_Ref)
  {
    return _data.descBindInfos[p_Ref._id];
  }
  _INTR_INLINE static glm::uvec3& _descDimensions(PipelineLayoutRef p_Ref)
  {
    return _data.descDimensions[p_Ref._id];
  }

  _INTR_INLINE static _INTR_ARRAY(uint32_t) &
      _dynamicOffsets(PipelineLayoutRef p_Ref)
  {
    return _data.dynamicOffsets[p_Ref._id];
  }
  _INTR_INLINE static VkDescriptorSet& _vkDescriptorSet(PipelineLayoutRef p_Ref)
  {
    return _data.vkDescriptorSet[p_Ref._id];
  }
};
}
}
}
}
