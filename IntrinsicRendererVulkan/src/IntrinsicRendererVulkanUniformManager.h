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
struct UniformManager
{
  struct UniformDataMemoryPage
  {
    uint32_t pageSize;
    uint32_t memoryOffset;
  };

  static void init();
  static void resetPerInstanceMemoryPages();

  _INTR_INLINE static uint8_t* allocatePerInstanceDataMemory(uint32_t p_Size,
                                                             uint32_t& p_Offset)
  {
    const uint32_t bufferIdx =
        Renderer::Vulkan::RenderSystem::_backbufferIndex %
        _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT;
    UniformDataMemoryPage page;

    if (p_Size < _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES)
    {
      page = _perInstanceMemoryPagesSmall[bufferIdx].pop_back();
    }
    else if (p_Size < _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES)
    {
      page = _perInstanceMemoryPagesLarge[bufferIdx].pop_back();
    }
    else
    {
      _INTR_ASSERT(false);
    }

    p_Offset = page.memoryOffset;
    return &_perInstanceMemory[page.memoryOffset];
  }

  // <-

  _INTR_INLINE static void allocatePerMaterialDataMemory(uint32_t p_Size,
                                                         uint32_t& p_Offset)
  {
    UniformDataMemoryPage page = _perMaterialMemoryPages.back();
    _perMaterialMemoryPages.pop_back();

    p_Offset = page.memoryOffset;
  }

  // <-

  _INTR_INLINE static void
  updatePerMaterialDataMemory(void* p_Data, uint32_t p_Size, uint32_t p_Offset)
  {
    using namespace Resources;

    // Update staging memory
    {
      memcpy(BufferManager::getGpuMemory(_perMaterialStagingUniformBuffer),
             p_Data, p_Size);
    }

    // ... and copy to device
    VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

    VkBufferCopy bufferCopy = {};
    {
      bufferCopy.dstOffset = p_Offset;
      bufferCopy.srcOffset = 0u;
      bufferCopy.size = p_Size;
    }

    vkCmdCopyBuffer(
        copyCmd, BufferManager::_vkBuffer(_perMaterialStagingUniformBuffer),
        BufferManager::_vkBuffer(_perMaterialUniformBuffer), 1u, &bufferCopy);

    RenderSystem::flushTemporaryCommandBuffer();
  }

  // <-

  _INTR_INLINE static void freePerMaterialDataMemory(uint32_t p_Offset)
  {
    UniformDataMemoryPage page = {_INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES,
                                  p_Offset};
    _perMaterialMemoryPages.push_back(page);
  }

  // <-

  // Static members
  static uint8_t* _perInstanceMemory;

  static Resources::BufferRef _perInstanceUniformBuffer;
  static Resources::BufferRef _perMaterialUniformBuffer;

private:
  static LockFreeStack<UniformDataMemoryPage,
                       _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT>
      _perInstanceMemoryPagesSmall[_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];
  static LockFreeStack<UniformDataMemoryPage,
                       _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT>
      _perInstanceMemoryPagesLarge[_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];

  static Resources::BufferRef _perMaterialStagingUniformBuffer;
  static _INTR_ARRAY(UniformManager::UniformDataMemoryPage)
      _perMaterialMemoryPages;
};
}
}
}
