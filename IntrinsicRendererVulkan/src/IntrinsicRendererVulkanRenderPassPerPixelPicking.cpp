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

using namespace RVResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
namespace
{
Resources::ImageRef _pickingImageRef;
Resources::ImageRef _pickingDepthImageRef;
Resources::BufferRefArray _pickingReadBackBufferRefs;
Resources::FramebufferRef _framebufferRef;

Resources::RenderPassRef _renderPassRef;
}

// Static members
glm::uvec2 PerPixelPicking::_perPixelPickingSize = glm::uvec2(256u, 128u);

void PerPixelPicking::init()
{
  RenderPassRefArray renderpassesToCreate;

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(PerPixelPicking));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription pickingAttachment = {Format::kR32UInt,
                                               AttachmentFlags::kClearOnLoad};
    AttachmentDescription depthStencilAttachment = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse,
        AttachmentFlags::kClearOnLoad | AttachmentFlags::kClearStencilOnLoad};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(pickingAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(depthStencilAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderpassesToCreate);
}

// <-

void PerPixelPicking::onReinitRendering()
{
  FramebufferRefArray fbsToDestroy;
  FramebufferRefArray fbsToCreate;
  ImageRefArray imgsToCreate;
  ImageRefArray imgsToDestroy;
  ImageRefArray buffersToCreate;
  ImageRefArray buffersToDestroy;

  // Cleanup old resources
  {
    if (_pickingImageRef.isValid())
      imgsToDestroy.push_back(_pickingImageRef);
    if (_pickingDepthImageRef.isValid())
      imgsToDestroy.push_back(_pickingDepthImageRef);
    if (_framebufferRef.isValid())
      fbsToDestroy.push_back(_framebufferRef);
    if (!_pickingReadBackBufferRefs.empty())
    {
      buffersToDestroy.insert(buffersToDestroy.end(),
                              _pickingReadBackBufferRefs.begin(),
                              _pickingReadBackBufferRefs.end());
    }

    ImageManager::destroyImagesAndResources(imgsToDestroy);
    FramebufferManager::destroyFramebuffersAndResources(fbsToDestroy);
    BufferManager::destroyBuffersAndResources(buffersToDestroy);
  }

  // Create images
  _pickingImageRef = ImageManager::createImage(_N(PerPixelPicking));
  {
    ImageManager::resetToDefault(_pickingImageRef);
    ImageManager::addResourceFlags(
        _pickingImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_pickingImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_pickingImageRef) =
        glm::uvec3(_perPixelPickingSize, 1u);
    ImageManager::_descImageFormat(_pickingImageRef) = Format::kR32UInt;
    ImageManager::_descImageType(_pickingImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_pickingImageRef);

  _pickingDepthImageRef = ImageManager::createImage(_N(PerPixelPickingDepth));
  {
    ImageManager::resetToDefault(_pickingDepthImageRef);
    ImageManager::addResourceFlags(
        _pickingDepthImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_pickingDepthImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_pickingDepthImageRef) =
        glm::uvec3(_perPixelPickingSize, 1u);
    ImageManager::_descImageFormat(_pickingDepthImageRef) =
        RenderSystem::_depthStencilFormatToUse;
    ImageManager::_descImageType(_pickingDepthImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_pickingDepthImageRef);

  // Create buffers
  _pickingReadBackBufferRefs.clear();
  for (uint32_t i = 0u; i < RenderSystem::_vkSwapchainImages.size(); ++i)
  {
    BufferRef bufferRef =
        BufferManager::createBuffer(_N(PerPixelPickingReadBack));
    {
      BufferManager::resetToDefault(bufferRef);
      BufferManager::addResourceFlags(
          bufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);
      BufferManager::_descMemoryPoolType(bufferRef) =
          MemoryPoolType::kResolutionDependentStagingBuffers;

      BufferManager::_descBufferType(bufferRef) = BufferType::kStorage;
      BufferManager::_descSizeInBytes(bufferRef) =
          (uint32_t)_perPixelPickingSize.x * (uint32_t)_perPixelPickingSize.y *
          sizeof(uint32_t);
    }

    _pickingReadBackBufferRefs.push_back(bufferRef);
    buffersToCreate.push_back(bufferRef);
  }

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(PerPixelPicking));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_pickingImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_pickingDepthImageRef);

    FramebufferManager::_descDimensions(_framebufferRef) = _perPixelPickingSize;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  fbsToCreate.push_back(_framebufferRef);

  BufferManager::createResources(buffersToCreate);
  ImageManager::createResources(imgsToCreate);
  FramebufferManager::createResources(fbsToCreate);
}

