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
namespace MemoryPoolType
{
enum Enum
{
  kStaticImages,
  kStaticBuffers,
  kStaticStagingBuffers,

  kResolutionDependentImages,
  kResolutionDependentBuffers,
  kResolutionDependentStagingBuffers,

  kVolatileStagingBuffers,

  kCount,

  kRangeStartStatic = kStaticImages,
  kRangeEndStatic = kStaticStagingBuffers,
  kRangeStartResolutionDependent = kResolutionDependentImages,
  kRangeEndResolutionDependent = kResolutionDependentStagingBuffers,
  kRangeStartVolatile = kVolatileStagingBuffers,
  kRangeEndVolatile = kVolatileStagingBuffers
};
}

struct GpuMemoryManager
{
  static void init();
  static void destroy();
  static void updateMemoryStats();

  // <-

  _INTR_INLINE static uint8_t* getHostVisibleMemoryForOffset(uint32_t p_Offset)
  {
    return &_mappedHostVisibleMemory[p_Offset];
  }

  // <-

  _INTR_INLINE static uint32_t
  allocateOffset(MemoryPoolType::Enum p_MemoryPoolType, uint32_t p_Size,
                 uint32_t p_Alignment)
  {
    return _memoryAllocators[p_MemoryPoolType].allocate(p_Size, p_Alignment);
  }

  // <-

  _INTR_INLINE static void resetAllocator(MemoryPoolType::Enum p_MemoryPoolType)
  {
    _memoryAllocators[p_MemoryPoolType].reset();
  }

  // <-

  _INTR_INLINE static uint32_t
  calcAvailableMemoryInBytes(MemoryPoolType::Enum p_MemoryPoolType)
  {
    return _memoryAllocators[p_MemoryPoolType].calcAvailableMemoryInBytes();
  }

  // <-

  static Core::LinearOffsetAllocator _memoryAllocators[MemoryPoolType::kCount];

  static uint32_t _deviceLocalMemorySizeInBytes;
  static uint32_t _hostVisibleMemorySizeInBytes;
  static VkDeviceMemory _vkDeviceLocalMemory;
  static VkDeviceMemory _vkHostVisibleMemory;

  static uint8_t* _mappedHostVisibleMemory;
};
}
}
}
