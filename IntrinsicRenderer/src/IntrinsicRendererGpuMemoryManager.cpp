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

// Precompiled header files
#include "stdafx_renderer.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
// Static members
_INTR_ARRAY(GpuMemoryPage)
GpuMemoryManager::_memoryPools[MemoryPoolType::kCount];

MemoryLocation::Enum
    GpuMemoryManager::_memoryPoolToMemoryLocation[MemoryPoolType::kCount] = {};
uint32_t GpuMemoryManager::_memoryLocationToMemoryPropertyFlags
    [MemoryLocation::kCount] = {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
const char* GpuMemoryManager::_memoryPoolNames[MemoryPoolType::kCount] = {};

namespace
{
}

void GpuMemoryManager::init()
{
  _INTR_LOG_INFO("Initializing GPU Memory Manager...");

  // Setup memory pools
  {
    _memoryPoolToMemoryLocation[MemoryPoolType::kStaticImages] =
        MemoryLocation::kDeviceLocal;
    _memoryPoolNames[MemoryPoolType::kStaticImages] = "Static Images";

    _memoryPoolToMemoryLocation[MemoryPoolType::kStaticBuffers] =
        MemoryLocation::kDeviceLocal;
    _memoryPoolNames[MemoryPoolType::kStaticBuffers] = "Static Buffers";

    _memoryPoolToMemoryLocation[MemoryPoolType::kStaticStagingBuffers] =
        MemoryLocation::kHostVisible;
    _memoryPoolNames[MemoryPoolType::kStaticStagingBuffers] =
        "Static Staging Buffers";

    _memoryPoolToMemoryLocation[MemoryPoolType::kResolutionDependentImages] =
        MemoryLocation::kDeviceLocal;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentImages] =
        "Resolution Dependent Images";

    _memoryPoolToMemoryLocation[MemoryPoolType::kResolutionDependentBuffers] =
        MemoryLocation::kDeviceLocal;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentBuffers] =
        "Resolution Dependent Buffers";

    _memoryPoolToMemoryLocation
        [MemoryPoolType::kResolutionDependentStagingBuffers] =
            MemoryLocation::kHostVisible;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentStagingBuffers] =
        "Resolution Dependent Staging Buffers";

    _memoryPoolToMemoryLocation[MemoryPoolType::kVolatileStagingBuffers] =
        MemoryLocation::kHostVisible;
    _memoryPoolNames[MemoryPoolType::kVolatileStagingBuffers] =
        "Volatile Staging Buffers";
  }
}

// <-

