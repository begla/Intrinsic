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
