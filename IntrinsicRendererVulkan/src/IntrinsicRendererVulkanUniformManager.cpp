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
// Static members
Resources::BufferRef _perInstanceUniformBuffer;
Resources::BufferRef _perMaterialUniformBuffer;
uint8_t* UniformManager::_perInstanceMemory = nullptr;
Tlsf::Allocator _perMaterialAllocator;
LockFreeStack<UniformManager::UniformDataMemoryPage,
              _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT>
    UniformManager::_perInstanceMemoryPagesSmall
        [_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];
LockFreeStack<UniformManager::UniformDataMemoryPage,
              _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT>
    UniformManager::_perInstanceMemoryPagesLarge
        [_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT];
_INTR_ARRAY(UniformManager::UniformDataMemoryPage)
UniformManager::_perMaterialMemoryPages;

Resources::BufferRef UniformManager::_perInstanceUniformBuffer;
Resources::BufferRef UniformManager::_perMaterialUniformBuffer;
Resources::BufferRef UniformManager::_perMaterialStagingUniformBuffer;

// <-

void UniformManager::init()
{
  using namespace Resources;

  _INTR_LOG_INFO("Initializing Uniform Manager...");
  _INTR_LOG_PUSH();

  BufferRefArray buffersToCreate;

  _perInstanceUniformBuffer =
      BufferManager::createBuffer(_N(_PerInstanceConstantBuffer));
  {
    BufferManager::resetToDefault(_perInstanceUniformBuffer);
    BufferManager::addResourceFlags(
        _perInstanceUniformBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    BufferManager::_descBufferMemoryUsage(_perInstanceUniformBuffer) =
        MemoryUsage::kStaging;
    BufferManager::_descBufferType(_perInstanceUniformBuffer) =
        BufferType::kUniform;
    BufferManager::_descSizeInBytes(_perInstanceUniformBuffer) =
        _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES;
    buffersToCreate.push_back(_perInstanceUniformBuffer);
  }

  _perMaterialUniformBuffer =
      BufferManager::createBuffer(_N(_PerMaterialConstantBuffer));
  {
    BufferManager::resetToDefault(_perMaterialUniformBuffer);
    BufferManager::addResourceFlags(
        _perMaterialUniformBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    BufferManager::_descBufferType(_perMaterialUniformBuffer) =
        BufferType::kUniform;
    BufferManager::_descSizeInBytes(_perMaterialUniformBuffer) =
        _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES;
    buffersToCreate.push_back(_perMaterialUniformBuffer);
  }

  // Staging buffer used to update the per material buffer data
  _perMaterialStagingUniformBuffer =
      BufferManager::createBuffer(_N(_PerMaterialStagingConstantBuffer));
  {
    BufferManager::resetToDefault(_perMaterialStagingUniformBuffer);
    BufferManager::addResourceFlags(
        _perMaterialStagingUniformBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    BufferManager::_descBufferType(_perMaterialStagingUniformBuffer) =
        BufferType::kUniform;
    BufferManager::_descBufferMemoryUsage(_perMaterialStagingUniformBuffer) =
        MemoryUsage::kStaging;
    BufferManager::_descSizeInBytes(_perMaterialStagingUniformBuffer) =
        _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES;
    buffersToCreate.push_back(_perMaterialStagingUniformBuffer);
  }

  BufferManager::createResources(buffersToCreate);

  // Get host memory
  _perInstanceMemory = BufferManager::getGpuMemory(_perInstanceUniformBuffer);

  // Init. per instance data memory pages
  {
    uint32_t offset = 0u;
    for (uint32_t bufferIdx = 0u;
         bufferIdx < _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT; ++bufferIdx)
    {
      _perInstanceMemoryPagesSmall[bufferIdx].resize(
          _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT);
      _perInstanceMemoryPagesLarge[bufferIdx].resize(
          _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT);

      for (uint32_t pageIdx = 0u;
           pageIdx < _perInstanceMemoryPagesSmall[bufferIdx].size(); ++pageIdx)
      {
        _INTR_ASSERT(
            (offset + _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES) <=
                _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES &&
            "Instance uniform memory exhausted");

        _perInstanceMemoryPagesSmall[bufferIdx][pageIdx] = {
            _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES, offset};
        offset += _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES;
      }
      for (uint32_t pageIdx = 0u;
           pageIdx < _perInstanceMemoryPagesLarge[bufferIdx].size(); ++pageIdx)
      {
        _INTR_ASSERT(
            (offset + _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES) <=
                _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES &&
            "Instance uniform memory exhausted");

        _perInstanceMemoryPagesLarge[bufferIdx][pageIdx] = {
            _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES, offset};
        offset += _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES;
      }
    }
  }
  _INTR_LOG_INFO(
      "Allocated %.2f MB of per instance uniform memory...",
      Math::bytesToMegaBytes(_INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES));

  // ... and the per material ones
  {
    _perMaterialMemoryPages.resize(_INTR_VK_PER_MATERIAL_PAGE_COUNT);

    uint32_t offset = 0u;
    for (uint32_t pageIdx = 0u; pageIdx < _perMaterialMemoryPages.size();
         ++pageIdx)
    {
      _INTR_ASSERT((offset + _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES) <=
                       _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES &&
                   "Per material uniform memory exhausted");

      _perMaterialMemoryPages[pageIdx] = {
          _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES, offset};
      offset += _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES;
    }
    _INTR_LOG_INFO(
        "Allocated %.2f MB of per material uniform memory...",
        Math::bytesToMegaBytes(_INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES));

    _INTR_LOG_POP();
  }
}

// <-

void UniformManager::resetPerInstanceMemoryPages()
{
  const uint32_t bufferIdx =
      RenderSystem::_backbufferIndex % _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT;
  _perInstanceMemoryPagesSmall[bufferIdx].resize(
      _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT);
  _perInstanceMemoryPagesLarge[bufferIdx].resize(
      _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT);
}
}
}
}
