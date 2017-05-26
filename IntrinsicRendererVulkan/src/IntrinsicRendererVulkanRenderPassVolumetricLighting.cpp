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
struct PerInstanceDataESMGenerate
{
  glm::uvec4 arrayIdx;
};

struct PerInstanceDataESMBlur
{
  glm::vec4 blurParams;
  glm::uvec4 arrayIdx;
};

struct PerInstanceData
{
  glm::mat4 projMatrix;
  glm::mat4 prevViewProjMatrix;

  glm::vec4 eyeVSVectorX;
  glm::vec4 eyeVSVectorY;
  glm::vec4 eyeVSVectorZ;

  glm::vec4 eyeWSVectorX;
  glm::vec4 eyeWSVectorY;
  glm::vec4 eyeWSVectorZ;

  glm::vec4 data0;

  glm::vec4 camPos;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];

  glm::vec4 nearFar;
  glm::vec4 nearFarWidthHeight;

  glm::vec4 mainLightDirAndTemp;
  glm::vec4 mainLightColorAndIntens;

  glm::vec4 haltonSamples;
} _perInstanceData;

glm::mat4 prevViewProjMatrix;

Resources::ImageRef _volLightingBufferImageRef;
Resources::ImageRef _volLightingBufferPrevFrameImageRef;
Resources::ImageRef _volLightingScatteringBufferImageRef;
Resources::ImageRef _shadowBufferExp;
Resources::ImageRef _shadowBufferExpPingPong;

Resources::RenderPassRef _renderPassRef;
Resources::FramebufferRefArray _framebufferRefs;
Resources::FramebufferRefArray _framebufferPingPongRefs;

Resources::PipelineRef _pipelineAccumRef;
Resources::PipelineRef _pipelineScatteringRef;

Resources::DrawCallRef _drawCallEsmGenerateRef;
Resources::DrawCallRef _drawCallEsmBlurRef;
Resources::DrawCallRef _drawCallEsmBlurPingPongRef;

Resources::ComputeCallRef _computeCallAccumRef;
Resources::ComputeCallRef _computeCallAccumPrevFrameRef;
Resources::ComputeCallRef _computeCallScatteringRef;
Resources::ComputeCallRef _computeCallScatteringPrevFrameRef;

