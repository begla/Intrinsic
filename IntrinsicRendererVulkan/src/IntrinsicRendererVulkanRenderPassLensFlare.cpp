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

#define HALTON_SAMPLE_COUNT 1024

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
Resources::RenderPassRef _renderPassRef;
Resources::PipelineRef _pipelineRef;
Resources::DrawCallRef _drawCallRef;
Resources::ComputeCallRef _bloomXComputeCallRef;
Resources::ComputeCallRef _bloomYComputeCallRef;
Resources::ImageRef _imageRef;
Resources::ImageRef _pingPongImageRef;

void dispatchBlur(VkCommandBuffer p_CommandBuffer)
{
  using namespace Resources;

  PerInstanceDataBlur blurData = {};
  {
    blurData.mipLevel[0] = 0u;
  }

  ComputeCallManager::updateUniformMemory({_bloomXComputeCallRef}, &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_bloomXComputeCallRef, p_CommandBuffer);

  ComputeCallManager::updateUniformMemory({_bloomYComputeCallRef}, &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_bloomYComputeCallRef, p_CommandBuffer);
}
}

struct PerInstanceDataLensFlare
{
  float _dummy;
};

void LensFlare::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(_N(LensFlare));
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName("lens_flare.frag")},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(LensFlare));
    RenderPassManager::resetToDefault(_renderPassRef);

    {
      AttachmentDescription lensFlareAttachment = {Format::kR16G16B16A16Float,
                                                   0u};
      RenderPassManager::_descAttachments(_renderPassRef)
          .push_back(lensFlareAttachment);
    }
  }
  renderpassesToCreate.push_back(_renderPassRef);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_N(LensFlare));
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descFragmentProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("lens_flare.frag");
    PipelineManager::_descVertexProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("fullscreen_triangle.vert");
    PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
    PipelineManager::_descPipelineLayout(_pipelineRef) = pipelineLayout;
    PipelineManager::_descVertexLayout(_pipelineRef) = Dod::Ref();
    PipelineManager::_descDepthStencilState(_pipelineRef) =
        DepthStencilStates::kDefaultNoDepthTestAndWrite;
    PipelineManager::_descScissorRenderSize(_pipelineRef) =
        RenderSize::kQuarter;
    PipelineManager::_descViewportRenderSize(_pipelineRef) =
        RenderSize::kQuarter;
  }
  pipelinesToCreate.push_back(_pipelineRef);

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  RenderPassManager::createResources(renderpassesToCreate);
  PipelineManager::createResources(pipelinesToCreate);
}

// <-

