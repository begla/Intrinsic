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
Resources::FramebufferRefArray _framebufferRefs;
Resources::RenderPassRef _renderPassRef;
Resources::PipelineRef _pipelineRef;
Resources::DrawCallRef _drawCallRef;

glm::vec2 _haltonSamples[HALTON_SAMPLE_COUNT];
}

namespace
{
struct PerInstanceDataPostCombine
{
  int32_t data0[4];
};
}

void PostCombine::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout =
        PipelineLayoutManager::createPipelineLayout(_N(PostCombine));
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName("post_combine.frag")},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(PostCombine));
    RenderPassManager::resetToDefault(_renderPassRef);

    {
      AttachmentDescription sceneAttachment = {Format::kB8G8R8A8Srgb, 0u};
      RenderPassManager::_descAttachments(_renderPassRef)
          .push_back(sceneAttachment);
    }
  }
  renderpassesToCreate.push_back(_renderPassRef);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_N(PostCombine));
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descFragmentProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("post_combine.frag");
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

  // Generate Halton Samples for film grain
  {
    for (uint32_t i = 0u; i < HALTON_SAMPLE_COUNT; ++i)
    {
      _haltonSamples[i] = glm::vec2(Math::calcHaltonSequence(i, 2u),
                                    Math::calcHaltonSequence(i, 3u));
    }
  }
}

// <-

void PostCombine::updateResolutionDependentResources()
{
  using namespace Resources;

  DrawCallRefArray drawCallsToDestroy;
  DrawCallRefArray drawcallsToCreate;

  // Cleanup old resources
  {
    if (_drawCallRef.isValid())
      drawCallsToDestroy.push_back(_drawCallRef);

    DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
    FramebufferManager::destroyFramebuffersAndResources(_framebufferRefs);
    _framebufferRefs.clear();
  }

  const glm::uvec3 dim = glm::uvec3(RenderSystem::_backbufferDimensions.x,
                                    RenderSystem::_backbufferDimensions.y, 1u);

  // Create framebuffers
  for (uint32_t i = 0u; i < (uint32_t)RenderSystem::_vkSwapchainImages.size();
       ++i)
  {
    FramebufferRef fbRef = FramebufferManager::createFramebuffer(_N(GBuffer));
    {
      FramebufferManager::resetToDefault(fbRef);
      FramebufferManager::addResourceFlags(
          fbRef, Dod::Resources::ResourceFlags::kResourceVolatile);

      FramebufferManager::_descAttachedImages(fbRef).push_back(
          ImageManager::getResourceByName(_INTR_STRING("Backbuffer") +
                                          StringUtil::toString<uint32_t>(i)));

      FramebufferManager::_descDimensions(fbRef) =
          glm::uvec2(RenderSystem::_backbufferDimensions.x,
                     RenderSystem::_backbufferDimensions.y);
      FramebufferManager::_descRenderPass(fbRef) = _renderPassRef;

      _framebufferRefs.push_back(fbRef);
    }
  }

  FramebufferManager::createResources(_framebufferRefs);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_N(PostCombine));
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
        UboType::kPerInstanceFragment, sizeof(PerInstanceDataPostCombine));
    DrawCallManager::bindImage(
        _drawCallRef, _N(sceneTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(Scene)), Samplers::kNearestClamp);
    DrawCallManager::bindImage(_drawCallRef, _N(bloomTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(_N(BloomSummed)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(lensDirtTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(lens_dirt)), Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(filmGrainTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(film_grain_noise)),
        Samplers::kNearestRepeat);
    DrawCallManager::bindImage(
        _drawCallRef, _N(lensFlareTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(LensFlare)), Samplers::kLinearClamp);
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(AvgLum), GpuProgramType::kFragment,
        Vulkan::Resources::BufferManager::getResourceByName(_N(BloomLumBuffer)),
        UboType::kInvalidUbo, sizeof(float));
  }
  drawcallsToCreate.push_back(_drawCallRef);

  DrawCallManager::createResources(drawcallsToCreate);
}

// <-

void PostCombine::destroy() {}

// <-

void PostCombine::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Post Combine");
  _INTR_PROFILE_GPU("Render Post Combine");

  // Update per instance data
  PerInstanceDataPostCombine perInstanceData = {
      (int32_t)(
          _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].x *
          255),
      (int32_t)(
          _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].y *
          255),
      0, 0};

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, &perInstanceData,
      sizeof(PerInstanceDataPostCombine));

  RenderSystem::beginRenderPass(
      _renderPassRef, _framebufferRefs[RenderSystem::_backbufferIndex],
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