_INTR_INLINE void
updatePerInstanceData(Components::CameraRef p_CameraRef,
                      Resources::ComputeCallRef p_CurrentAccumComputeCallRef)
{
  using namespace Resources;

  Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(p_CameraRef));

  // Post effect data
  {
    const glm::vec3 mainLightDir =
        Core::Resources::PostEffectManager::_descMainLightOrientation(
            Core::Resources::PostEffectManager::_blendTargetRef) *
        glm::vec3(0.0f, 0.0f, 1.0f);

    _perInstanceData.data0.x =
        Core::Resources::PostEffectManager::_descVolumetricLightingScattering(
            Core::Resources::PostEffectManager::_blendTargetRef) *
        VolumetricLighting::_globalScatteringFactor;
    _perInstanceData.data0.z =
        Core::Resources::PostEffectManager::_descAmbientFactor(
            Core::Resources::PostEffectManager::_blendTargetRef) *
        Lighting::_globalAmbientFactor;
    _perInstanceData.mainLightDirAndTemp = glm::vec4(
        mainLightDir, Core::Resources::PostEffectManager::_descMainLightTemp(
                          Core::Resources::PostEffectManager::_blendTargetRef));
    _perInstanceData.mainLightColorAndIntens =
        glm::vec4(Core::Resources::PostEffectManager::_descMainLightColor(
                      Core::Resources::PostEffectManager::_blendTargetRef),
                  Core::Resources::PostEffectManager::_descMainLightIntens(
                      Core::Resources::PostEffectManager::_blendTargetRef));
  }

  _perInstanceData.haltonSamples =
      RenderProcess::UniformManager::_uniformDataSource.haltonSamples32;

  _perInstanceData.prevViewProjMatrix = prevViewProjMatrix;
  prevViewProjMatrix =
      Components::CameraManager::_viewProjectionMatrix(p_CameraRef);
  _perInstanceData.projMatrix =
      Components::CameraManager::_projectionMatrix(p_CameraRef);

  _perInstanceData.camPos =
      glm::vec4(Components::NodeManager::_worldPosition(camNodeRef),
                TaskManager::_frameCounter);

  _perInstanceData.eyeVSVectorX = glm::vec4(
      glm::vec3(1.0 / _perInstanceData.projMatrix[0][0], 0.0, 0.0), 0.0);
  _perInstanceData.eyeVSVectorY = glm::vec4(
      glm::vec3(0.0, 1.0 / _perInstanceData.projMatrix[1][1], 0.0), 0.0);
  _perInstanceData.eyeVSVectorZ = glm::vec4(glm::vec3(0.0, 0.0, -1.0), 0.0);
  _perInstanceData.eyeWSVectorX =
      Components::CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorX;
  _perInstanceData.eyeWSVectorX.w = TaskManager::_totalTimePassed;
  _perInstanceData.eyeWSVectorY =
      Components::CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorY;
  _perInstanceData.eyeWSVectorZ =
      Components::CameraManager::_inverseViewMatrix(p_CameraRef) *
      _perInstanceData.eyeVSVectorZ;

  _perInstanceData.nearFar = glm::vec4(
      Components::CameraManager::_descNearPlane(p_CameraRef),
      Components::CameraManager::_descFarPlane(p_CameraRef), 0.0f, 0.0f);

  Math::FrustumCorners viewSpaceCorners;
  Math::extractFrustumsCorners(
      Components::CameraManager::_inverseProjectionMatrix(p_CameraRef),
      viewSpaceCorners);

  _perInstanceData.nearFarWidthHeight = glm::vec4(
      viewSpaceCorners.c[3].x - viewSpaceCorners.c[2].x /* Near Width */,
      viewSpaceCorners.c[2].y - viewSpaceCorners.c[1].y /* Near Height */,
      viewSpaceCorners.c[7].x - viewSpaceCorners.c[6].x /* Far Width */,
      viewSpaceCorners.c[6].y - viewSpaceCorners.c[5].y /* Far Height */);

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

  ComputeCallManager::updateUniformMemory({p_CurrentAccumComputeCallRef},
                                          &_perInstanceData,
                                          sizeof(PerInstanceData));
}

