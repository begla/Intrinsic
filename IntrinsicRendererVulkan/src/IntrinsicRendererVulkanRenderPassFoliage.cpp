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

void Foliage::init()
{
  using namespace Resources;

  RenderPassRefArray renderpassesToCreate;

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(Foliage));
    RenderPassManager::resetToDefault(_renderPassRef);


    AttachmentDescription albedoAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription normalAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription parameter0Attachment = {Format::kR16G16B16A16Float,
                                                  0u};
    AttachmentDescription depthStencilAttachment = { Vulkan::RenderSystem::_depthBufferFormat, //Format::kD24UnormS8UInt,
                                                    0u};

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

void Foliage::updateResolutionDependentResources()
{
  using namespace Resources;

  _albedoImageRef = ImageManager::getResourceByName(_N(GBufferAlbedo));
  _normalImageRef = ImageManager::getResourceByName(_N(GBufferNormal));
  _parameter0ImageRef = ImageManager::getResourceByName(_N(GBufferParameter0));
  _depthImageRef = ImageManager::getResourceByName(_N(GBufferDepth));

  FramebufferRefArray fbsToDestroy;
  FramebufferRefArray fbsToCreate;

  // Cleanup old resources
  {
    if (_framebufferRef.isValid())
      fbsToDestroy.push_back(_framebufferRef);

    FramebufferManager::destroyFramebuffersAndResources(fbsToDestroy);
  }

  glm::uvec2 dim = glm::uvec2(RenderSystem::_backbufferDimensions.x,
                              RenderSystem::_backbufferDimensions.y);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(Foliage));
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
    FramebufferManager::_descDimensions(_framebufferRef) = dim;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  fbsToCreate.push_back(_framebufferRef);

  FramebufferManager::createResources(fbsToCreate);
}

// <-

void Foliage::destroy() {}

// <-

void Foliage::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Foliage");
  _INTR_PROFILE_GPU("Render Foliage");

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();
  RenderSystem::_visibleDrawCallsPerMaterialPass[0u][MaterialPass::kFoliage]
      .copy(visibleDrawCalls);

  if (visibleDrawCalls.empty())
  {
    return;
  }

  DrawCallManager::sortDrawCallsFrontToBack(visibleDrawCalls);

  // Update per mesh uniform data
  {
    Core::Components::MeshManager::updateUniformData(visibleDrawCalls);
  }

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                       _framebufferRef);
    _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (Foliage)",
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
}
