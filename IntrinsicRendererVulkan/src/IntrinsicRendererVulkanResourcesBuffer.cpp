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
namespace Resources
{
void BufferManager::createResources(const BufferRefArray& p_Buffers)
{
  for (uint32_t i = 0u; i < p_Buffers.size(); ++i)
  {
    BufferRef bufferRef = p_Buffers[i];

    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage =
          Helper::mapBufferTypeToVkUsageFlagBits(_descBufferType(bufferRef)) |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _descSizeInBytes(bufferRef);
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkBuffer& buffer = _vkBuffer(bufferRef);
    _INTR_ASSERT(buffer == VK_NULL_HANDLE);

    VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                     nullptr, &buffer);
    _INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, buffer, &memReqs);

    VkFlags reqMask = 0u;
    if (_descBufferMemoryUsage(bufferRef) ==
        MemoryUsage::kHostVisibleAndCoherent)
    {
      reqMask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    VkMemoryAllocateInfo allocInfo = {};
    {
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.pNext = nullptr;
      allocInfo.memoryTypeIndex =
          Helper::computeGpuMemoryTypeIdx(memReqs.memoryTypeBits, reqMask);
      allocInfo.allocationSize = memReqs.size;
    }

    VkDeviceMemory& mem = _vkDeviceMemory(bufferRef);
    result =
        vkAllocateMemory(RenderSystem::_vkDevice, &allocInfo, nullptr, &mem);
    _INTR_VK_CHECK_RESULT(result);

    result = vkBindBufferMemory(RenderSystem::_vkDevice, buffer, mem, 0u);
    _INTR_VK_CHECK_RESULT(result);

    void* initialData = _descInitialData(bufferRef);
    if (initialData)
    {
      // Init. temp staging buffer
      VkMemoryAllocateInfo stagingBufferAllocInfo = {};
      {
        stagingBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        stagingBufferAllocInfo.pNext = nullptr;
        stagingBufferAllocInfo.memoryTypeIndex =
            Helper::computeGpuMemoryTypeIdx(
                memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBufferAllocInfo.allocationSize = memReqs.size;
      }

      VkDeviceMemory stagingMemory;
      result =
          vkAllocateMemory(RenderSystem::_vkDevice, &stagingBufferAllocInfo,
                           nullptr, &stagingMemory);
      _INTR_VK_CHECK_RESULT(result);

      VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
      {
        stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      }

      VkBuffer stagingBuffer;
      result = vkCreateBuffer(RenderSystem::_vkDevice, &stagingBufferCreateInfo,
                              nullptr, &stagingBuffer);
      _INTR_VK_CHECK_RESULT(result);

      result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                                  stagingMemory, 0u);
      _INTR_VK_CHECK_RESULT(result);

      // Copy initial data to staging memory
      {
        void* stagingMemMapped;
        result =
            vkMapMemory(RenderSystem::_vkDevice, stagingMemory, 0u,
                        _descSizeInBytes(bufferRef), 0u, &stagingMemMapped);
        _INTR_VK_CHECK_RESULT(result);

        memcpy(stagingMemMapped, initialData, _descSizeInBytes(bufferRef));

        vkUnmapMemory(RenderSystem::_vkDevice, stagingMemory);
      }

      VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

      VkBufferCopy bufferCopy = {};
      {
        bufferCopy.dstOffset = 0u;
        bufferCopy.srcOffset = 0u;
        bufferCopy.size = _descSizeInBytes(bufferRef);
      }

      // Finally copy from the staging buffer to the actual buffer
      vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1u, &bufferCopy);

      RenderSystem::flushTemporaryCommandBuffer();

      vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffer, nullptr);
      vkFreeMemory(RenderSystem::_vkDevice, stagingMemory, nullptr);
    }
  }
}
}
}
}
}
