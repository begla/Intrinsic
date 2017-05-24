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
namespace Resources
{
void VertexLayoutManager::createResources(
    const VertexLayoutRefArray& p_VertexLayouts)
{
  for (uint32_t vtxLayoutIdx = 0u; vtxLayoutIdx < p_VertexLayouts.size();
       ++vtxLayoutIdx)
  {
    VertexLayoutRef vertexLayoutRef = p_VertexLayouts[vtxLayoutIdx];

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
		VkFormat format = Helper::mapFormatToVkFormat((Format::Enum)vtxAttribute.format);
		if (format == VK_FORMAT_R16G16B16_SFLOAT)
		{
			format = VK_FORMAT_R16G16B16A16_SFLOAT;
		}
        vertexAttributeDesc.binding = vtxAttribute.binding;
        vertexAttributeDesc.location = vtxAttribute.location;
<<<<<<< HEAD
		vertexAttributeDesc.format = format;
          
=======
        vertexAttributeDesc.format =
            Helper::mapFormatToVkFormat((Format::Enum)vtxAttribute.format);

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(RenderSystem::_vkPhysicalDevice,
                                            vertexAttributeDesc.format, &props);

        if ((props.bufferFeatures && VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0u)
        {
          _INTR_ASSERT(
              false &&
              "Vertex buffer format is not supported on this platform");
        }

>>>>>>> c38c40efd79533577cbe3d578b7b645b2afe767b
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
