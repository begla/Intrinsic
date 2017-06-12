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

using namespace RVResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
struct UniformManager
{
  static void init();
  static void onFrameEnded();

  _INTR_INLINE static uint8_t* allocatePerInstanceDataMemory(uint32_t p_Size,
                                                             uint32_t& p_Offset)
  {
    const uint32_t bufferIdx = RV::RenderSystem::_backbufferIndex %
                               _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT;

    MemoryBlock block;

    if (p_Size < _INTR_VK_PER_INSTANCE_BLOCK_SMALL_SIZE_IN_BYTES)
    {
      block = _perInstanceAllocatorSmall[bufferIdx].allocate();
    }
    else if (p_Size < _INTR_VK_PER_INSTANCE_BLOCK_LARGE_SIZE_IN_BYTES)
    {
      block = _perInstanceAllocatorLarge[bufferIdx].allocate();
    }
    else
    {
      _INTR_ASSERT(false);
    }

    p_Offset = block.memoryOffset;
    return block.memory;
  }

  // <-

  _INTR_INLINE static uint32_t allocatePerMaterialDataMemory()
  {
    return _perMaterialAllocator.allocate().memoryOffset;
  }

  // <-

  _INTR_INLINE static void freePerMaterialDataMemory(uint32_t p_Offset)
  {
    _perMaterialAllocator.free({nullptr, p_Offset});
  }

  // <-

  _INTR_INLINE static void
  updatePerMaterialDataMemory(void* p_Data, uint32_t p_Size, uint32_t p_Offset)
  {
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

  // Static members
  static uint8_t* _perInstanceMemory;
  static uint8_t* _perFrameMemory;

  static BufferRef _perInstanceUniformBuffer;
  static BufferRef _perMaterialUniformBuffer;
  static BufferRef _perFrameUniformBuffer;

private:
  static LockFreeFixedBlockAllocator<
      _INTR_VK_PER_INSTANCE_BLOCK_SMALL_COUNT,
      _INTR_VK_PER_INSTANCE_BLOCK_SMALL_SIZE_IN_BYTES>
      _perInstanceAllocatorSmall[_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];
  static LockFreeFixedBlockAllocator<
      _INTR_VK_PER_INSTANCE_BLOCK_LARGE_COUNT,
      _INTR_VK_PER_INSTANCE_BLOCK_LARGE_SIZE_IN_BYTES>
      _perInstanceAllocatorLarge[_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];
  static LockFreeFixedBlockAllocator<_INTR_VK_PER_MATERIAL_BLOCK_COUNT,
                                     _INTR_VK_PER_MATERIAL_BLOCK_SIZE_IN_BYTES>
      _perMaterialAllocator;

  static BufferRef _perMaterialStagingUniformBuffer;
};
}
}
}
