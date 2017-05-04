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
Resources::ImageRef _lightingBufferImageRef;
Resources::ImageRef _lightingBufferTransparentsImageRef;
Resources::FramebufferRef _framebufferRef;
Resources::FramebufferRef _framebufferTransparentsRef;
Resources::RenderPassRef _renderPassRef;
Resources::PipelineRef _pipelineRef;
Resources::DrawCallRef _drawCallRef;
Resources::DrawCallRef _drawCallTransparentsRef;

// <-

struct PerInstanceData
{
  glm::mat4 viewMatrix;
  glm::mat4 invProjectionMatrix;
  glm::mat4 invViewMatrix;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];
} _perInstanceData;

// <-

void renderLighting(Resources::FramebufferRef p_FramebufferRef,
                    Resources::DrawCallRef p_DrawCall,
                    Resources::ImageRef p_LightingBufferRef,
                    Components::CameraRef p_CameraRef)
{
  using namespace Resources;

  // Update per instance data
  {
    _perInstanceData.invProjectionMatrix =
        Components::CameraManager::_inverseProjectionMatrix(p_CameraRef);
    _perInstanceData.viewMatrix =
        Components::CameraManager::_viewMatrix(p_CameraRef);
    _perInstanceData.invViewMatrix =
        Components::CameraManager::_inverseViewMatrix(p_CameraRef);

    const _INTR_ARRAY(Core::Resources::FrustumRef)& shadowFrustums =
        RenderProcess::Default::_shadowFrustums[p_CameraRef];

    for (uint32_t i = 0u; i < shadowFrustums.size(); ++i)
    {
      Core::Resources::FrustumRef shadowFrustumRef = shadowFrustums[i];

      // Transform from camera view space => light proj. space
      _perInstanceData.shadowViewProjMatrix[i] =
          Core::Resources::FrustumManager::_viewProjectionMatrix(
              shadowFrustumRef) *
          Components::CameraManager::_inverseViewMatrix(p_CameraRef);
    }

    DrawCallRefArray dcsToUpdate = {p_DrawCall};
    DrawCallManager::allocateAndUpdateUniformMemory(
        dcsToUpdate, nullptr, 0u, &_perInstanceData, sizeof(PerInstanceData));
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ImageManager::insertImageMemoryBarrier(
      p_LightingBufferRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  RenderSystem::beginRenderPass(_renderPassRef, p_FramebufferRef,
                                VK_SUBPASS_CONTENTS_INLINE);
  {
    RenderSystem::dispatchDrawCall(p_DrawCall, primaryCmdBuffer);
  }
  RenderSystem::endRenderPass(_renderPassRef);

  ImageManager::insertImageMemoryBarrier(
      p_LightingBufferRef, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
}

void Lighting::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(_N(Lighting));
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u, {Resources::GpuProgramManager::getResourceByName("lighting.frag")},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(Lighting));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription colorAttachment = {Format::kR16G16B16A16Float, 0u};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(colorAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_N(Lighting));
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descFragmentProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("lighting.frag");
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

void Lighting::onReinitRendering()
{
  using namespace Resources;

  ImageRefArray imgsToDestroy;
  ImageRefArray imgsToCreate;
  FramebufferRefArray framebuffersToDestroy;
  FramebufferRefArray framebuffersToCreate;
  DrawCallRefArray drawCallsToDestroy;
  DrawCallRefArray drawcallsToCreate;

  // Cleanup old resources
  {
    if (_drawCallRef.isValid())
      drawCallsToDestroy.push_back(_drawCallRef);
    if (_drawCallTransparentsRef.isValid())
      drawCallsToDestroy.push_back(_drawCallTransparentsRef);

    if (_framebufferRef.isValid())
      framebuffersToDestroy.push_back(_framebufferRef);
    if (_framebufferTransparentsRef.isValid())
      framebuffersToDestroy.push_back(_framebufferTransparentsRef);

    if (_lightingBufferImageRef.isValid())
      imgsToDestroy.push_back(_lightingBufferImageRef);
    if (_lightingBufferTransparentsImageRef.isValid())
      imgsToDestroy.push_back(_lightingBufferTransparentsImageRef);

    FramebufferManager::destroyFramebuffersAndResources(framebuffersToDestroy);
    DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
    ImageManager::destroyImagesAndResources(imgsToDestroy);
  }

  glm::uvec3 dim = glm::uvec3(RenderSystem::_backbufferDimensions.x,
                              RenderSystem::_backbufferDimensions.y, 1u);

  _lightingBufferImageRef = ImageManager::createImage(_N(LightBuffer));
  {
    ImageManager::resetToDefault(_lightingBufferImageRef);
    ImageManager::addResourceFlags(
        _lightingBufferImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_lightingBufferImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_lightingBufferImageRef) = dim;
    ImageManager::_descImageFormat(_lightingBufferImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_lightingBufferImageRef) = ImageType::kTexture;
  }
  imgsToCreate.push_back(_lightingBufferImageRef);

  _lightingBufferTransparentsImageRef =
      ImageManager::createImage(_N(LightBufferTransparents));
  {
    ImageManager::resetToDefault(_lightingBufferTransparentsImageRef);
    ImageManager::addResourceFlags(
        _lightingBufferTransparentsImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_lightingBufferTransparentsImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_lightingBufferTransparentsImageRef) = dim;
    ImageManager::_descImageFormat(_lightingBufferTransparentsImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_lightingBufferTransparentsImageRef) =
        ImageType::kTexture;
  }
  imgsToCreate.push_back(_lightingBufferTransparentsImageRef);

  ImageManager::createResources(imgsToCreate);

  _framebufferRef = FramebufferManager::createFramebuffer(_N(Lighting));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_lightingBufferImageRef);

    FramebufferManager::_descDimensions(_framebufferRef) =
        glm::uvec2(RenderSystem::_backbufferDimensions.x,
                   RenderSystem::_backbufferDimensions.y);
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;

    framebuffersToCreate.push_back(_framebufferRef);
  }

  _framebufferTransparentsRef =
      FramebufferManager::createFramebuffer(_N(LightingTransparents));
  {
    FramebufferManager::resetToDefault(_framebufferTransparentsRef);
    FramebufferManager::addResourceFlags(
        _framebufferTransparentsRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferTransparentsRef)
        .push_back(_lightingBufferTransparentsImageRef);

    FramebufferManager::_descDimensions(_framebufferTransparentsRef) =
        glm::uvec2(RenderSystem::_backbufferDimensions.x,
                   RenderSystem::_backbufferDimensions.y);
    FramebufferManager::_descRenderPass(_framebufferTransparentsRef) =
        _renderPassRef;

    framebuffersToCreate.push_back(_framebufferTransparentsRef);
  }

  FramebufferManager::createResources(framebuffersToCreate);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_N(Lighting));
  {
    DrawCallManager::resetToDefault(_drawCallRef);
    DrawCallManager::addResourceFlags(
        _drawCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    DrawCallManager::_descPipeline(_drawCallRef) = _pipelineRef;
    DrawCallManager::_descVertexCount(_drawCallRef) = 3u;

    DrawCallManager::bindBuffer(
        _drawCallRef, _N(PerInstance), GpuProgramType::kFragment,
        UniformManager::_perInstanceUniformBuffer,
        UboType::kPerInstanceFragment, sizeof(PerInstanceData));
    DrawCallManager::bindImage(
        _drawCallRef, _N(albedoTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferAlbedo)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(normalTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferNormal)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(parameter0Tex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferParameter0)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(depthTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferDepth)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(_drawCallRef, _N(irradianceTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(
                                   _N(GCanyon_C_YumaPoint_3k_cube_irradiance)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(_drawCallRef, _N(specularTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(
                                   _N(GCanyon_C_YumaPoint_3k_cube_specular)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallRef, _N(shadowBufferTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(MaterialBuffer), GpuProgramType::kFragment,
        MaterialBuffer::_materialBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(MaterialBuffer::_materialBuffer));
  }

  drawcallsToCreate.push_back(_drawCallRef);

  _drawCallTransparentsRef =
      DrawCallManager::createDrawCall(_N(LightingTransparents));
  {
    DrawCallManager::resetToDefault(_drawCallTransparentsRef);
    DrawCallManager::addResourceFlags(
        _drawCallTransparentsRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    DrawCallManager::_descPipeline(_drawCallTransparentsRef) = _pipelineRef;
    DrawCallManager::_descVertexCount(_drawCallTransparentsRef) = 3u;

    DrawCallManager::bindBuffer(
        _drawCallTransparentsRef, _N(PerInstance), GpuProgramType::kFragment,
        UniformManager::_perInstanceUniformBuffer,
        UboType::kPerInstanceFragment, sizeof(PerInstanceData));
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(albedoTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsAlbedo)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(normalTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsNormal)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(parameter0Tex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsParameter0)),
        Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(depthTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(GBufferTransparentsDepth)),
        Samplers::kNearestClamp);
    DrawCallManager::bindImage(_drawCallTransparentsRef, _N(irradianceTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(
                                   _N(GCanyon_C_YumaPoint_3k_cube_irradiance)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(_drawCallTransparentsRef, _N(specularTex),
                               GpuProgramType::kFragment,
                               ImageManager::getResourceByName(
                                   _N(GCanyon_C_YumaPoint_3k_cube_specular)),
                               Samplers::kLinearClamp);
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(shadowBufferTex),
        GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
    DrawCallManager::bindBuffer(
        _drawCallTransparentsRef, _N(MaterialBuffer), GpuProgramType::kFragment,
        MaterialBuffer::_materialBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(MaterialBuffer::_materialBuffer));
  }
  drawcallsToCreate.push_back(_drawCallTransparentsRef);

  DrawCallManager::createResources(drawcallsToCreate);
}

// <-

void Lighting::destroy() {}

// <-

void Lighting::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Lighting");
  _INTR_PROFILE_GPU("Render Lighting");

  renderLighting(_framebufferRef, _drawCallRef, _lightingBufferImageRef,
                 p_CameraRef);
  renderLighting(_framebufferTransparentsRef, _drawCallTransparentsRef,
                 _lightingBufferTransparentsImageRef, p_CameraRef);
}
}
}
}
}
