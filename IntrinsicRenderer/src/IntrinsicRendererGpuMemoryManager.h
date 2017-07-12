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

#define _INTR_GPU_PAGE_SIZE_IN_BYTES (80u * 1024u * 1024u)

namespace Intrinsic
{
namespace Renderer
{
struct GpuMemoryPage
{
  Core::Memory::LinearOffsetAllocator _allocator;
  VkDeviceMemory _vkDeviceMemory;
  uint8_t* _mappedMemory;
  uint32_t _memoryTypeIdx;
};

struct GpuMemoryManager
{
  static void init();
  static void destroy();
  static void updateMemoryStats();

  // <-

  static GpuMemoryAllocationInfo
  allocateOffset(MemoryPoolType::Enum p_MemoryPoolType, uint32_t p_Size,
                 uint32_t p_Alignment, uint32_t p_MemoryTypeFlags);
  // <-

  _INTR_INLINE static void resetPool(MemoryPoolType::Enum p_MemoryPoolType)
  {
    for (uint32_t pageIdx = 0u; pageIdx < _memoryPools[p_MemoryPoolType].size();
         ++pageIdx)
    {
      _memoryPools[p_MemoryPoolType][pageIdx]._allocator.reset();
    }
  }

  // <-

  _INTR_INLINE static uint32_t
  calcAvailablePoolMemoryInBytes(MemoryPoolType::Enum p_MemoryPoolType)
  {
    uint32_t totalAvailableMemoryInBytes = 0u;
    for (uint32_t pageIdx = 0u; pageIdx < _memoryPools[p_MemoryPoolType].size();
         ++pageIdx)
    {
      totalAvailableMemoryInBytes +=
          _memoryPools[p_MemoryPoolType][pageIdx]
              ._allocator.calcAvailableMemoryInBytes();
    }
    return totalAvailableMemoryInBytes;
  }

  _INTR_INLINE static uint32_t
  calcPoolSizeInBytes(MemoryPoolType::Enum p_MemoryPoolType)
  {
    uint32_t totalSizeInBytes = 0u;
    for (uint32_t pageIdx = 0u; pageIdx < _memoryPools[p_MemoryPoolType].size();
         ++pageIdx)
    {
      totalSizeInBytes +=
          _memoryPools[p_MemoryPoolType][pageIdx]._allocator.size();
    }
    return totalSizeInBytes;
  }

private:
  static _INTR_ARRAY(GpuMemoryPage) _memoryPools[MemoryPoolType::kCount];

  static MemoryLocation::Enum
      _memoryPoolToMemoryLocation[MemoryPoolType::kCount];
  static const char* _memoryPoolNames[MemoryPoolType::kCount];
  static uint32_t _memoryLocationToMemoryPropertyFlags[MemoryLocation::kCount];
};
}
}