_INTR_INLINE Resources::ComputeCallRef
createComputeCallAccumulation(glm::vec3 p_Dim,
                              Resources::BufferRef p_LightBuffer,
                              Resources::BufferRef p_LightIndexBuffer,
                              Resources::BufferRef p_IrradProbeBuffer,
                              Resources::BufferRef p_IrradProbeIndexBuffer,
                              Resources::BufferRef p_CurrentVolLightingBuffer,
                              Resources::BufferRef p_PrevVolLightingBuffer)
{
  using namespace Resources;

  ComputeCallRef computeCallRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLighting));
  {
    ComputeCallManager::resetToDefault(computeCallRef);
    ComputeCallManager::addResourceFlags(
        computeCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallRef) =
        glm::uvec3(Math::divideByMultiple(p_Dim.x, 4u),
                   Math::divideByMultiple(p_Dim.y, 4u),
                   Math::divideByMultiple(p_Dim.z, 4u));
    ComputeCallManager::_descPipeline(computeCallRef) = _pipelineAccumRef;

    ComputeCallManager::bindBuffer(
        computeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        computeCallRef, _N(shadowBufferTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
    ComputeCallManager::bindImage(computeCallRef, _N(shadowBufferExpTex),
                                  GpuProgramType::kCompute, _shadowBufferExp,
                                  Samplers::kLinearClamp);
    ComputeCallManager::bindImage(
        computeCallRef, _N(output0Tex), GpuProgramType::kCompute,
        p_CurrentVolLightingBuffer, Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(
        computeCallRef, _N(prevVolLightBufferTex), GpuProgramType::kCompute,
        p_PrevVolLightingBuffer, Samplers::kLinearClamp);
    ComputeCallManager::bindImage(
        computeCallRef, _N(kelvinLutTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(_N(kelvin_rgb_LUT)),
        Samplers::kLinearClamp);
    ComputeCallManager::bindBuffer(
        computeCallRef, _N(LightBuffer), GpuProgramType::kCompute,
        p_LightBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(p_LightBuffer));
    ComputeCallManager::bindBuffer(
        computeCallRef, _N(LightIndexBuffer), GpuProgramType::kCompute,
        p_LightIndexBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(p_LightIndexBuffer));
    ComputeCallManager::bindBuffer(
        computeCallRef, _N(IrradProbeBuffer), GpuProgramType::kCompute,
        p_IrradProbeBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(p_IrradProbeBuffer));
    ComputeCallManager::bindBuffer(
        computeCallRef, _N(IrradProbeIndexBuffer), GpuProgramType::kCompute,
        p_IrradProbeIndexBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(p_IrradProbeIndexBuffer));
  }

  return computeCallRef;
}

_INTR_INLINE Resources::ComputeCallRef
createComputeCallScattering(glm::vec3 p_Dim,
                            Resources::BufferRef p_CurrentVolLightingBuffer)
{
  using namespace Resources;

  ComputeCallRef computeCallScatteringRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLighting));
  {
    ComputeCallManager::resetToDefault(computeCallScatteringRef);
    ComputeCallManager::addResourceFlags(
        computeCallScatteringRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(computeCallScatteringRef) =
        glm::uvec3(Math::divideByMultiple(p_Dim.x, 8u),
                   Math::divideByMultiple(p_Dim.y, 8u), 1u);
    ComputeCallManager::_descPipeline(computeCallScatteringRef) =
        _pipelineScatteringRef;

    ComputeCallManager::bindBuffer(
        computeCallScatteringRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        computeCallScatteringRef, _N(volLightBufferTex),
        GpuProgramType::kCompute, p_CurrentVolLightingBuffer,
        Samplers::kNearestClamp);
    ComputeCallManager::bindImage(
        computeCallScatteringRef, _N(volLightScatterBufferTex),
        GpuProgramType::kCompute, _volLightingScatteringBufferImageRef,
        Samplers::kInvalidSampler);
  }

  return computeCallScatteringRef;
}

_INTR_INLINE void generateExponentialShadowMaps(uint32_t p_ShadowMapCount)
{
  using namespace Resources;

  for (uint32_t shadowMapIndex = 0u; shadowMapIndex < p_ShadowMapCount;
       ++shadowMapIndex)
  {
    PerInstanceDataESMGenerate instanceData;
    instanceData.arrayIdx.x = shadowMapIndex;

    DrawCallManager::allocateAndUpdateUniformMemory(
        {_drawCallEsmGenerateRef}, nullptr, 0u, &instanceData,
        sizeof(PerInstanceDataESMGenerate));

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExp, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0u, shadowMapIndex);

    RenderSystem::beginRenderPass(_renderPassRef,
                                  _framebufferRefs[shadowMapIndex],
                                  VK_SUBPASS_CONTENTS_INLINE, 0u, nullptr);
    {

      RenderSystem::dispatchDrawCall(_drawCallEsmGenerateRef,
                                     RenderSystem::getPrimaryCommandBuffer());
    }
    RenderSystem::endRenderPass(_renderPassRef);

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExp, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0u, shadowMapIndex);
  }
}

