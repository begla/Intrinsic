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
namespace Resources
{
// Typedefs
typedef Dod::Ref PipelineLayoutRef;
typedef Dod::RefArray PipelineLayoutRefArray;

struct PipelineLayoutData : Dod::Resources::ResourceDataBase
{
  PipelineLayoutData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_PIPELINE_LAYOUT_COUNT)
  {
    bindingDescs.resize(_INTR_MAX_PIPELINE_LAYOUT_COUNT);

    vkPipelineLayout.resize(_INTR_MAX_PIPELINE_LAYOUT_COUNT);
    vkDescriptorSetLayout.resize(_INTR_MAX_PIPELINE_LAYOUT_COUNT);
    vkDescriptorPool.resize(_INTR_MAX_PIPELINE_LAYOUT_COUNT);
  }

  // Description
  _INTR_ARRAY(_INTR_ARRAY(BindingDescription)) bindingDescs;

  // Resources
  _INTR_ARRAY(VkPipelineLayout) vkPipelineLayout;
  _INTR_ARRAY(VkDescriptorSetLayout) vkDescriptorSetLayout;
  _INTR_ARRAY(VkDescriptorPool) vkDescriptorPool;
};

struct PipelineLayoutManager
    : Dod::Resources::ResourceManagerBase<PipelineLayoutData,
                                          _INTR_MAX_PIPELINE_LAYOUT_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Pipeline Layout Manager...");

    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static PipelineLayoutRef createPipelineLayout(const Name& p_Name)
  {
    PipelineLayoutRef ref = Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(BufferRef p_Ref)
  {
    _descBindingDescs(p_Ref).clear();
  }

  _INTR_INLINE static void destroyPipelineLayout(PipelineLayoutRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(PipelineLayoutRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_compileDescriptor(p_Ref,
                                                             p_GenerateDesc,
                                                             p_Properties,
                                                             p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(PipelineLayoutRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_initFromDescriptor(p_Ref,
                                                              p_GenerateDesc,
                                                              p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_saveToSingleFile(p_FileName,
                                                            compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<PipelineLayoutData,
                                        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::
        _loadFromSingleFile(p_FileName, initFromDescriptor, resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const PipelineLayoutRefArray& p_PipelineLayouts);
  static void destroyResources(const PipelineLayoutRefArray& p_PipelineLayouts);

  // <-

  _INTR_INLINE static void destroyPipelineLayoutsAndResources(
      const PipelineLayoutRefArray& p_PipelineLayouts)
  {
    destroyResources(p_PipelineLayouts);

    for (uint32_t i = 0u; i < p_PipelineLayouts.size(); ++i)
    {
      destroyPipelineLayout(p_PipelineLayouts[i]);
    }
  }

  static VkDescriptorSet
  allocateAndWriteDescriptorSet(PipelineLayoutRef p_Ref,
                                const _INTR_ARRAY(BindingInfo) & p_BindInfos);

  // Description
  _INTR_INLINE static _INTR_ARRAY(BindingDescription) &
      _descBindingDescs(PipelineLayoutRef p_Ref)
  {
    return _data.bindingDescs[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static VkPipelineLayout&
  _vkPipelineLayout(PipelineLayoutRef p_Ref)
  {
    return _data.vkPipelineLayout[p_Ref._id];
  }
  _INTR_INLINE static VkDescriptorSetLayout&
  _vkDescriptorSetLayout(PipelineLayoutRef p_Ref)
  {
    return _data.vkDescriptorSetLayout[p_Ref._id];
  }
  _INTR_INLINE static VkDescriptorPool&
  _vkDescriptorPool(PipelineLayoutRef p_Ref)
  {
    return _data.vkDescriptorPool[p_Ref._id];
  }
};
}
}
}
