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
struct MaterialBufferEntry
{
  float refractionFactor;
  float translucencyThicknessFactor;
  float emissiveIntensity;
};

struct MaterialBuffer
{
  static void init();

  _INTR_INLINE static uint32_t allocateMaterialBufferEntry()
  {
    const uint32_t idx = _materialBufferEntries.back();
    _materialBufferEntries.pop_back();

    return idx;
  }

  _INTR_INLINE static void freeMaterialBufferEntry(uint32_t p_Index)
  {
    _materialBufferEntries.push_back(p_Index);
  }

  _INTR_INLINE static void
  updateMaterialBufferEntry(const uint32_t p_Index,
                            const MaterialBufferEntry& p_MaterialBufferEntry)
  {
    // Update staging buffer
    {
      memcpy(BufferManager::getGpuMemory(_materialStagingBuffer),
             &p_MaterialBufferEntry, sizeof(MaterialBufferEntry));
    }

    // ... and copy to device
    VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

    VkBufferCopy bufferCopy = {};
    {
      bufferCopy.dstOffset = p_Index * sizeof(MaterialBufferEntry);
      bufferCopy.srcOffset = 0u;
      bufferCopy.size = sizeof(MaterialBufferEntry);
    }

    vkCmdCopyBuffer(copyCmd, BufferManager::_vkBuffer(_materialStagingBuffer),
                    BufferManager::_vkBuffer(_materialBuffer), 1u, &bufferCopy);

    RenderSystem::flushTemporaryCommandBuffer();
  }

  static BufferRef _materialBuffer;

private:
  static _INTR_ARRAY(uint32_t) _materialBufferEntries;
  static BufferRef _materialStagingBuffer;
};
}
}
}
