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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
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

  _INTR_ARRAY(_INTR_ARRAY(BindingDescription)) bindingDescs;

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
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_compileDescriptor(p_Ref,
                                                             p_Properties,
                                                             p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(PipelineLayoutRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineLayoutData,
        _INTR_MAX_PIPELINE_LAYOUT_COUNT>::_initFromDescriptor(p_Ref,
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

  static void createResources(const PipelineLayoutRefArray& p_Pipelinelayouts);

  _INTR_INLINE static void
  destroyResources(const PipelineLayoutRefArray& p_Pipelinelayouts)
  {
    for (uint32_t i = 0u; i < p_Pipelinelayouts.size(); ++i)
    {
      PipelineLayoutRef ref = p_Pipelinelayouts[i];

      VkPipelineLayout& pipelineLayout = _vkPipelineLayout(ref);
      if (pipelineLayout != VK_NULL_HANDLE)
      {
        vkDestroyPipelineLayout(RenderSystem::_vkDevice, pipelineLayout,
                                nullptr);
        pipelineLayout = VK_NULL_HANDLE;
      }

      VkDescriptorSetLayout& descSetLayout = _vkDescriptorSetLayout(ref);
      if (descSetLayout != VK_NULL_HANDLE)
      {
        vkDestroyDescriptorSetLayout(RenderSystem::_vkDevice, descSetLayout,
                                     nullptr);
        descSetLayout = VK_NULL_HANDLE;
      }

      VkDescriptorPool& descPool = _vkDescriptorPool(ref);
      if (descPool != VK_NULL_HANDLE)
      {
        vkDestroyDescriptorPool(RenderSystem::_vkDevice, descPool, nullptr);
        descPool = VK_NULL_HANDLE;
      }
    }
  }

  static VkDescriptorSet
  allocateAndWriteDescriptorSet(PipelineLayoutRef p_Ref,
                                const _INTR_ARRAY(BindingInfo) & p_BindInfos);

  // Getter/Setter
  // ->

  _INTR_INLINE static _INTR_ARRAY(BindingDescription) &
      _descBindingDescs(PipelineLayoutRef p_Ref)
  {
    return _data.bindingDescs[p_Ref._id];
  }

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
}
