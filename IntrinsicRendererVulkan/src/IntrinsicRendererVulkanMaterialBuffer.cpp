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
