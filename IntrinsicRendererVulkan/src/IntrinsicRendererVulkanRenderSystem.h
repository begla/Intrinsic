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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
struct RenderSystem
{
  static void init(void* p_PlatformHandle, void* p_PlatformWindow);

  // <-

  static void beginFrame();
  static void endFrame();

  // <-

  static void onViewportChanged();
  static void resizeSwapChain(bool p_Force = false);

  // <-

  _INTR_INLINE static VkCommandBuffer getPrimaryCommandBuffer()
  {
    return _vkCommandBuffers[_backbufferIndex];
  }

  // <-

  _INTR_INLINE static VkCommandBuffer*
  getSecondaryCommandBuffers(uint32_t p_CommandBufferIdx)
  {
    return &_vkSecondaryCommandBuffers
        [_backbufferIndex * _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT +
         p_CommandBufferIdx];
  }

  // <-

  _INTR_INLINE static uint32_t requestSecondaryCommandBuffers(uint32_t p_Count)
  {
    _INTR_ASSERT((_allocatedSecondaryCmdBufferCount + p_Count) <
                 _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT);
    uint32_t firstIdx = _allocatedSecondaryCmdBufferCount;
    _allocatedSecondaryCmdBufferCount += p_Count;
    return firstIdx;
  }

  // <-

  _INTR_INLINE static void beginPrimaryCommandBuffer()
  {
    VkCommandBufferBeginInfo cmdBufBeginInfo = {};
    {
      cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      cmdBufBeginInfo.pNext = nullptr;
      cmdBufBeginInfo.flags = 0u;
      cmdBufBeginInfo.pInheritanceInfo = nullptr;
    }

    VkResult result = vkBeginCommandBuffer(_vkCommandBuffers[_backbufferIndex],
                                           &cmdBufBeginInfo);
    _INTR_VK_CHECK_RESULT(result);
  }

  // <-

