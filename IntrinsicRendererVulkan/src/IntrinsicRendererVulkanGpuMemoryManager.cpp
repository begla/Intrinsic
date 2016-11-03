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

// Precompiled header files
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
// Static members
Core::LinearOffsetAllocator
    GpuMemoryManager::_memoryAllocators[MemoryPoolType::kCount];

uint32_t GpuMemoryManager::_deviceLocalMemorySizeInBytes = 0u;
uint32_t GpuMemoryManager::_hostVisibleMemorySizeInBytes = 0u;
VkDeviceMemory GpuMemoryManager::_vkDeviceLocalMemory = VK_NULL_HANDLE;
VkDeviceMemory GpuMemoryManager::_vkHostVisibleMemory = VK_NULL_HANDLE;
uint8_t* GpuMemoryManager::_mappedHostVisibleMemory = nullptr;

namespace
{
uint32_t _memoryPoolSizesInBytes[MemoryPoolType::kCount] = {};
MemoryUsage::Enum _memoryPoolUsages[MemoryPoolType::kCount] = {};
const char* _memoryPoolNames[MemoryPoolType::kCount] = {};
}

void GpuMemoryManager::init()
{
  _INTR_LOG_INFO("Initializing GPU Memory Manager...");
  _INTR_LOG_PUSH();

  // Setup memory pools
  {
    _memoryPoolSizesInBytes[MemoryPoolType::kStaticImages] =
        512u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kStaticImages] = MemoryUsage::kOptimal;
    _memoryPoolNames[MemoryPoolType::kStaticImages] = "Static Images";

    _memoryPoolSizesInBytes[MemoryPoolType::kStaticBuffers] =
        64u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kStaticBuffers] = MemoryUsage::kOptimal;
    _memoryPoolNames[MemoryPoolType::kStaticBuffers] = "Static Buffers";

    _memoryPoolSizesInBytes[MemoryPoolType::kStaticStagingBuffers] =
        64u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kStaticStagingBuffers] =
        MemoryUsage::kStaging;
    _memoryPoolNames[MemoryPoolType::kStaticStagingBuffers] =
        "Static Staging Buffers";

    _memoryPoolSizesInBytes[MemoryPoolType::kResolutionDependentImages] =
        512u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kResolutionDependentImages] =
        MemoryUsage::kOptimal;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentImages] =
        "Resolution Dependent Images";

    _memoryPoolSizesInBytes[MemoryPoolType::kResolutionDependentBuffers] =
        64u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kResolutionDependentBuffers] =
        MemoryUsage::kOptimal;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentBuffers] =
        "Resolution Dependent Buffers";

    _memoryPoolSizesInBytes
        [MemoryPoolType::kResolutionDependentStagingBuffers] =
            64u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kResolutionDependentStagingBuffers] =
        MemoryUsage::kStaging;
    _memoryPoolNames[MemoryPoolType::kResolutionDependentStagingBuffers] =
        "Resolution Dependent Staging Buffers";

    _memoryPoolSizesInBytes[MemoryPoolType::kVolatileStagingBuffers] =
        64u * 1024u * 1024u;
    _memoryPoolUsages[MemoryPoolType::kVolatileStagingBuffers] =
        MemoryUsage::kStaging;
    _memoryPoolNames[MemoryPoolType::kVolatileStagingBuffers] =
        "Volatile Staging Buffers";
  }

  static const uint32_t deviceLocalMemoryPropertyFlags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  uint32_t deviceLocalMemoryTypeIdx = (uint32_t)-1;

  static const uint32_t hostVisiblePropertyFlags =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  uint32_t hostVisibleMemoryTypeIdx = (uint32_t)-1;

  for (uint32_t i = 0;
       i < RenderSystem::_vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
  {
    const VkMemoryType& memoryType =
        RenderSystem::_vkPhysicalDeviceMemoryProperties.memoryTypes[i];

    if (deviceLocalMemoryTypeIdx == (uint32_t)-1)
    {
      if ((memoryType.propertyFlags & deviceLocalMemoryPropertyFlags) ==
          deviceLocalMemoryPropertyFlags)
      {
        deviceLocalMemoryTypeIdx = i;
      }
    }
    if (hostVisibleMemoryTypeIdx == (uint32_t)-1)
    {
      if ((memoryType.propertyFlags & hostVisiblePropertyFlags) ==
          hostVisiblePropertyFlags)
      {
        hostVisibleMemoryTypeIdx = i;
      }
    }
  }
  _INTR_ASSERT(deviceLocalMemoryTypeIdx != (uint32_t)-1 &&
               hostVisibleMemoryTypeIdx != (uint32_t)-1 &&
               "Required memory type not available");

  // Allocate device local memory
  {
    _deviceLocalMemorySizeInBytes = 0u;

    for (uint32_t i = 0u; i < MemoryPoolType::kCount; ++i)
    {
      if (_memoryPoolUsages[i] == MemoryUsage::kOptimal)
      {
        _deviceLocalMemorySizeInBytes += _memoryPoolSizesInBytes[i];
      }
    }

    VkMemoryAllocateInfo memAllocInfo = {};
    {
      memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memAllocInfo.pNext = 0u;
      memAllocInfo.allocationSize = _deviceLocalMemorySizeInBytes;
      memAllocInfo.memoryTypeIndex = deviceLocalMemoryTypeIdx;
    }

    VkResult result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo,
                                       nullptr, &_vkDeviceLocalMemory);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_LOG_INFO("Allocated %.2f MB of device local memory...",
                   Math::bytesToMegaBytes(_deviceLocalMemorySizeInBytes));
  }

  // Allocate host visible memory
  {
    for (uint32_t i = 0u; i < MemoryPoolType::kCount; ++i)
    {
      if (_memoryPoolUsages[i] == MemoryUsage::kStaging)
      {
        _hostVisibleMemorySizeInBytes += _memoryPoolSizesInBytes[i];
      }
    }

    VkMemoryAllocateInfo memAllocInfo = {};
    {
      memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memAllocInfo.pNext = 0u;
      memAllocInfo.allocationSize = _hostVisibleMemorySizeInBytes;
      memAllocInfo.memoryTypeIndex = hostVisibleMemoryTypeIdx;
    }

    VkResult result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo,
                                       nullptr, &_vkHostVisibleMemory);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_LOG_INFO("Allocated %.2f MB of host visible memory...",
                   Math::bytesToMegaBytes(_hostVisibleMemorySizeInBytes));
  }

  // Setup allocators
  {
    uint32_t deviceLocalOffset = 0u;
    uint32_t hostVisibleOffset = 0u;

    for (uint32_t i = 0u; i < MemoryPoolType::kCount; ++i)
    {
      if (_memoryPoolUsages[i] == MemoryUsage::kOptimal)
      {
        _memoryAllocators[i].init(_memoryPoolSizesInBytes[i],
                                  deviceLocalOffset);
        deviceLocalOffset += _memoryPoolSizesInBytes[i];
      }
      else if (_memoryPoolUsages[i] == MemoryUsage::kStaging)
      {
        _memoryAllocators[i].init(_memoryPoolSizesInBytes[i],
                                  hostVisibleOffset);
        hostVisibleOffset += _memoryPoolSizesInBytes[i];
      }
      else
      {
        _INTR_ASSERT(false);
      }
    }
  }

  // Map host visible memory
  {
    VkResult result = vkMapMemory(RenderSystem::_vkDevice, _vkHostVisibleMemory,
                                  0u, _hostVisibleMemorySizeInBytes, 0u,
                                  (void**)&_mappedHostVisibleMemory);
    _INTR_VK_CHECK_RESULT(result);
  }

  _INTR_LOG_POP();
}

