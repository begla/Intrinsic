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
void FramebufferManager::createResources(
    const FramebufferRefArray& p_FrameBuffers)
{
  for (uint32_t fbIdx = 0u; fbIdx < p_FrameBuffers.size(); ++fbIdx)
  {
    FramebufferRef frameBufferRef = p_FrameBuffers[fbIdx];

    AttachmentInfoArray& attachedImgs = _descAttachedImages(frameBufferRef);
    _INTR_ASSERT(!attachedImgs.empty());

    // Collect image views from images
    _INTR_ARRAY(VkImageView) attachments;
    {
      attachments.resize(attachedImgs.size());
      for (uint32_t attIdx = 0u; attIdx < attachments.size(); ++attIdx)
      {
        AttachmentInfo attachmentInfo = attachedImgs[attIdx];
        attachments[attIdx] = Resources::ImageManager::_vkSubResourceImageView(
            attachmentInfo.imageRef, attachmentInfo.arrayLayerIdx, 0u);
      }
    }

    VkFramebufferCreateInfo fbCreateInfo = {};
    {
      fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      fbCreateInfo.pNext = nullptr;

      Resources::RenderPassRef renderPassRef = _descRenderPass(frameBufferRef);
      fbCreateInfo.renderPass =
          Resources::RenderPassManager::_vkRenderPass(renderPassRef);

      fbCreateInfo.attachmentCount = (uint32_t)attachments.size();
      fbCreateInfo.pAttachments = attachments.data();

      const glm::uvec2& dimensions = _descDimensions(frameBufferRef);
      fbCreateInfo.width = (uint32_t)dimensions.x;
      fbCreateInfo.height = (uint32_t)dimensions.y;
      fbCreateInfo.layers = 1u;
    }

    VkFramebuffer& vkFrameBuffer = _vkFrameBuffer(frameBufferRef);
    VkResult result = vkCreateFramebuffer(
        RenderSystem::_vkDevice, &fbCreateInfo, nullptr, &vkFrameBuffer);
  }
}
}
}
}
}