  _INTR_INLINE static void
  beginSecondaryCommandBuffer(uint32_t p_CmdBufferIdx,
                              VkRenderPass p_VkRenderPass,
                              VkFramebuffer p_VkFramebuffer)
  {
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    {
      inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
      inheritanceInfo.pNext = nullptr;
      inheritanceInfo.renderPass = p_VkRenderPass;
      inheritanceInfo.framebuffer = p_VkFramebuffer;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    {
      commandBufferBeginInfo.sType =
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      commandBufferBeginInfo.pNext = nullptr;
      commandBufferBeginInfo.flags =
          VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
      commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
    }

    VkResult result = vkBeginCommandBuffer(
        _vkSecondaryCommandBuffers[_backbufferIndex *
                                       _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT +
                                   p_CmdBufferIdx],
        &commandBufferBeginInfo);
    _INTR_VK_CHECK_RESULT(result);
  }

  _INTR_INLINE static void beginSecondaryCommandBuffer(uint32_t p_CmdBufferIdx)
  {
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    {
      commandBufferBeginInfo.sType =
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      commandBufferBeginInfo.pNext = nullptr;
      commandBufferBeginInfo.flags = 0u;
      commandBufferBeginInfo.pInheritanceInfo = nullptr;
    }

    VkResult result = vkBeginCommandBuffer(
        _vkSecondaryCommandBuffers[_backbufferIndex *
                                       _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT +
                                   p_CmdBufferIdx],
        &commandBufferBeginInfo);
    _INTR_VK_CHECK_RESULT(result);
  }

  // <-

  _INTR_INLINE static void endPrimaryCommandBuffer()
  {
    VkResult result = vkEndCommandBuffer(_vkCommandBuffers[_backbufferIndex]);
    _INTR_VK_CHECK_RESULT(result);
  }

  // <-

  _INTR_INLINE static void endSecondaryCommandBuffer(uint32_t p_CmdBufferIdx)
  {
    VkResult result = vkEndCommandBuffer(
        _vkSecondaryCommandBuffers[_backbufferIndex *
                                       _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT +
                                   p_CmdBufferIdx]);
    _INTR_VK_CHECK_RESULT(result);
  }

  // <-

  _INTR_INLINE static VkCommandBuffer beginTemporaryCommandBuffer()
  {
    VkCommandBufferBeginInfo cmdBufInfo = {};
    {
      cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      cmdBufInfo.pNext = nullptr;
      cmdBufInfo.flags = 0u;
      cmdBufInfo.pInheritanceInfo = nullptr;
    }
    VkResult result = vkBeginCommandBuffer(_vkTempCommandBuffer, &cmdBufInfo);
    _INTR_VK_CHECK_RESULT(result);

    return _vkTempCommandBuffer;
  }

  // <-

  _INTR_INLINE static void flushTemporaryCommandBuffer()
  {
    vkEndCommandBuffer(_vkTempCommandBuffer);

    VkSubmitInfo submitInfo = {};
    {
      submitInfo.pNext = nullptr;
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1u;
      submitInfo.pCommandBuffers = &_vkTempCommandBuffer;
    }

    VkResult result = vkQueueSubmit(RenderSystem::_vkQueue, 1, &submitInfo,
                                    _vkTempCommandBufferFence);
    _INTR_VK_CHECK_RESULT(result);

    result = vkWaitForFences(RenderSystem::_vkDevice, 1u,
                             &_vkTempCommandBufferFence, VK_TRUE, UINT64_MAX);
    _INTR_VK_CHECK_RESULT(result);

    result =
        vkResetFences(RenderSystem::_vkDevice, 1u, &_vkTempCommandBufferFence);
    _INTR_VK_CHECK_RESULT(result);
  }

  // <-

  static void dispatchComputeCall(Core::Dod::Ref p_ComputeCall,
                                  VkCommandBuffer p_CommandBuffer);
  static void dispatchDrawCall(Core::Dod::Ref p_DrawCall,
                               VkCommandBuffer p_CommandBuffer);

  // <-

  static void beginRenderPass(
      Core::Dod::Ref p_RenderPass, Core::Dod::Ref p_Framebuffer,
      VkSubpassContents p_SubpassContents = VK_SUBPASS_CONTENTS_INLINE,
      uint32_t p_ClearValueCount = 0u, VkClearValue* p_ClearValues = nullptr);
  static _INTR_INLINE void endRenderPass(Core::Dod::Ref p_RenderPass)
  {
    vkCmdEndRenderPass(getPrimaryCommandBuffer());
  }

  // <-

  _INTR_INLINE static void
  releaseResource(const Intrinsic::Core::Name& p_TypeName, void* p_UserData0,
                  void* p_UserData1)
  {
    ResourceReleaseEntry entry = {p_TypeName, p_UserData0, p_UserData1, 0u};
    _resourcesToFree.push_back(entry);
  }

  // <-

  _INTR_INLINE static glm::uvec2
  getAbsoluteRenderSize(RenderSize::Enum p_RenderSize)
  {
    switch (p_RenderSize)
    {
    case RenderSize::kFull:
      return _backbufferDimensions;
    case RenderSize::kHalf:
      return _backbufferDimensions / 2u;
    case RenderSize::kQuarter:
      return _backbufferDimensions / 4u;
    }

    return glm::uvec2(0u);
  }

  // <-

  _INTR_INLINE static bool waitForFrame(uint32_t p_Idx)
  {
    if ((_activeBackbufferMask & (1u << p_Idx)) > 0u)
    {
      VkResult result = VkResult::VK_TIMEOUT;

      do
      {
        result = vkWaitForFences(_vkDevice, 1u, &_vkDrawFences[p_Idx], VK_TRUE,
                                 UINT64_MAX);
      } while (result == VK_TIMEOUT);
      _INTR_VK_CHECK_RESULT(result);

      result = vkResetFences(_vkDevice, 1u, &_vkDrawFences[p_Idx]);
      _INTR_VK_CHECK_RESULT(result);

      _activeBackbufferMask &= ~(1u << p_Idx);
      return true;
    }

    return false;
  }

  // <-

  _INTR_INLINE static bool waitForAllFrames()
  {
    bool waited = false;
    for (uint32_t idx = 0u; idx < (uint32_t)_vkSwapchainImages.size(); ++idx)
    {
      waited = waited || waitForFrame(idx);
    }

    return waited;
  }

  // <-

  static VkInstance _vkInstance;

  static VkPhysicalDevice _vkPhysicalDevice;
  static VkPhysicalDeviceMemoryProperties _vkPhysicalDeviceMemoryProperties;

  static VkDevice _vkDevice;
  static VkPipelineCache _vkPipelineCache;

  static VkSurfaceKHR _vkSurface;
  static VkSwapchainKHR _vkSwapchain;
  static _INTR_ARRAY(VkImage) _vkSwapchainImages;
  static _INTR_ARRAY(VkImageView) _vkSwapchainImageViews;
  static glm::uvec2 _backbufferDimensions;
  static glm::uvec2 _customBackbufferDimensions;

  static VkQueue _vkQueue;

  static uint32_t _vkGraphicsAndComputeQueueFamilyIndex;

  // <-

  static uint32_t _backbufferIndex;
  static uint32_t _activeBackbufferMask;
  static Format::Enum _depthBufferFormat;

  // <-
  static Format::Enum _depthStencilFormatToUse;

private:
  static void initManagers();

  // <-

  static void initVkInstance();
  static void initVkDevice();
  static void initVkSurface(void* p_PlatformHandle, void* p_PlatformWindow);
  static void initOrUpdateVkSwapChain();
  static void initVkPipelineCache();
  static void initVkCommandPools();
  static void initVkCommandBuffers();
  static void initVkTempCommandBuffer();
  static void initVkSupportedDepthBufferFormat();
  static void destroyVkCommandBuffers();
  static void initVkSynchronization();
  static void setupPlatformDependentFormats();

  // <-

  _INTR_INLINE static void insertPrePresentBarrier()
  {
    VkCommandBuffer vkCmdBuffer = getPrimaryCommandBuffer();

    VkImageMemoryBarrier prePresentBarrier = {};
    {
      prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      prePresentBarrier.pNext = nullptr;
      prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      prePresentBarrier.subresourceRange.baseMipLevel = 0u;
      prePresentBarrier.subresourceRange.levelCount = 1u;
      prePresentBarrier.subresourceRange.baseArrayLayer = 0u;
      prePresentBarrier.subresourceRange.layerCount = 1u;
      prePresentBarrier.image = _vkSwapchainImages[_backbufferIndex];

      vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &prePresentBarrier);
    }
  }

