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
void RenderPassManager::createResources(
    const RenderPassRefArray& p_RenderPasses)
{
  for (uint32_t i = 0u; i < p_RenderPasses.size(); ++i)
  {
    RenderPassRef rpRef = p_RenderPasses[i];
    VkRenderPass& renderPass = _vkRenderPass(rpRef);
    AttachmentArray& attachments = _descAttachments(rpRef);

    _INTR_ARRAY(VkAttachmentDescription) attachmentDescs;
    attachmentDescs.reserve(attachments.size());
    _INTR_ARRAY(VkAttachmentReference) colorRefs;
    _INTR_ARRAY(VkAttachmentReference) depthRefs;

    for (uint32_t i = 0u; i < (uint32_t)attachments.size(); ++i)
    {
      AttachmentDescription& attach = attachments[i];

      VkImageLayout imageLayout =
          !Helper::isFormatDepthStencilFormat((Format::Enum)attach.format)
              ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
              : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      // Create descs.
      VkAttachmentDescription attachmentDesc = {};
      {
        attachmentDesc.format =
            Helper::mapFormatToVkFormat((Format::Enum)attach.format);
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp =
            (attach.flags & AttachmentFlags::kClearOnLoad) > 0u
                ? VK_ATTACHMENT_LOAD_OP_CLEAR
                : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp =
            (attach.flags & AttachmentFlags::kClearStencilOnLoad) > 0u
                ? VK_ATTACHMENT_LOAD_OP_CLEAR
                : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = imageLayout;
        attachmentDesc.finalLayout = imageLayout;
        attachmentDesc.flags = 0u;
      }

      attachmentDescs.push_back(attachmentDesc);

      // Create refs.
      VkAttachmentReference attachmentRef = {};
      {
        attachmentRef.attachment = i;
        attachmentRef.layout = imageLayout;
      }

      if (imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
      {
        colorRefs.push_back(attachmentRef);
      }
      else if (imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
      {
        depthRefs.push_back(attachmentRef);
      }
      else
      {
        _INTR_ASSERT(false);
      }
    }
    _INTR_ASSERT(depthRefs.size() <= 1u);

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0u;
    subpass.inputAttachmentCount = 0u;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = (uint32_t)colorRefs.size();
    subpass.pColorAttachments = colorRefs.data();
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment =
        !depthRefs.empty() ? depthRefs.data() : nullptr;
    subpass.preserveAttachmentCount = 0u;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = (uint32_t)attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.data();
    renderPassInfo.subpassCount = 1u;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;
    renderPassInfo.flags = 0u;

    VkResult result = vkCreateRenderPass(RenderSystem::_vkDevice,
                                         &renderPassInfo, nullptr, &renderPass);
    _INTR_VK_CHECK_RESULT(result);
  }
}
}
}
}
}