GpuMemoryAllocationInfo
GpuMemoryManager::allocateOffset(MemoryPoolType::Enum p_MemoryPoolType,
                                 uint32_t p_Size, uint32_t p_Alignment,
                                 uint32_t p_MemoryTypeFlags)
{
  _INTR_ARRAY(GpuMemoryPage)& poolPages = _memoryPools[p_MemoryPoolType];

  // Try to find a fitting page
  for (uint32_t pageIdx = 0u; pageIdx < _memoryPools[p_MemoryPoolType].size();
       ++pageIdx)
  {
    GpuMemoryPage& page = poolPages[pageIdx];

    if ((p_MemoryTypeFlags & (1u << page._memoryTypeIdx)) > 0u &&
        page._allocator.fits(p_Size, p_Alignment))
    {
      const uint32_t offset = page._allocator.allocate(p_Size, p_Alignment);
      return {p_MemoryPoolType,
              pageIdx,
              offset,
              page._vkDeviceMemory,
              p_Size,
              p_Alignment,
              page._mappedMemory != nullptr ? &page._mappedMemory[offset]
                                            : nullptr};
    }
  }

  // No existing page found, allocate a new one
  for (uint32_t memoryTypeIdx = 0;
       memoryTypeIdx <
       RenderSystem::_vkPhysicalDeviceMemoryProperties.memoryTypeCount;
       ++memoryTypeIdx)
  {
    const VkMemoryType& memoryType =
        RenderSystem::_vkPhysicalDeviceMemoryProperties
            .memoryTypes[memoryTypeIdx];

    const MemoryLocation::Enum memLocation =
        _memoryPoolToMemoryLocation[p_MemoryPoolType];
    const uint32_t memoryPropertyFlags =
        _memoryLocationToMemoryPropertyFlags[memLocation];

    if ((memoryType.propertyFlags & memoryPropertyFlags) ==
            memoryPropertyFlags &&
        (p_MemoryTypeFlags & (1u << memoryTypeIdx)) > 0u)
    {
      poolPages.resize(poolPages.size() + 1u);
      GpuMemoryPage& page = poolPages.back();
      {
        page._allocator.init(_INTR_GPU_PAGE_SIZE_IN_BYTES);
        page._memoryTypeIdx = memoryTypeIdx;

        VkMemoryAllocateInfo memAllocInfo = {};
        {
          memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          memAllocInfo.pNext = 0u;
          memAllocInfo.allocationSize = _INTR_GPU_PAGE_SIZE_IN_BYTES;
          memAllocInfo.memoryTypeIndex = memoryTypeIdx;
        }

        VkResult result =
            vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                             &page._vkDeviceMemory);
        _INTR_VK_CHECK_RESULT(result);
      }

      // Map device local memory
      if (memLocation == MemoryLocation::kHostVisible)
      {
        VkResult result = vkMapMemory(
            RenderSystem::_vkDevice, page._vkDeviceMemory, 0u,
            _INTR_GPU_PAGE_SIZE_IN_BYTES, 0u, (void**)&page._mappedMemory);
        _INTR_VK_CHECK_RESULT(result);
      }

      _INTR_ASSERT(page._allocator.fits(p_Size, p_Alignment) &&
                   "Allocation does not fit in a single page");

      const uint32_t offset = page._allocator.allocate(p_Size, p_Alignment);
      return {p_MemoryPoolType,
              (uint32_t)poolPages.size() - 1u,
              offset,
              page._vkDeviceMemory,
              p_Size,
              p_Alignment,
              page._mappedMemory != nullptr ? &page._mappedMemory[offset]
                                            : nullptr};
    }
  }

  _INTR_ASSERT(false && "Failed to allocate a new GPU memory page");
  return {};
}

// <-

void GpuMemoryManager::updateMemoryStats()
{
#if defined(_INTR_PROFILING_ENABLED)
  static MicroProfileToken tokens[MemoryPoolType::kCount][2u];
  static bool init = false;

  if (!init)
  {
    for (uint32_t memoryPoolType = 0u; memoryPoolType < MemoryPoolType::kCount;
         ++memoryPoolType)
    {
      static char charBuffer[128];
      sprintf(charBuffer, "Total %s Memory (MB)",
              _memoryPoolNames[memoryPoolType]);
      tokens[memoryPoolType][0] = MicroProfileGetCounterToken(charBuffer);

      sprintf(charBuffer, "Available %s Memory (MB)",
              _memoryPoolNames[memoryPoolType]);
      tokens[memoryPoolType][1] = MicroProfileGetCounterToken(charBuffer);
    }

    init = true;
  }

  for (uint32_t memoryPoolType = 0u; memoryPoolType < MemoryPoolType::kCount;
       ++memoryPoolType)
  {
    MicroProfileCounterSet(tokens[memoryPoolType][0],
                           (uint64_t)Math::bytesToMegaBytes(calcPoolSizeInBytes(
                               (MemoryPoolType::Enum)memoryPoolType)));
    MicroProfileCounterSet(
        tokens[memoryPoolType][1],
        (uint64_t)Math::bytesToMegaBytes(calcAvailablePoolMemoryInBytes(
            (MemoryPoolType::Enum)memoryPoolType)));
  }
#endif // _INTR_PROFILING_ENABLED
}

// <-

void GpuMemoryManager::destroy() {}
}
}
