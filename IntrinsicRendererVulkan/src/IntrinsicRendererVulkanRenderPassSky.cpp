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
Resources::ImageRef _gbufferAlbedoImageRef;
Resources::ImageRef _gbufferDepthImageRef;
Resources::FramebufferRef _framebufferRef;
Resources::RenderPassRef _renderPassRef;
}

void Sky::init()
{
  using namespace Resources;

  RenderPassRefArray renderpassesToCreate;

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(GBufferSky));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription albedoAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription depthStencilAttachment = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse, 0u};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(albedoAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(depthStencilAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderpassesToCreate);
}

// <-

void Sky::updateResolutionDependentResources()
{
  using namespace Resources;

  _gbufferAlbedoImageRef = ImageManager::getResourceByName(_N(GBufferAlbedo));
  _gbufferDepthImageRef = ImageManager::getResourceByName(_N(GBufferDepth));

  FramebufferRefArray fbsToCreate;
  FramebufferRefArray fbsToDestroy;

  // Cleanup old resources
  {
    if (_framebufferRef.isValid())
      fbsToDestroy.push_back(_framebufferRef);

    FramebufferManager::destroyFramebuffersAndResources(fbsToDestroy);
  }

  glm::uvec2 dim = glm::uvec2(RenderSystem::_backbufferDimensions.x,
                              RenderSystem::_backbufferDimensions.y);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(GBufferSky));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_gbufferAlbedoImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_gbufferDepthImageRef);
    FramebufferManager::_descDimensions(_framebufferRef) = dim;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  fbsToCreate.push_back(_framebufferRef);

  FramebufferManager::createResources(fbsToCreate);
}

// <-

void Sky::destroy() {}

// <-

void Sky::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Sky");
  _INTR_PROFILE_GPU("Render Sky");

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();
  RenderProcess::Default::_visibleDrawCallsPerMaterialPass
      [0u][MaterialManager::getMaterialPassId(_N(GBufferSky))]
          .copy(visibleDrawCalls);

  if (visibleDrawCalls.empty())
  {
    return;
  }

  // Update per mesh uniform data
  {
    Core::Components::MeshManager::updateUniformData(visibleDrawCalls);
  }

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                       _framebufferRef);
    _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (Sky)",
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
}
