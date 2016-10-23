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
    _renderPassRef = RenderPassManager::createRenderPass(_N(Sky));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription albedoAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription depthStencilAttachment = {Format::kD24UnormS8UInt,
                                                    0u};

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
  _framebufferRef = FramebufferManager::createFramebuffer(_N(Sky));
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

  static Components::MeshRefArray visibleMeshes;
  visibleMeshes.clear();
  visibleMeshes.insert(
      visibleMeshes.begin(),
      RenderSystem::_visibleMeshComponentsPerMaterialPass[0u]
                                                         [MaterialPass::kSky]
                                                             .begin(),
      RenderSystem::_visibleMeshComponentsPerMaterialPass[0u]
                                                         [MaterialPass::kSky]
                                                             .end());

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();
  visibleDrawCalls.insert(
      visibleDrawCalls.begin(),
      RenderSystem::_visibleDrawCallsPerMaterialPass[0u][MaterialPass::kSky]
          .begin(),
      RenderSystem::_visibleDrawCallsPerMaterialPass[0u][MaterialPass::kSky]
          .end());

  if (visibleDrawCalls.empty())
  {
    return;
  }

  // Update per mesh uniform data
  {
    Core::Components::MeshManager::updateUniformData(
        RenderSystem::_visibleDrawCallsPerMaterialPass[0u][MaterialPass::kSky]);
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
