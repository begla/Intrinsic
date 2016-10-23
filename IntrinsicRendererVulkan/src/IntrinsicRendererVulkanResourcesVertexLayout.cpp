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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
void VertexLayoutManager::createResources(
    const VertexLayoutRefArray& p_VertexLayoutes)
{
  for (uint32_t vtxLayoutIdx = 0u; vtxLayoutIdx < p_VertexLayoutes.size();
       ++vtxLayoutIdx)
  {
    VertexLayoutRef vertexLayoutRef = p_VertexLayoutes[vtxLayoutIdx];

    _INTR_ARRAY(VertexAttribute)& vertexAttributes =
        _descVertexAttributes(vertexLayoutRef);
    _INTR_ARRAY(VertexBinding)& vertexBindings =
        _descVertexBindings(vertexLayoutRef);

    _INTR_ARRAY(
        VkVertexInputAttributeDescription)& vkVertexInputAttributeDescs =
        _vkVertexInputAttributeDescs(vertexLayoutRef);
    _INTR_ARRAY(VkVertexInputBindingDescription)& vkVertexInputBindingDescs =
        _vkVertexInputBindingDescs(vertexLayoutRef);

    vkVertexInputBindingDescs.resize(vertexBindings.size());
    vkVertexInputAttributeDescs.resize(vertexAttributes.size());

    for (uint32_t vtxBindingIdx = 0u;
         vtxBindingIdx < (uint32_t)vertexBindings.size(); ++vtxBindingIdx)
    {
      VkVertexInputBindingDescription& vertexInputBindingDesc =
          vkVertexInputBindingDescs[vtxBindingIdx];
      const VertexBinding& vtxBinding = vertexBindings[vtxBindingIdx];

      {
        vertexInputBindingDesc.binding = vtxBinding.binding;
        vertexInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputBindingDesc.stride = vtxBinding.stride;
      }
    }

    for (uint32_t i = 0u; i < (uint32_t)vertexAttributes.size(); ++i)
    {
      VkVertexInputAttributeDescription& vertexAttributeDesc =
          vkVertexInputAttributeDescs[i];
      const VertexAttribute& vtxAttribute = vertexAttributes[i];

      {
        vertexAttributeDesc.binding = vtxAttribute.binding;
        vertexAttributeDesc.location = vtxAttribute.location;
        vertexAttributeDesc.format =
            Helper::mapFormatToVkFormat((Format::Enum)vtxAttribute.format);
        vertexAttributeDesc.offset = vtxAttribute.offset;
      }
    }

    VkPipelineVertexInputStateCreateInfo& createInfo =
        _vkPipelineVertexInputStateCreateInfo(vertexLayoutRef);
    {
      createInfo.sType =
          VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      createInfo.pNext = nullptr;
      createInfo.flags = 0u;
      createInfo.vertexBindingDescriptionCount =
          (uint32_t)vkVertexInputBindingDescs.size();
      createInfo.pVertexBindingDescriptions = vkVertexInputBindingDescs.data();
      createInfo.vertexAttributeDescriptionCount =
          (uint32_t)vkVertexInputAttributeDescs.size();
      createInfo.pVertexAttributeDescriptions =
          vkVertexInputAttributeDescs.data();
    }
  }
}
}
}
}
}
