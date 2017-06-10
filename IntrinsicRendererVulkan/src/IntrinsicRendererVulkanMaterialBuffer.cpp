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
Resources::BufferRef MaterialBuffer::_materialBuffer;
Resources::BufferRef MaterialBuffer::_materialStagingBuffer;

_INTR_ARRAY(uint32_t) MaterialBuffer::_materialBufferEntries;

void MaterialBuffer::init()
{
  BufferRefArray buffersToCreate;

  _materialBuffer = BufferManager::createBuffer(_N(MaterialBuffer));
  {
    BufferManager::resetToDefault(_materialBuffer);
    BufferManager::addResourceFlags(
        _materialBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

    BufferManager::_descBufferType(_materialBuffer) = BufferType::kStorage;
    BufferManager::_descSizeInBytes(_materialBuffer) =
        sizeof(MaterialBufferEntry) * _INTR_MAX_MATERIAL_COUNT;
    buffersToCreate.push_back(_materialBuffer);
  }

  _materialStagingBuffer =
      BufferManager::createBuffer(_N(_MaterialStagingBuffer));
  {
    BufferManager::resetToDefault(_materialStagingBuffer);
    BufferManager::addResourceFlags(
        _materialStagingBuffer,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    BufferManager::_descBufferType(_materialStagingBuffer) =
        BufferType::kStorage;
    BufferManager::_descMemoryPoolType(_materialStagingBuffer) =
        MemoryPoolType::kStaticStagingBuffers;
    BufferManager::_descSizeInBytes(_materialStagingBuffer) =
        sizeof(MaterialBufferEntry);
    buffersToCreate.push_back(_materialStagingBuffer);
  }

  BufferManager::createResources(buffersToCreate);

  _materialBufferEntries.clear();
  for (uint32_t i = 0u; i < _INTR_MAX_MATERIAL_COUNT; ++i)
  {
    _materialBufferEntries.push_back(_INTR_MAX_MATERIAL_COUNT - 1u - i);
  }
}
}
}
}