  // <-

  _INTR_INLINE static void insertPostPresentBarrier()
  {
    VkCommandBuffer vkCmdBuffer = getPrimaryCommandBuffer();

    VkImageMemoryBarrier postPresentBarrier = {};
    {
      postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      postPresentBarrier.pNext = nullptr;
      postPresentBarrier.srcAccessMask = 0u;
      postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      postPresentBarrier.subresourceRange.aspectMask =
          VK_IMAGE_ASPECT_COLOR_BIT;
      postPresentBarrier.subresourceRange.baseMipLevel = 0u;
      postPresentBarrier.subresourceRange.levelCount = 1u;
      postPresentBarrier.subresourceRange.baseArrayLayer = 0u;
      postPresentBarrier.subresourceRange.layerCount = 1u;
      postPresentBarrier.image = _vkSwapchainImages[_backbufferIndex];

      vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
                           0, nullptr, 1, &postPresentBarrier);
    }
  }

  // <-

  static void releaseQueuedResources();
  static void reinitRendering();

  // <-

  static VkCommandPool _vkPrimaryCommandPool;
  static _INTR_ARRAY(VkCommandPool) _vkSecondaryCommandPools;

  static _INTR_ARRAY(VkCommandBuffer) _vkCommandBuffers;
  static _INTR_ARRAY(VkCommandBuffer) _vkSecondaryCommandBuffers;

  static VkCommandBuffer _vkTempCommandBuffer;
  static VkFence _vkTempCommandBufferFence;

  static VkSemaphore _vkImageAcquiredSemaphore;
  static _INTR_ARRAY(VkFence) _vkDrawFences;

  // <-

  static uint32_t _allocatedSecondaryCmdBufferCount;
  static _INTR_ARRAY(ResourceReleaseEntry) _resourcesToFree;
};
}
}
}
