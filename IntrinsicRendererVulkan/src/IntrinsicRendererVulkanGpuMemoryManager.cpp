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

#define STATIC_TEXTURE_MEMORY_SIZE_IN_BYTES 1024u * 1024u * 1024u
#define STATIC_BUFFER_MEMORY_SIZE_IN_BYTES 512u * 1024 * 1024u
#define STATIC_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES 256u * 1024u * 1024u
#define VOLATILE_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES 128u * 1024u * 1024u

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
// Static members
Core::LinearOffsetAllocator GpuMemoryManager::_staticImageMemoryAllocator;
Core::LinearOffsetAllocator GpuMemoryManager::_staticBufferMemoryAllocator;
Core::LinearOffsetAllocator
    GpuMemoryManager::_staticStagingBufferMemoryAllocator;
Core::LinearOffsetAllocator
    GpuMemoryManager::_volatileStagingBufferMemoryAllocator;

uint32_t GpuMemoryManager::_deviceLocalMemorySizeInBytes = 0u;
uint32_t GpuMemoryManager::_hostVisibleMemorySizeInBytes = 0u;
VkDeviceMemory GpuMemoryManager::_vkDeviceLocalMemory = VK_NULL_HANDLE;
VkDeviceMemory GpuMemoryManager::_vkHostVisibleMemory = VK_NULL_HANDLE;
uint8_t* GpuMemoryManager::_mappedHostVisibleMemory = nullptr;

void GpuMemoryManager::init()
{
  _INTR_LOG_INFO("Initializing GPU Memory Manager...");
  _INTR_LOG_PUSH();

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
    _deviceLocalMemorySizeInBytes = STATIC_TEXTURE_MEMORY_SIZE_IN_BYTES +
                                    STATIC_BUFFER_MEMORY_SIZE_IN_BYTES;

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
    _hostVisibleMemorySizeInBytes =
        STATIC_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES +
        VOLATILE_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES;

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
    _staticImageMemoryAllocator.init(STATIC_TEXTURE_MEMORY_SIZE_IN_BYTES,
                                     deviceLocalOffset);
    deviceLocalOffset += STATIC_TEXTURE_MEMORY_SIZE_IN_BYTES;
    _staticBufferMemoryAllocator.init(STATIC_BUFFER_MEMORY_SIZE_IN_BYTES,
                                      deviceLocalOffset);
    deviceLocalOffset += STATIC_BUFFER_MEMORY_SIZE_IN_BYTES;

    uint32_t hostVisibleOffset = 0u;
    _staticStagingBufferMemoryAllocator.init(
        STATIC_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES, hostVisibleOffset);
    hostVisibleOffset += STATIC_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES;
    _volatileStagingBufferMemoryAllocator.init(
        VOLATILE_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES, hostVisibleOffset);
    hostVisibleOffset += VOLATILE_STAGING_BUFFER_MEMORY_SIZE_IN_BYTES;
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
  _INTR_PROFILE_COUNTER_SET(
      "Total Static Image Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(_staticImageMemoryAllocator.size()));
  _INTR_PROFILE_COUNTER_SET(
      "Available Static Image Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(
          _staticImageMemoryAllocator.calcAvailableMemoryInBytes()));

  _INTR_PROFILE_COUNTER_SET(
      "Total Static Buffer Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(_staticBufferMemoryAllocator.size()));
  _INTR_PROFILE_COUNTER_SET(
      "Available Static Buffer Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(
          _staticBufferMemoryAllocator.calcAvailableMemoryInBytes()));

  _INTR_PROFILE_COUNTER_SET("Total Static Staging Buffer Memory (MB)",
                            (uint64_t)Math::bytesToMegaBytes(
                                _staticStagingBufferMemoryAllocator.size()));
  _INTR_PROFILE_COUNTER_SET(
      "Available Static Staging Buffer Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(
          _staticStagingBufferMemoryAllocator.calcAvailableMemoryInBytes()));

  _INTR_PROFILE_COUNTER_SET("Total Volatile Staging Buffer Memory (MB)",
                            (uint64_t)Math::bytesToMegaBytes(
                                _volatileStagingBufferMemoryAllocator.size()));
  _INTR_PROFILE_COUNTER_SET(
      "Available Volatile Staging Buffer Memory (MB)",
      (uint64_t)Math::bytesToMegaBytes(
          _volatileStagingBufferMemoryAllocator.calcAvailableMemoryInBytes()));
}

// <-

void GpuMemoryManager::destroy() {}
}
}
}