// <-

void GpuMemoryManager::updateMemoryStats()
{
#if defined(_INTR_PROFILING_ENABLED)
  static MicroProfileToken tokens[MemoryPoolType::kCount][2u];
  static bool init = false;

  if (!init)
  {
    for (uint32_t i = 0u; i < MemoryPoolType::kCount; ++i)
    {
      static char charBuffer[128];
      sprintf(charBuffer, "Total %s Memory (MB)", _memoryPoolNames[i]);
      tokens[i][0] = MicroProfileGetCounterToken(charBuffer);

      sprintf(charBuffer, "Available %s Memory (MB)", _memoryPoolNames[i]);
      tokens[i][1] = MicroProfileGetCounterToken(charBuffer);
    }

    init = true;
  }

  for (uint32_t i = 0u; i < MemoryPoolType::kCount; ++i)
  {
    MicroProfileCounterSet(tokens[i][0], (uint64_t)Math::bytesToMegaBytes(
                                             _memoryAllocators[i].size()));
    MicroProfileCounterSet(
        tokens[i][1], (uint64_t)Math::bytesToMegaBytes(
                          _memoryAllocators[i].calcAvailableMemoryInBytes()));
  }
#endif // _INTR_PROFILING_ENABLED
}

// <-

void GpuMemoryManager::destroy() {}
}
}
}
