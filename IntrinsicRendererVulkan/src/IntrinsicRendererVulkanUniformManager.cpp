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
Resources::BufferRef _perInstanceUniformBuffer;
Resources::BufferRef _perMaterialUniformBuffer;
uint8_t* UniformManager::_mappedPerInstanceMemory = nullptr;
Tlsf::Allocator _perMaterialAllocator;
LockFreeStack<UniformManager::UniformDataMemoryPage,
              _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT>
    UniformManager::_perInstanceMemoryPagesSmall
        [PER_INSTANCE_DATA_BUFFER_COUNT];
LockFreeStack<UniformManager::UniformDataMemoryPage,
              _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT>
    UniformManager::_perInstanceMemoryPagesLarge
        [PER_INSTANCE_DATA_BUFFER_COUNT];
_INTR_ARRAY(UniformManager::UniformDataMemoryPage)
UniformManager::_perMaterialMemoryPages;

Resources::BufferRef UniformManager::_perInstanceUniformBuffer;
Resources::BufferRef UniformManager::_perMaterialUniformBuffer;

VkBuffer UniformManager::_perMaterialUpdateStagingBuffer;
VkDeviceMemory UniformManager::_perMaterialUpdateStagingBufferMemory;

// <-

void UniformManager::init()
{
  Resources::BufferRefArray buffersToCreate;

  _perInstanceUniformBuffer =
      Resources::BufferManager::createBuffer(_N(_PerInstanceConstantBuffer));
  {
    Resources::BufferManager::resetToDefault(_perInstanceUniformBuffer);
    Resources::BufferManager::addResourceFlags(
        _perInstanceUniformBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    Resources::BufferManager::_descBufferMemoryUsage(
        _perInstanceUniformBuffer) = MemoryUsage::kHostVisibleAndCoherent;
    Resources::BufferManager::_descBufferType(_perInstanceUniformBuffer) =
        BufferType::kUniform;
    Resources::BufferManager::_descSizeInBytes(_perInstanceUniformBuffer) =
        _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES;
    buffersToCreate.push_back(_perInstanceUniformBuffer);
  }

  _perMaterialUniformBuffer =
      Resources::BufferManager::createBuffer(_N(_PerMaterialConstantBuffer));
  {
    Resources::BufferManager::resetToDefault(_perMaterialUniformBuffer);
    Resources::BufferManager::addResourceFlags(
        _perMaterialUniformBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    Resources::BufferManager::_descBufferType(_perMaterialUniformBuffer) =
        BufferType::kUniform;
    Resources::BufferManager::_descSizeInBytes(_perMaterialUniformBuffer) =
        _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES;
    buffersToCreate.push_back(_perMaterialUniformBuffer);
  }

  Resources::BufferManager::createResources(buffersToCreate);

  // Map host memory
  _mappedPerInstanceMemory =
      Resources::BufferManager::mapBuffer(_perInstanceUniformBuffer);

  // Init. per instance data memory pages
  {
    uint32_t offset = 0u;
    for (uint32_t bufferIdx = 0u; bufferIdx < PER_INSTANCE_DATA_BUFFER_COUNT;
         ++bufferIdx)
    {
      _perInstanceMemoryPagesSmall[bufferIdx].resize(
          _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT);
      _perInstanceMemoryPagesLarge[bufferIdx].resize(
          _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT);

      for (uint32_t pageIdx = 0u;
           pageIdx < _perInstanceMemoryPagesSmall[bufferIdx].size(); ++pageIdx)
      {
        _perInstanceMemoryPagesSmall[bufferIdx][pageIdx] = {
            _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES, offset};
        offset += _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES;
        _INTR_ASSERT(offset < _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES &&
                     "Instance uniform memory exhausted");
      }
      for (uint32_t pageIdx = 0u;
           pageIdx < _perInstanceMemoryPagesLarge[bufferIdx].size(); ++pageIdx)
      {
        _perInstanceMemoryPagesLarge[bufferIdx][pageIdx] = {
            _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES, offset};
        offset += _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES;
        _INTR_ASSERT(offset < _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES &&
                     "Instance uniform memory exhausted");
      }
    }
  }

  // ... and the per material ones
  {
    _perMaterialMemoryPages.resize(_INTR_VK_PER_MATERIAL_PAGE_COUNT);

    uint32_t offset = 0u;
    for (uint32_t pageIdx = 0u; pageIdx < _perMaterialMemoryPages.size();
         ++pageIdx)
    {
      _perMaterialMemoryPages[pageIdx] = {
          _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES, offset};
      offset += _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES;
      _INTR_ASSERT(offset < _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES);
    }
  }

  // Create staging buffer for updating material uniform data
  {
    VkBufferCreateInfo bufferCreateInfo = {};
    {
      bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferCreateInfo.pNext = nullptr;
      bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferCreateInfo.size = _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES;
      bufferCreateInfo.queueFamilyIndexCount = 0;
      bufferCreateInfo.pQueueFamilyIndices = nullptr;
      bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferCreateInfo.flags = 0u;
    }

    VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                     nullptr, &_perMaterialUpdateStagingBuffer);
    _INTR_VK_CHECK_RESULT(result);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(RenderSystem::_vkDevice,
                                  _perMaterialUpdateStagingBuffer, &memReqs);

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
                              nullptr, &_perMaterialUpdateStagingBufferMemory);
    _INTR_VK_CHECK_RESULT(result);

    result = vkBindBufferMemory(RenderSystem::_vkDevice,
                                _perMaterialUpdateStagingBuffer,
                                _perMaterialUpdateStagingBufferMemory, 0u);
    _INTR_VK_CHECK_RESULT(result);
  }
}

void UniformManager::beginFrame()
{
  const uint32_t bufferIdx = Renderer::Vulkan::RenderSystem::_backbufferIndex %
                             PER_INSTANCE_DATA_BUFFER_COUNT;
  _perInstanceMemoryPagesSmall[bufferIdx].resize(
      _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT);
  _perInstanceMemoryPagesLarge[bufferIdx].resize(
      _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT);
}

void UniformManager::endFrame() {}
}
}
}