void LensFlare::updateResolutionDependentResources()
{
  using namespace Resources;

  DrawCallRefArray drawCallsToDestroy;
  DrawCallRefArray drawCallsToCreate;
  ComputeCallRefArray computeCallsToDestroy;
  ComputeCallRefArray computeCallsToCreate;
  ImageRefArray imagesToDestroy;
  ImageRefArray imagesToCreate;
  FramebufferRefArray fbsToDestroy;
  FramebufferRefArray fbsToCreate;

  // Cleanup old resources
  {
    // Frame buffers
    if (_framebufferRef.isValid())
      fbsToDestroy.push_back(_framebufferRef);
    // Draw calls
    if (_drawCallRef.isValid())
      drawCallsToDestroy.push_back(_drawCallRef);
    // Compute calls
    if (_bloomXComputeCallRef.isValid())
      computeCallsToDestroy.push_back(_bloomXComputeCallRef);
    if (_bloomYComputeCallRef.isValid())
      computeCallsToDestroy.push_back(_bloomYComputeCallRef);
    // Images
    if (_imageRef.isValid())
      imagesToDestroy.push_back(_imageRef);
    if (_pingPongImageRef.isValid())
      imagesToDestroy.push_back(_pingPongImageRef);

    FramebufferManager::destroyFramebuffersAndResources(fbsToDestroy);
    DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
    ComputeCallManager::destroyComputeCallsAndResources(computeCallsToDestroy);
    ImageManager::destroyImagesAndResources(imagesToDestroy);
  }

  glm::uvec3 dim = glm::uvec3(RenderSystem::_backbufferDimensions.x / 4u,
                              RenderSystem::_backbufferDimensions.y / 4u, 1u);

  // Create lens flare image
  _imageRef = ImageManager::createImage(_N(LensFlare));
  {
    ImageManager::resetToDefault(_imageRef);
    ImageManager::addResourceFlags(
        _imageRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ImageManager::_descDimensions(_imageRef) = dim;
    ImageManager::_descImageFormat(_imageRef) = Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_imageRef) = ImageType::kTexture;
    ImageManager::_descImageFlags(_imageRef) |= ImageFlags::kUsageStorage;
  }
  imagesToCreate.push_back(_imageRef);

  // Create blur ping pong image
  _pingPongImageRef = ImageManager::createImage(_N(LensFlarePingPong));
  {
    ImageManager::resetToDefault(_pingPongImageRef);
    ImageManager::addResourceFlags(
        _pingPongImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ImageManager::_descDimensions(_pingPongImageRef) = dim;
    ImageManager::_descImageFormat(_pingPongImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_pingPongImageRef) = ImageType::kTexture;
    ImageManager::_descImageFlags(_pingPongImageRef) =
        ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
  }
  imagesToCreate.push_back(_pingPongImageRef);

  ImageManager::createResources(imagesToCreate);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(GBuffer));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_imageRef);

    FramebufferManager::_descDimensions(_framebufferRef) = dim;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;

    fbsToCreate.push_back(_framebufferRef);
  }

  FramebufferManager::createResources(fbsToCreate);

  // Compute calls
  _bloomXComputeCallRef =
      ComputeCallManager::createComputeCall(_N(LensFlareBlurX));
  {
    Vulkan::Resources::ComputeCallManager::resetToDefault(
        _bloomXComputeCallRef);
    Vulkan::Resources::ComputeCallManager::addResourceFlags(
        _bloomXComputeCallRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    Vulkan::Resources::ComputeCallManager::_descPipeline(
        _bloomXComputeCallRef) =
        PipelineManager::getResourceByName(_N(BloomBlurX));

    Vulkan::Resources::ComputeCallManager::_descDimensions(
        _bloomXComputeCallRef) =
        glm::uvec3(Bloom::calculateThreadGroups(dim.x, BLUR_X_THREADS_X),
                   Bloom::calculateThreadGroups(dim.y, BLUR_X_THREADS_Y), 1u);

    ComputeCallManager::bindBuffer(
        _bloomXComputeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataBlur));
    ComputeCallManager::bindImage(_bloomXComputeCallRef, _N(outTex),
                                  GpuProgramType::kCompute, _pingPongImageRef,
                                  Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(_bloomXComputeCallRef, _N(inTex),
                                  GpuProgramType::kCompute, _imageRef,
                                  Samplers::kLinearClamp);

    computeCallsToCreate.push_back(_bloomXComputeCallRef);
  }

  _bloomYComputeCallRef =
      ComputeCallManager::createComputeCall(_N(LensFlareBlurY));
  {
    Vulkan::Resources::ComputeCallManager::resetToDefault(
        _bloomYComputeCallRef);
    Vulkan::Resources::ComputeCallManager::addResourceFlags(
        _bloomYComputeCallRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    Vulkan::Resources::ComputeCallManager::_descPipeline(
        _bloomYComputeCallRef) =
        PipelineManager::getResourceByName(_N(BloomBlurY));

    Vulkan::Resources::ComputeCallManager::_descDimensions(
        _bloomYComputeCallRef) =
        glm::uvec3(Bloom::calculateThreadGroups(dim.x, BLUR_Y_THREADS_X),
                   Bloom::calculateThreadGroups(dim.y, BLUR_Y_THREADS_Y), 1u);

    ComputeCallManager::bindBuffer(
        _bloomYComputeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataBlur));
    ComputeCallManager::bindImage(_bloomYComputeCallRef, _N(outTex),
                                  GpuProgramType::kCompute, _imageRef,
                                  Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(_bloomYComputeCallRef, _N(inTex),
                                  GpuProgramType::kCompute, _pingPongImageRef,
                                  Samplers::kLinearClamp);

    computeCallsToCreate.push_back(_bloomYComputeCallRef);
  }

  ComputeCallManager::createResources(computeCallsToCreate);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_N(LensFlare));
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
        UboType::kPerInstanceFragment, sizeof(PerInstanceDataLensFlare));
    DrawCallManager::bindImage(_drawCallRef, _N(brightnessTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(_N(BloomBright)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(_drawCallRef, _N(lensFlareTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(_N(lens_flare)),
                               Samplers::kLinearClamp);
  }
  drawCallsToCreate.push_back(_drawCallRef);

  DrawCallManager::createResources(drawCallsToCreate);
}

// <-

void LensFlare::destroy() {}

// <-

void LensFlare::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Lens Flare");
  _INTR_PROFILE_GPU("Render Lens Flare");

  // Update per instance data
  PerInstanceDataLensFlare perInstanceData = {};

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, &perInstanceData,
      sizeof(PerInstanceDataLensFlare));

  ImageManager::insertImageMemoryBarrier(
      _imageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef);
  {
    RenderSystem::dispatchDrawCall(_drawCallRef,
                                   RenderSystem::getPrimaryCommandBuffer());
  }
  RenderSystem::endRenderPass(_renderPassRef);

  // Blur
  ImageManager::insertImageMemoryBarrier(
      _pingPongImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
  ImageManager::insertImageMemoryBarrier(
      _imageRef, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_GENERAL);

  static const uint32_t blurPassCount = 4u;
  for (uint32_t i = 0u; i < blurPassCount; ++i)
  {
    dispatchBlur(RenderSystem::getPrimaryCommandBuffer());
  }

  ImageManager::insertImageMemoryBarrier(
      _imageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
}
}
}
}
