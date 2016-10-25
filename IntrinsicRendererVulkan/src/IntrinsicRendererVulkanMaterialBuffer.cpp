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
// Static members
Resources::BufferRef MaterialBuffer::_materialBuffer;

_INTR_ARRAY(uint32_t) MaterialBuffer::_materialBufferEntries;
VkBuffer MaterialBuffer::_materialStagingBuffer;
VkDeviceMemory MaterialBuffer::_materialStagingBufferMemory;

void MaterialBuffer::init()
{
  Resources::BufferRefArray buffersToCreate;

  _materialBuffer = Resources::BufferManager::createBuffer(_N(_MaterialBuffer));
  {
    Resources::BufferManager::resetToDefault(_materialBuffer);
    Resources::BufferManager::addResourceFlags(
        _materialBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

    Resources::BufferManager::_descBufferType(_materialBuffer) =
        BufferType::kStorage;
    Resources::BufferManager::_descSizeInBytes(_materialBuffer) =
        sizeof(MaterialBufferEntry) * _INTR_MAX_MATERIAL_COUNT;
    buffersToCreate.push_back(_materialBuffer);
  }

  Resources::BufferManager::createResources(buffersToCreate);

  // Create staging buffer
  {
    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = sizeof(MaterialBufferEntry);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                     nullptr, &_materialStagingBuffer);
    _INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice,
                                  _materialStagingBuffer, &memReqs);

    VkMemoryAllocateInfo stagingBufferAllocInfo = {};
    {
      stagingBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      stagingBufferAllocInfo.pNext = nullptr;
      stagingBufferAllocInfo.memoryTypeIndex = Helper::computeGpuMemoryTypeIdx(
          memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      stagingBufferAllocInfo.allocationSize = memReqs.size;
    }

    result = vkAllocateMemory(RenderSystem::_vkDevice, &stagingBufferAllocInfo,
                              nullptr, &_materialStagingBufferMemory);
    _INTR_VK_CHECK_RESULT(result);

    result = vkBindBufferMemory(RenderSystem::_vkDevice, _materialStagingBuffer,
                                _materialStagingBufferMemory, 0u);
    _INTR_VK_CHECK_RESULT(result);
  }

  _materialBufferEntries.clear();
  for (uint32_t i = 0u; i < _INTR_MAX_MATERIAL_COUNT; ++i)
  {
    _materialBufferEntries.push_back(_INTR_MAX_MATERIAL_COUNT - 1u - i);
  }
}
}
}
}