// <-

void PerPixelPicking::destroy() {}

// <-

void PerPixelPicking::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  if (GameStates::Editing::_editingMode != GameStates::EditingMode::kSelection)
  {
    return;
  }

  _INTR_PROFILE_CPU("Render Pass", "Render Per Pixel Picking");
  _INTR_PROFILE_GPU("Render Per Pixel Picking");

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();

  RenderProcess::Default::getVisibleDrawCalls(
      p_CameraRef, 0u, MaterialManager::getMaterialPassId(_N(PerPixelPicking)))
      .copy(visibleDrawCalls);

  if (visibleDrawCalls.empty())
  {
    return;
  }

  // Update per mesh uniform data
  {
    CComponents::MeshManager::updateUniformData(visibleDrawCalls);
  }

  ImageManager::insertImageMemoryBarrier(
      _pickingImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  ImageManager::insertImageMemoryBarrier(
      _pickingDepthImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  VkClearValue clearValues[2] = {};
  {
    clearValues[0].color.uint32[0u] = (uint32_t)-1;
    clearValues[1].depthStencil.depth = 1.0f;
  }

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
                                2u, clearValues);
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                       _framebufferRef);
    _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (Per Pixel Picking)",
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);

  ImageManager::insertImageMemoryBarrier(
      _pickingImageRef, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

  // Read back previous picking result
  BufferRef readBackBufferToUse =
      _pickingReadBackBufferRefs[RenderSystem::_backbufferIndex];

  VkBufferImageCopy bufferImageCopy = {};
  {
    bufferImageCopy.bufferOffset = 0u;
    bufferImageCopy.imageOffset = {};
    bufferImageCopy.bufferRowLength = (uint32_t)_perPixelPickingSize.x;
    bufferImageCopy.bufferImageHeight = (uint32_t)_perPixelPickingSize.y;
    bufferImageCopy.imageExtent.width = (uint32_t)_perPixelPickingSize.x;
    bufferImageCopy.imageExtent.height = (uint32_t)_perPixelPickingSize.y;
    bufferImageCopy.imageExtent.depth = 1u;
    bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferImageCopy.imageSubresource.baseArrayLayer = 0u;
    bufferImageCopy.imageSubresource.layerCount = 1u;
    bufferImageCopy.imageSubresource.mipLevel = 0u;
  }

  vkCmdCopyImageToBuffer(RenderSystem::getPrimaryCommandBuffer(),
                         ImageManager::_vkImage(_pickingImageRef),
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         BufferManager::_vkBuffer(readBackBufferToUse), 1u,
                         &bufferImageCopy);
}

// <-

Components::NodeRef PerPixelPicking::pickNode(const glm::vec2& p_UV)
{
  BufferRef readBackBufferToUse =
      _pickingReadBackBufferRefs[RenderSystem::_backbufferIndex];

  uint32_t* bufferMem =
      (uint32_t*)Resources::BufferManager::getGpuMemory(readBackBufferToUse);

  const uint32_t x = (uint32_t)(p_UV.x * (_perPixelPickingSize.x - 1.0f) +
                                (0.5f / _perPixelPickingSize.x));
  const uint32_t y = (uint32_t)(p_UV.y * (_perPixelPickingSize.y - 1.0f) +
                                (0.5f / _perPixelPickingSize.y));

  const uint32_t bufferIndex = x + y * (uint32_t)_perPixelPickingSize.x;
  const uint32_t refId = bufferMem[bufferIndex];

  if (refId != (uint32_t)-1)
  {
    return Components::NodeRef(refId, 0u);
  }

  return Components::NodeRef();
}
}
}
}
}
