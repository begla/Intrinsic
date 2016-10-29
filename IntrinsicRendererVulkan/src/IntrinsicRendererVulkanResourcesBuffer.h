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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
typedef Dod::Ref BufferRef;
typedef _INTR_ARRAY(BufferRef) BufferRefArray;

struct BufferData : Dod::Resources::ResourceDataBase
{
  BufferData() : Dod::Resources::ResourceDataBase(_INTR_MAX_BUFFER_COUNT)
  {
    descBufferType.resize(_INTR_MAX_BUFFER_COUNT);
    descBufferMemoryUsage.resize(_INTR_MAX_BUFFER_COUNT);
    descSizeInBytes.resize(_INTR_MAX_BUFFER_COUNT);
    descInitialData.resize(_INTR_MAX_BUFFER_COUNT);

    vkDescriptorBufferInfo.resize(_INTR_MAX_BUFFER_COUNT);
    vkBuffer.resize(_INTR_MAX_BUFFER_COUNT);
    vkDeviceMemory.resize(_INTR_MAX_BUFFER_COUNT);
    memoryOffset.resize(_INTR_MAX_BUFFER_COUNT);

    for (uint32_t i = 0u; i < _INTR_MAX_BUFFER_COUNT; ++i)
    {
      memoryOffset[i] = (uint32_t)-1;
    }
  }

  // <-

  _INTR_ARRAY(BufferType::Enum) descBufferType;
  _INTR_ARRAY(MemoryUsage::Enum) descBufferMemoryUsage;
  _INTR_ARRAY(uint32_t) descSizeInBytes;
  _INTR_ARRAY(void*) descInitialData;

  _INTR_ARRAY(VkDescriptorBufferInfo) vkDescriptorBufferInfo;
  _INTR_ARRAY(VkBuffer) vkBuffer;
  _INTR_ARRAY(VkDeviceMemory) vkDeviceMemory;
  _INTR_ARRAY(uint32_t) memoryOffset;
};

struct BufferManager
    : Dod::Resources::ResourceManagerBase<BufferData, _INTR_MAX_BUFFER_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Buffer Manager...");

    Dod::Resources::ResourceManagerBase<
        BufferData, _INTR_MAX_BUFFER_COUNT>::_initResourceManager();
  }

  // <-

  _INTR_INLINE static BufferRef createBuffer(const Name& p_Name)
  {
    BufferRef ref = Dod::Resources::ResourceManagerBase<
        BufferData, _INTR_MAX_BUFFER_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(BufferRef p_Ref)
  {
    _descBufferType(p_Ref) = BufferType::kVertex;
    _descSizeInBytes(p_Ref) = 0u;
    _descBufferMemoryUsage(p_Ref) = MemoryUsage::kOptimal;
    _descInitialData(p_Ref) = nullptr;
  }

  // <-

  _INTR_INLINE static void destroyBuffer(BufferRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        BufferData, _INTR_MAX_BUFFER_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(BufferRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        BufferData, _INTR_MAX_BUFFER_COUNT>::_compileDescriptor(p_Ref,
                                                                p_Properties,
                                                                p_Document);
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(BufferRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        BufferData, _INTR_MAX_BUFFER_COUNT>::_initFromDescriptor(p_Ref,
                                                                 p_Properties);
  }

  // <-

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<BufferData, _INTR_MAX_BUFFER_COUNT>::
        _saveToSingleFile(p_FileName, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<BufferData, _INTR_MAX_BUFFER_COUNT>::
        _loadFromSingleFile(p_FileName, initFromDescriptor, resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  static void createResources(const BufferRefArray& p_Buffers);

  // <-

  _INTR_INLINE static void destroyResources(const BufferRefArray& p_Buffers)
  {
    for (uint32_t i = 0u; i < p_Buffers.size(); ++i)
    {
      BufferRef ref = p_Buffers[i];

      VkBuffer& buffer = _vkBuffer(ref);
      if (buffer != VK_NULL_HANDLE)
      {
        RenderSystem::releaseResource(_N(VkBuffer), (void*)buffer, nullptr);
        buffer = VK_NULL_HANDLE;
      }

      _vkDeviceMemory(ref) = VK_NULL_HANDLE;
      _memoryOffset(ref) = (uint32_t)-1;
    }
  }

  // <-

  _INTR_INLINE static void
  destroyBuffersAndResources(const BufferRefArray& p_Buffers)
  {
    destroyResources(p_Buffers);

    for (uint32_t i = 0u; i < p_Buffers.size(); ++i)
    {
      destroyBuffer(p_Buffers[i]);
    }
  }

  // <-

  _INTR_INLINE static uint8_t* getGpuMemory(BufferRef p_Ref)
  {
    _INTR_ASSERT(_descBufferMemoryUsage(p_Ref) == MemoryUsage::kStaging &&
                 "Memory access not available for non-staging buffers");
    return GpuMemoryManager::getHostVisibleMemoryForOffset(
        _memoryOffset(p_Ref));
  }

  // <-

  _INTR_INLINE static void insertBufferMemoryBarrier(
      BufferRef p_Ref, VkAccessFlags p_SrcAccessMask,
      VkAccessFlags p_DstAccessMask,
      VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VkPipelineStageFlags p_DstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
  {
    Helper::insertBufferMemoryBarrier(RenderSystem::getPrimaryCommandBuffer(),
                                      _vkBuffer(p_Ref), _descSizeInBytes(p_Ref),
                                      0u, p_SrcAccessMask, p_DstAccessMask,
                                      p_SrcStages, p_DstStages);
  }

  // Members refs.
  // ->

  _INTR_INLINE static BufferType::Enum& _descBufferType(BufferRef p_Ref)
  {
    return _data.descBufferType[p_Ref._id];
  }
  _INTR_INLINE static void*& _descInitialData(BufferRef p_Ref)
  {
    return _data.descInitialData[p_Ref._id];
  }
  _INTR_INLINE static MemoryUsage::Enum& _descBufferMemoryUsage(BufferRef p_Ref)
  {
    return _data.descBufferMemoryUsage[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descSizeInBytes(BufferRef p_Ref)
  {
    return _data.descSizeInBytes[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkBuffer& _vkBuffer(BufferRef p_Ref)
  {
    return _data.vkBuffer[p_Ref._id];
  }
  _INTR_INLINE static VkDeviceMemory& _vkDeviceMemory(BufferRef p_Ref)
  {
    return _data.vkDeviceMemory[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _memoryOffset(BufferRef p_Ref)
  {
    return _data.memoryOffset[p_Ref._id];
  }
};
}
}
}
}
