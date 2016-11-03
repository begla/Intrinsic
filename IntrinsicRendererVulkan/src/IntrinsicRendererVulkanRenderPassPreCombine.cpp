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
Resources::FramebufferRef _framebufferRef;
Resources::ImageRef _imageRef;
Resources::RenderPassRef _renderPassRef;
Resources::PipelineRef _pipelineRef;
Resources::DrawCallRef _drawCallRef;

// <-

struct PerInstanceDataPreCombine
{
  glm::mat4 invProjMatrix;
  glm::mat4 invViewProjMatrix;
  glm::vec4 camPosition;
};
}

void PreCombine::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout =
        PipelineLayoutManager::createPipelineLayout(_N(PreCombine));
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName("pre_combine.frag")},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(PreCombine));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription colorAttachment = {Format::kR16G16B16A16Float, 0u};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(colorAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_N(PreCombine));
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descFragmentProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("pre_combine.frag");
    PipelineManager::_descVertexProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("fullscreen_triangle.vert");
    PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
    PipelineManager::_descPipelineLayout(_pipelineRef) = pipelineLayout;
    PipelineManager::_descVertexLayout(_pipelineRef) = Dod::Ref();
    PipelineManager::_descDepthStencilState(_pipelineRef) =
        DepthStencilStates::kDefaultNoDepthTestAndWrite;
  }
  pipelinesToCreate.push_back(_pipelineRef);

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  RenderPassManager::createResources(renderpassesToCreate);
  PipelineManager::createResources(pipelinesToCreate);
}

// <-

void PreCombine::updateResolutionDependentResources()
{
  using namespace Resources;

  DrawCallRefArray imagesToDestroy;
  DrawCallRefArray imagesToCreate;
  DrawCallRefArray framebuffersToDestroy;
  DrawCallRefArray framebuffersToCreate;
  DrawCallRefArray drawCallsToDestroy;
  DrawCallRefArray drawCallsToCreate;

  // Cleanup old resources
  {
    if (_drawCallRef.isValid())
      drawCallsToDestroy.push_back(_drawCallRef);
    if (_framebufferRef.isValid())
      framebuffersToDestroy.push_back(_framebufferRef);
    if (_imageRef.isValid())
      imagesToDestroy.push_back(_imageRef);

    DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
    FramebufferManager::destroyFramebuffersAndResources(framebuffersToDestroy);
    ImageManager::destroyImagesAndResources(imagesToDestroy);
  }

  glm::uvec3 dim = glm::uvec3(RenderSystem::_backbufferDimensions.x,
                              RenderSystem::_backbufferDimensions.y, 1u);

  // Create scene image
  _imageRef = ImageManager::createImage(_N(Scene));
  {
    ImageManager::resetToDefault(_imageRef);
    ImageManager::addResourceFlags(
        _imageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_imageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_imageRef) = dim;
    ImageManager::_descImageFormat(_imageRef) = Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_imageRef) = ImageType::kTexture;
  }
  imagesToCreate.push_back(_imageRef);

  ImageManager::createResources(imagesToCreate);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(GBuffer));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_imageRef);

    FramebufferManager::_descDimensions(_framebufferRef) =
        glm::uvec2(RenderSystem::_backbufferDimensions.x,
                   RenderSystem::_backbufferDimensions.y);
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  framebuffersToCreate.push_back(_framebufferRef);

  FramebufferManager::createResources(framebuffersToCreate);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_N(PreCombine));
  {
    Vulkan::Resources::DrawCallManager::resetToDefault(_drawCallRef);
    Vulkan::Resources::DrawCallManager::addResourceFlags(
        _drawCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    Vulkan::Resources::DrawCallManager::_descPipeline(_drawCallRef) =
        _pipelineRef;
    Vulkan::Resources::DrawCallManager::_descVertexCount(_drawCallRef) = 3u;

    DrawCallManager::bindBuffer(
        _drawCallRef, _N(PerInstance), GpuProgramType::kFragment,
        UniformManager::_perInstanceUniformBuffer,
        UboType::kPerInstanceFragment, sizeof(PerInstanceDataPreCombine));
    DrawCallManager::bindImage(
        _drawCallRef, _N(albedoTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferAlbedo)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(normalTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferNormal)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(param0Tex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferParameter0)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(albedoTransparentsTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsAlbedo)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(normalTransparentsTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsNormal)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(param0TransparentsTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsParameter0)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(_drawCallRef, _N(lightBufferTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(_N(LightBuffer)),
                               Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(lightBufferTransparentsTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(LightBufferTransparents)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(depthBufferTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferDepth)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(depthBufferTransparentsTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsDepth)),
        Samplers::kNearestClamp);
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(MaterialBuffer), GpuProgramType::kFragment,
        MaterialBuffer::_materialBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(MaterialBuffer::_materialBuffer));
    DrawCallManager::bindImage(
        _drawCallRef, _N(volumetricLightingScatteringBufferTex),
        GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(VolumetricLightingScatteringBuffer)),
        Samplers::kLinearClamp);
  }
  drawCallsToCreate.push_back(_drawCallRef);

  DrawCallManager::createResources(drawCallsToCreate);
}

// <-

void PreCombine::destroy() {}

// <-

void PreCombine::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Renderer", "Render Pre Combine");
  _INTR_PROFILE_GPU("Render Pre Combine");

  Components::CameraRef camRef = World::getActiveCamera();
  Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(camRef));

  // Update per instance data
  PerInstanceDataPreCombine perInstanceData = {
      Components::CameraManager::_inverseProjectionMatrix(camRef),
      Components::CameraManager::_inverseViewProjectionMatrix(camRef),
      glm::vec4(Components::NodeManager::_worldPosition(camNodeRef), 0.0f)};

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, &perInstanceData,
      sizeof(PerInstanceDataPreCombine));

  ImageManager::insertImageMemoryBarrier(
      _imageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_INLINE);
  {
    RenderSystem::dispatchDrawCall(_drawCallRef,
                                   RenderSystem::getPrimaryCommandBuffer());
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
}
