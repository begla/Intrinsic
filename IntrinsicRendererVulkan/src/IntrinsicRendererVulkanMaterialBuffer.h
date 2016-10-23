// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
struct MaterialBufferEntry
{
  // Water
  float refractionFactor;

  // Translucency
  float translucencyThicknessFactor;
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
    {
      void* stagingMemMapped;
      VkResult result =
          vkMapMemory(RenderSystem::_vkDevice, _materialStagingBufferMemory, 0u,
                      sizeof(MaterialBufferEntry), 0u, &stagingMemMapped);
      _INTR_VK_CHECK_RESULT(result);

      memcpy(stagingMemMapped, &p_MaterialBufferEntry,
             sizeof(MaterialBufferEntry));

      vkUnmapMemory(RenderSystem::_vkDevice, _materialStagingBufferMemory);
    }

    VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

    VkBufferCopy bufferCopy = {};
    {
      bufferCopy.dstOffset = p_Index * sizeof(MaterialBufferEntry);
      bufferCopy.srcOffset = 0u;
      bufferCopy.size = sizeof(MaterialBufferEntry);
    }

    vkCmdCopyBuffer(copyCmd, _materialStagingBuffer,
                    Resources::BufferManager::_vkBuffer(_materialBuffer), 1u,
                    &bufferCopy);

    RenderSystem::flushTemporaryCommandBuffer();
  }

  static Resources::BufferRef _materialBuffer;

private:
  static _INTR_ARRAY(uint32_t) _materialBufferEntries;
  static VkBuffer _materialStagingBuffer;
  static VkDeviceMemory _materialStagingBufferMemory;
};
}
}
}