_INTR_INLINE void blurExponentialShadowMaps(uint32_t p_ShadowMapCount)
{
  using namespace Resources;

  static const float blurRadius = 2.0f;

  for (uint32_t shadowMapIndex = 0u; shadowMapIndex < p_ShadowMapCount;
       ++shadowMapIndex)
  {
    PerInstanceDataESMBlur instanceData;
    instanceData.arrayIdx.x = shadowMapIndex;
    instanceData.blurParams = glm::vec4(blurRadius, 0.0f, 1.0f, 0.0f);

    // Blur horizontally
    DrawCallManager::allocateAndUpdateUniformMemory(
        {_drawCallEsmBlurRef}, nullptr, 0u, &instanceData,
        sizeof(PerInstanceDataESMBlur));

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExpPingPong, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0u, shadowMapIndex);

    RenderSystem::beginRenderPass(_renderPassRef,
                                  _framebufferPingPongRefs[shadowMapIndex],
                                  VK_SUBPASS_CONTENTS_INLINE, 0u, nullptr);
    {

      RenderSystem::dispatchDrawCall(_drawCallEsmBlurRef,
                                     RenderSystem::getPrimaryCommandBuffer());
    }
    RenderSystem::endRenderPass(_renderPassRef);

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExp, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0u, shadowMapIndex);
    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExpPingPong, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0u, shadowMapIndex);

    // Blur vertically
    instanceData.blurParams = glm::vec4(blurRadius, 0.0f, 0.0f, 1.0f);

    DrawCallManager::allocateAndUpdateUniformMemory(
        {_drawCallEsmBlurPingPongRef}, nullptr, 0u, &instanceData,
        sizeof(PerInstanceDataESMBlur));

    RenderSystem::beginRenderPass(_renderPassRef,
                                  _framebufferRefs[shadowMapIndex],
                                  VK_SUBPASS_CONTENTS_INLINE, 0u, nullptr);
    {

      RenderSystem::dispatchDrawCall(_drawCallEsmBlurPingPongRef,
                                     RenderSystem::getPrimaryCommandBuffer());
    }
    RenderSystem::endRenderPass(_renderPassRef);

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferExp, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0u, shadowMapIndex);
  }
}
}

// Static members
float VolumetricLighting::_globalScatteringFactor = 1.0f;

