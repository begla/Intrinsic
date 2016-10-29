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
struct GpuMemoryManager
{
  static void init();
  static void destroy();
  static void updateMemoryStats();

  _INTR_INLINE static uint8_t* getHostVisibleMemoryForOffset(uint32_t p_Offset)
  {
    return &_mappedHostVisibleMemory[p_Offset];
  }

  static Core::LinearOffsetAllocator _staticImageMemoryAllocator;
  static Core::LinearOffsetAllocator _staticBufferMemoryAllocator;
  static Core::LinearOffsetAllocator _staticStagingBufferMemoryAllocator;
  static Core::LinearOffsetAllocator _volatileStagingBufferMemoryAllocator;

  static uint32_t _deviceLocalMemorySizeInBytes;
  static uint32_t _hostVisibleMemorySizeInBytes;
  static VkDeviceMemory _vkDeviceLocalMemory;
  static VkDeviceMemory _vkHostVisibleMemory;

  static uint8_t* _mappedHostVisibleMemory;
};
}
}
}
