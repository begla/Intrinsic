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
namespace RenderPass
{
namespace
{
Resources::ImageRef _albedoImageRef;
Resources::ImageRef _normalImageRef;
Resources::ImageRef _parameter0ImageRef;
Resources::ImageRef _depthImageRef;
Resources::FramebufferRef _framebufferRef;

Resources::RenderPassRef _renderPassRef;
}

void GBuffer::init()
{
  using namespace Resources;

  RenderPassRefArray renderpassesToCreate;

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(GBuffer));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription albedoAttachment = {Format::kR16G16B16A16Float,
                                              AttachmentFlags::kClearOnLoad};
    AttachmentDescription normalAttachment = {Format::kR16G16B16A16Float,
                                              AttachmentFlags::kClearOnLoad};
    AttachmentDescription parameter0Attachment = {
        Format::kR16G16B16A16Float, AttachmentFlags::kClearOnLoad};
    AttachmentDescription depthStencilAttachment = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse,
        AttachmentFlags::kClearOnLoad | AttachmentFlags::kClearStencilOnLoad};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(albedoAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(normalAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(parameter0Attachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(depthStencilAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderpassesToCreate);
}

// <-

void GBuffer::onReinitRendering()
{
  using namespace Resources;

  FramebufferRefArray fbsToDestroy;
  FramebufferRefArray imgsToDestroy;
  FramebufferRefArray fbsToCreate;
  FramebufferRefArray imgsToCreate;

  // Cleanup old resources
  {
    if (_framebufferRef.isValid())
      fbsToDestroy.push_back(_framebufferRef);
    if (_albedoImageRef.isValid())
      imgsToDestroy.push_back(_albedoImageRef);
    if (_normalImageRef.isValid())
      imgsToDestroy.push_back(_normalImageRef);
    if (_parameter0ImageRef.isValid())
      imgsToDestroy.push_back(_parameter0ImageRef);
    if (_depthImageRef.isValid())
      imgsToDestroy.push_back(_depthImageRef);

    ImageManager::destroyImagesAndResources(imgsToDestroy);
    FramebufferManager::destroyFramebuffersAndResources(fbsToDestroy);
  }

  glm::uvec3 dim = glm::vec3(RenderSystem::_backbufferDimensions.x,
                             RenderSystem::_backbufferDimensions.y, 1u);

  // Create images
  _albedoImageRef = ImageManager::createImage(_N(GBufferAlbedo));
  {
    ImageManager::resetToDefault(_albedoImageRef);
    ImageManager::addResourceFlags(
        _albedoImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_albedoImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_albedoImageRef) = dim;
    ImageManager::_descImageFormat(_albedoImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_albedoImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_albedoImageRef);

  _normalImageRef = ImageManager::createImage(_N(GBufferNormal));
  {
    ImageManager::resetToDefault(_normalImageRef);
    ImageManager::addResourceFlags(
        _normalImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_normalImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_normalImageRef) = dim;
    ImageManager::_descImageFormat(_normalImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_normalImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_normalImageRef);

  _parameter0ImageRef = ImageManager::createImage(_N(GBufferParameter0));
  {
    ImageManager::resetToDefault(_parameter0ImageRef);
    ImageManager::addResourceFlags(
        _parameter0ImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_parameter0ImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_parameter0ImageRef) = dim;
    ImageManager::_descImageFormat(_parameter0ImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_parameter0ImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_parameter0ImageRef);

  _depthImageRef = ImageManager::createImage(_N(GBufferDepth));
  {
    ImageManager::resetToDefault(_depthImageRef);
    ImageManager::addResourceFlags(
        _depthImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_depthImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_depthImageRef) = dim;
    ImageManager::_descImageFormat(_depthImageRef) =
        RenderSystem::_depthStencilFormatToUse;
    ImageManager::_descImageType(_depthImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_depthImageRef);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(GBuffer));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_albedoImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_normalImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_parameter0ImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_depthImageRef);
    FramebufferManager::_descDimensions(_framebufferRef) = glm::vec2(dim);
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  fbsToCreate.push_back(_framebufferRef);

  ImageManager::createResources(imgsToCreate);
  FramebufferManager::createResources(fbsToCreate);
}

// <-

void GBuffer::destroy() {}

// <-

void GBuffer::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render GBuffer");
  _INTR_PROFILE_GPU("Render GBuffer");

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();
  RenderProcess::Default::_visibleDrawCallsPerMaterialPass
      [0u][MaterialManager::getMaterialPassId(_N(GBuffer))]
          .copy(visibleDrawCalls);
  RenderProcess::Default::_visibleDrawCallsPerMaterialPass
      [0u][MaterialManager::getMaterialPassId(_N(GBufferLayered))]
          .copy(visibleDrawCalls);

  DrawCallManager::sortDrawCallsFrontToBack(visibleDrawCalls);

  // Update per mesh uniform data
  {
    Core::Components::MeshManager::updateUniformData(visibleDrawCalls);
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ImageManager::insertImageMemoryBarrier(
      _albedoImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  ImageManager::insertImageMemoryBarrier(
      _normalImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  ImageManager::insertImageMemoryBarrier(
      _parameter0ImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  ImageManager::insertImageMemoryBarrier(
      _depthImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  VkClearValue clearValues[4] = {};
  {
    clearValues[0].color.float32[0u] = 0.45f;
    clearValues[0].color.float32[1u] = 0.93f;
    clearValues[0].color.float32[2u] = 1.0f;
    clearValues[0].color.float32[3u] = 1.0f;
    clearValues[3].depthStencil.depth = 1.0f;
  }

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
                                4u, clearValues);
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                       _framebufferRef);
    _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (GBuffer)",
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
}