void VolumetricLighting::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  DrawCallRefArray drawCallsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutAccum;
  PipelineLayoutRef pipelineLayoutScattering;
  PipelineLayoutRef pipelineLayoutEsm;
  {
    {
      pipelineLayoutAccum =
          PipelineLayoutManager::createPipelineLayout(_N(VolumetricLighting));
      PipelineLayoutManager::resetToDefault(pipelineLayoutAccum);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {Resources::GpuProgramManager::getResourceByName(
              "volumetric_lighting.comp")},
          pipelineLayoutAccum);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutAccum);

    {
      pipelineLayoutScattering = PipelineLayoutManager::createPipelineLayout(
          _N(VolumetricLightingScattering));
      PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {Resources::GpuProgramManager::getResourceByName(
              "volumetric_lighting_scattering.comp")},
          pipelineLayoutScattering);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);

    {
      pipelineLayoutEsm =
          PipelineLayoutManager::createPipelineLayout(_N(ESMGenerate));
      PipelineLayoutManager::resetToDefault(pipelineLayoutEsm);

      GpuProgramManager::reflectPipelineLayout(
          8u,
          {Resources::GpuProgramManager::getResourceByName(
              "esm_generate.frag")},
          pipelineLayoutEsm);
    }
    pipelineLayoutsToCreate.push_back(pipelineLayoutEsm);
  }

  const glm::uvec2 expShadowBufferDim = RenderPass::Shadow::_shadowMapSize / 4u;

  // Render passes
  RenderPassRefArray renderPassesToCreate;
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(ShadowESM));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription shadowBufferExpAttachment = {
        (uint8_t)Format::kR32G32B32A32SFloat, 0u};
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(shadowBufferExpAttachment);
  }
  renderPassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderPassesToCreate);

  // Pipeline
  PipelineRef pipelineEsmGenerateRef;
  PipelineRef pipelineEsmBlurRef;
  {
    {
      _pipelineAccumRef =
          PipelineManager::createPipeline(_N(VolumetricLighting));
      PipelineManager::resetToDefault(_pipelineAccumRef);

      PipelineManager::_descComputeProgram(_pipelineAccumRef) =
          GpuProgramManager::getResourceByName("volumetric_lighting.comp");
      PipelineManager::_descPipelineLayout(_pipelineAccumRef) =
          pipelineLayoutAccum;
    }
    pipelinesToCreate.push_back(_pipelineAccumRef);

    {
      _pipelineScatteringRef =
          PipelineManager::createPipeline(_N(VolumetricLighting));
      PipelineManager::resetToDefault(_pipelineScatteringRef);

      PipelineManager::_descComputeProgram(_pipelineScatteringRef) =
          GpuProgramManager::getResourceByName(
              "volumetric_lighting_scattering.comp");
      PipelineManager::_descPipelineLayout(_pipelineScatteringRef) =
          pipelineLayoutScattering;
    }
    pipelinesToCreate.push_back(_pipelineScatteringRef);

    pipelineEsmGenerateRef = PipelineManager::createPipeline(_N(ESMGenerate));
    {

      PipelineManager::resetToDefault(pipelineEsmGenerateRef);

      PipelineManager::_descFragmentProgram(pipelineEsmGenerateRef) =
          GpuProgramManager::getResourceByName("esm_generate.frag");
      PipelineManager::_descVertexProgram(pipelineEsmGenerateRef) =
          GpuProgramManager::getResourceByName("fullscreen_triangle.vert");
      PipelineManager::_descRenderPass(pipelineEsmGenerateRef) = _renderPassRef;
      PipelineManager::_descPipelineLayout(pipelineEsmGenerateRef) =
          pipelineLayoutEsm;
      PipelineManager::_descVertexLayout(pipelineEsmGenerateRef) = Dod::Ref();
      PipelineManager::_descDepthStencilState(pipelineEsmGenerateRef) =
          DepthStencilStates::kDefaultNoDepthTestAndWrite;
      PipelineManager::_descViewportRenderSize(pipelineEsmGenerateRef) =
          (uint8_t)RenderSize::kCustom;
      PipelineManager::_descScissorRenderSize(pipelineEsmGenerateRef) =
          (uint8_t)RenderSize::kCustom;
      PipelineManager::_descAbsoluteViewportDimensions(pipelineEsmGenerateRef) =
          expShadowBufferDim;
      PipelineManager::_descAbsoluteScissorDimensions(pipelineEsmGenerateRef) =
          expShadowBufferDim;
    }
    pipelinesToCreate.push_back(pipelineEsmGenerateRef);

    pipelineEsmBlurRef = PipelineManager::createPipeline(_N(ESMBLur));
    {

      PipelineManager::resetToDefault(pipelineEsmBlurRef);

      PipelineManager::_descFragmentProgram(pipelineEsmBlurRef) =
          GpuProgramManager::getResourceByName("esm_blur.frag");
      PipelineManager::_descVertexProgram(pipelineEsmBlurRef) =
          GpuProgramManager::getResourceByName("fullscreen_triangle.vert");
      PipelineManager::_descRenderPass(pipelineEsmBlurRef) = _renderPassRef;
      PipelineManager::_descPipelineLayout(pipelineEsmBlurRef) =
          pipelineLayoutEsm;
      PipelineManager::_descVertexLayout(pipelineEsmBlurRef) = Dod::Ref();
      PipelineManager::_descDepthStencilState(pipelineEsmBlurRef) =
          DepthStencilStates::kDefaultNoDepthTestAndWrite;
      PipelineManager::_descViewportRenderSize(pipelineEsmBlurRef) =
          (uint8_t)RenderSize::kCustom;
      PipelineManager::_descScissorRenderSize(pipelineEsmBlurRef) =
          (uint8_t)RenderSize::kCustom;
      PipelineManager::_descAbsoluteViewportDimensions(pipelineEsmBlurRef) =
          expShadowBufferDim;
      PipelineManager::_descAbsoluteScissorDimensions(pipelineEsmBlurRef) =
          expShadowBufferDim;
    }
    pipelinesToCreate.push_back(pipelineEsmBlurRef);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  ImageRefArray imgsToCreate;
  ComputeCallRefArray computeCallsToCreate;

  const glm::uvec3 computeDim = glm::uvec3(160u, 90u, 128u);

  // Images
  {
    _shadowBufferExp = ImageManager::createImage(_N(ShadowBufferExp));
    {
      ImageManager::resetToDefault(_shadowBufferExp);
      ImageManager::addResourceFlags(
          _shadowBufferExp, Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_shadowBufferExp) =
          glm::uvec3(expShadowBufferDim, 1u);
      ImageManager::_descImageFormat(_shadowBufferExp) =
          Format::kR32G32B32A32SFloat;
      ImageManager::_descImageType(_shadowBufferExp) = ImageType::kTexture;
      ImageManager::_descArrayLayerCount(_shadowBufferExp) =
          _INTR_MAX_SHADOW_MAP_COUNT;
    }
    imgsToCreate.push_back(_shadowBufferExp);

    _shadowBufferExpPingPong =
        ImageManager::createImage(_N(ShadowBufferExpPingPong));
    {
      ImageManager::resetToDefault(_shadowBufferExpPingPong);
      ImageManager::addResourceFlags(
          _shadowBufferExpPingPong,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_shadowBufferExpPingPong) =
          glm::uvec3(expShadowBufferDim, 1u);
      ImageManager::_descImageFormat(_shadowBufferExpPingPong) =
          Format::kR32G32B32A32SFloat;
      ImageManager::_descImageType(_shadowBufferExpPingPong) =
          ImageType::kTexture;
      ImageManager::_descArrayLayerCount(_shadowBufferExpPingPong) =
          _INTR_MAX_SHADOW_MAP_COUNT;
    }
    imgsToCreate.push_back(_shadowBufferExpPingPong);

    _volLightingBufferImageRef =
        ImageManager::createImage(_N(VolumetricLightingBuffer));
    {
      ImageManager::resetToDefault(_volLightingBufferImageRef);
      ImageManager::addResourceFlags(
          _volLightingBufferImageRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_volLightingBufferImageRef) = computeDim;
      ImageManager::_descImageFormat(_volLightingBufferImageRef) =
          Format::kR16G16B16A16Float;
      ImageManager::_descImageType(_volLightingBufferImageRef) =
          ImageType::kTexture;
      ImageManager::_descImageFlags(_volLightingBufferImageRef) =
          ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
    }
    imgsToCreate.push_back(_volLightingBufferImageRef);

    _volLightingBufferPrevFrameImageRef =
        ImageManager::createImage(_N(VolumetricLightingBufferPrevFrame));
    {
      ImageManager::resetToDefault(_volLightingBufferPrevFrameImageRef);
      ImageManager::addResourceFlags(
          _volLightingBufferPrevFrameImageRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_volLightingBufferPrevFrameImageRef) =
          computeDim;
      ImageManager::_descImageFormat(_volLightingBufferPrevFrameImageRef) =
          Format::kR16G16B16A16Float;
      ImageManager::_descImageType(_volLightingBufferPrevFrameImageRef) =
          ImageType::kTexture;
      ImageManager::_descImageFlags(_volLightingBufferPrevFrameImageRef) =
          ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
    }
    imgsToCreate.push_back(_volLightingBufferPrevFrameImageRef);

    _volLightingScatteringBufferImageRef =
        ImageManager::createImage(_N(VolumetricLightingScatteringBuffer));
    {
      ImageManager::resetToDefault(_volLightingScatteringBufferImageRef);
      ImageManager::addResourceFlags(
          _volLightingScatteringBufferImageRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      ImageManager::_descDimensions(_volLightingScatteringBufferImageRef) =
          computeDim;
      ImageManager::_descImageFormat(_volLightingScatteringBufferImageRef) =
          Format::kR16G16B16A16Float;
      ImageManager::_descImageType(_volLightingScatteringBufferImageRef) =
          ImageType::kTexture;
      ImageManager::_descImageFlags(_volLightingScatteringBufferImageRef) =
          ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
    }
    imgsToCreate.push_back(_volLightingScatteringBufferImageRef);
  }
  ImageManager::createResources(imgsToCreate);

  // Draw calls
  {
    _drawCallEsmGenerateRef = DrawCallManager::createDrawCall(_N(ESMGenerate));
    {
      DrawCallManager::resetToDefault(_drawCallEsmGenerateRef);
      DrawCallManager::addResourceFlags(
          _drawCallEsmGenerateRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      DrawCallManager::_descPipeline(_drawCallEsmGenerateRef) =
          pipelineEsmGenerateRef;
      DrawCallManager::_descVertexCount(_drawCallEsmGenerateRef) = 3u;

      DrawCallManager::bindBuffer(
          _drawCallEsmGenerateRef, _N(PerInstance), GpuProgramType::kFragment,
          UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceFragment, sizeof(PerInstanceDataESMGenerate));
      DrawCallManager::bindImage(
          _drawCallEsmGenerateRef, _N(inputTex), GpuProgramType::kFragment,
          ImageManager::getResourceByName(_N(ShadowBuffer)),
          Samplers::kLinearClamp);

      drawCallsToCreate.push_back(_drawCallEsmGenerateRef);
    }

    _drawCallEsmBlurRef = DrawCallManager::createDrawCall(_N(ESMBlur));
    {
      DrawCallManager::resetToDefault(_drawCallEsmBlurRef);
      DrawCallManager::addResourceFlags(
          _drawCallEsmBlurRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      DrawCallManager::_descPipeline(_drawCallEsmBlurRef) = pipelineEsmBlurRef;
      DrawCallManager::_descVertexCount(_drawCallEsmBlurRef) = 3u;

      DrawCallManager::bindBuffer(
          _drawCallEsmBlurRef, _N(PerInstance), GpuProgramType::kFragment,
          UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceFragment, sizeof(PerInstanceDataESMGenerate));
      DrawCallManager::bindImage(_drawCallEsmBlurRef, _N(inputTex),
                                 GpuProgramType::kFragment, _shadowBufferExp,
                                 Samplers::kLinearClamp);

      drawCallsToCreate.push_back(_drawCallEsmBlurRef);
    }

    _drawCallEsmBlurPingPongRef =
        DrawCallManager::createDrawCall(_N(ESMBlurPingPong));
    {
      DrawCallManager::resetToDefault(_drawCallEsmBlurPingPongRef);
      DrawCallManager::addResourceFlags(
          _drawCallEsmBlurPingPongRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      DrawCallManager::_descPipeline(_drawCallEsmBlurPingPongRef) =
          pipelineEsmBlurRef;
      DrawCallManager::_descVertexCount(_drawCallEsmBlurPingPongRef) = 3u;

      DrawCallManager::bindBuffer(
          _drawCallEsmBlurPingPongRef, _N(PerInstance),
          GpuProgramType::kFragment, UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceFragment, sizeof(PerInstanceDataESMGenerate));
      DrawCallManager::bindImage(
          _drawCallEsmBlurPingPongRef, _N(inputTex), GpuProgramType::kFragment,
          _shadowBufferExpPingPong, Samplers::kLinearClamp);

      drawCallsToCreate.push_back(_drawCallEsmBlurPingPongRef);
    }
  }
  DrawCallManager::createResources(drawCallsToCreate);

  // Compute calls
  {
    BufferRef lightBuffer = BufferManager::getResourceByName(_N(LightBuffer));
    BufferRef lightIndexBuffer =
        BufferManager::getResourceByName(_N(LightIndexBuffer));
    BufferRef irradProbeBuffer =
        BufferManager::getResourceByName(_N(IrradProbeBuffer));
    BufferRef irradProbeIndexBuffer =
        BufferManager::getResourceByName(_N(IrradProbeIndexBuffer));

    // Accumulation
    _computeCallAccumRef = createComputeCallAccumulation(
        computeDim, lightBuffer, lightIndexBuffer, irradProbeBuffer,
        irradProbeIndexBuffer, _volLightingBufferImageRef,
        _volLightingBufferPrevFrameImageRef);

    _computeCallAccumPrevFrameRef = createComputeCallAccumulation(
        computeDim, lightBuffer, lightIndexBuffer, irradProbeBuffer,
        irradProbeIndexBuffer, _volLightingBufferPrevFrameImageRef,
        _volLightingBufferImageRef);

    computeCallsToCreate.push_back(_computeCallAccumRef);
    computeCallsToCreate.push_back(_computeCallAccumPrevFrameRef);

    // Scattering
    _computeCallScatteringRef =
        createComputeCallScattering(computeDim, _volLightingBufferImageRef);
    _computeCallScatteringPrevFrameRef = createComputeCallScattering(
        computeDim, _volLightingBufferPrevFrameImageRef);

    computeCallsToCreate.push_back(_computeCallScatteringRef);
    computeCallsToCreate.push_back(_computeCallScatteringPrevFrameRef);
  }
  ComputeCallManager::createResources(computeCallsToCreate);

  // Create framebuffers
  for (uint32_t shadowMapIdx = 0u; shadowMapIdx < _INTR_MAX_SHADOW_MAP_COUNT;
       ++shadowMapIdx)
  {
    FramebufferRef frameBufferRef =
        FramebufferManager::createFramebuffer(_N(ShadowESM));
    {
      FramebufferManager::resetToDefault(frameBufferRef);
      FramebufferManager::addResourceFlags(
          frameBufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

      FramebufferManager::_descAttachedImages(frameBufferRef)
          .push_back(AttachmentInfo(_shadowBufferExp, shadowMapIdx));
      FramebufferManager::_descDimensions(frameBufferRef) = expShadowBufferDim;
      FramebufferManager::_descRenderPass(frameBufferRef) = _renderPassRef;
    }
    _framebufferRefs.push_back(frameBufferRef);
  }

  for (uint32_t shadowMapIdx = 0u; shadowMapIdx < _INTR_MAX_SHADOW_MAP_COUNT;
       ++shadowMapIdx)
  {
    FramebufferRef frameBufferRef =
        FramebufferManager::createFramebuffer(_N(ShadowESMPingPong));
    {
      FramebufferManager::resetToDefault(frameBufferRef);
      FramebufferManager::addResourceFlags(
          frameBufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

      FramebufferManager::_descAttachedImages(frameBufferRef)
          .push_back(AttachmentInfo(_shadowBufferExpPingPong, shadowMapIdx));
      FramebufferManager::_descDimensions(frameBufferRef) = expShadowBufferDim;
      FramebufferManager::_descRenderPass(frameBufferRef) = _renderPassRef;
    }
    _framebufferPingPongRefs.push_back(frameBufferRef);
  }

  FramebufferManager::createResources(_framebufferRefs);
  FramebufferManager::createResources(_framebufferPingPongRefs);
}

// <-

void VolumetricLighting::onReinitRendering() {}

// <-

void VolumetricLighting::destroy() {}

// <-

void VolumetricLighting::render(float p_DeltaT,
                                Components::CameraRef p_CameraRef)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  const _INTR_ARRAY(Core::Resources::FrustumRef)& shadowFrustums =
      RenderProcess::Default::_shadowFrustums[p_CameraRef];

  const uint32_t shadowMapCount = (uint32_t)shadowFrustums.size();
  generateExponentialShadowMaps(shadowMapCount);
  blurExponentialShadowMaps(shadowMapCount);

  ComputeCallRef accumComputeCallRefToUse = _computeCallAccumRef;
  if ((TaskManager::_frameCounter % 2u) != 0u)
  {
    accumComputeCallRefToUse = _computeCallAccumPrevFrameRef;

    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferPrevFrameImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  }
  else
  {
    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferPrevFrameImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  }

  {
    // Update per instance data
    updatePerInstanceData(p_CameraRef, accumComputeCallRefToUse);
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  {
    RenderSystem::dispatchComputeCall(accumComputeCallRefToUse,
                                      primaryCmdBuffer);
  }

  ComputeCallRef scatteringComputeCalltoUse = _computeCallScatteringRef;
  if ((TaskManager::_frameCounter % 2u) != 0u)
  {
    scatteringComputeCalltoUse = _computeCallScatteringPrevFrameRef;

    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferPrevFrameImageRef, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  }
  else
  {
    ImageManager::insertImageMemoryBarrier(
        _volLightingBufferImageRef, VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  }

  ImageManager::insertImageMemoryBarrier(
      _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  {
    RenderSystem::dispatchComputeCall(scatteringComputeCalltoUse,
                                      primaryCmdBuffer);
  }
  ImageManager::insertImageMemoryBarrier(
      _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
}
}
}
}
