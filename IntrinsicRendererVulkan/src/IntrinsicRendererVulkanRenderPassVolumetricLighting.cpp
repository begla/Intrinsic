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
struct PerInstanceData
{
  glm::mat4 projMatrix;
  glm::mat4 viewProjMatrix;
  glm::mat4 prevViewProjMatrix;
  glm::mat4 viewMatrix;
  glm::mat4 invViewMatrix;

  glm::vec4 eyeVSVectorX;
  glm::vec4 eyeVSVectorY;
  glm::vec4 eyeVSVectorZ;

  glm::vec4 eyeWSVectorX;
  glm::vec4 eyeWSVectorY;
  glm::vec4 eyeWSVectorZ;

  glm::vec4 data0;

  glm::vec4 camPos;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];
} _perInstanceData;

Resources::ImageRef _volLightingBufferImageRef;
Resources::ImageRef _volLightingBufferPrevFrameImageRef;
Resources::ImageRef _volLightingScatteringBufferImageRef;
Resources::PipelineRef _pipelineRef;
Resources::PipelineRef _pipelineScatteringRef;
Resources::ComputeCallRef _computeCallRef;
Resources::ComputeCallRef _computeCallPrevFrameRef;
Resources::ComputeCallRef _computeCallScatteringRef;
}

void VolumetricLighting::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Pipeline layouts
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout =
        PipelineLayoutManager::createPipelineLayout(_N(VolumetricLighting));
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u, {Resources::GpuProgramManager::getResourceByName(
                "volumetric_lighting.comp")},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  PipelineLayoutRef pipelineLayoutScattering;
  {
    pipelineLayoutScattering = PipelineLayoutManager::createPipelineLayout(
        _N(VolumetricLightingScattering));
    PipelineLayoutManager::resetToDefault(pipelineLayoutScattering);

    GpuProgramManager::reflectPipelineLayout(
        8u, {Resources::GpuProgramManager::getResourceByName(
                "volumetric_lighting_scattering.comp")},
        pipelineLayoutScattering);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayoutScattering);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_N(VolumetricLighting));
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descComputeProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName("volumetric_lighting.comp");
    PipelineManager::_descPipelineLayout(_pipelineRef) = pipelineLayout;
  }
  pipelinesToCreate.push_back(_pipelineRef);

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

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);
}

// <-

void VolumetricLighting::updateResolutionDependentResources()
{
  using namespace Resources;

  ImageRefArray imgsToDestroy;
  ImageRefArray imgsToCreate;
  ComputeCallRefArray computeCallsToDestroy;
  ComputeCallRefArray computeCallsToCreate;

  // Cleanup old resources
  {
    if (_computeCallRef.isValid())
      computeCallsToDestroy.push_back(_computeCallRef);
    if (_computeCallPrevFrameRef.isValid())
      computeCallsToDestroy.push_back(_computeCallPrevFrameRef);
    if (_computeCallScatteringRef.isValid())
      computeCallsToDestroy.push_back(_computeCallScatteringRef);

    if (_volLightingBufferImageRef.isValid())
      imgsToDestroy.push_back(_volLightingBufferImageRef);
    if (_volLightingBufferPrevFrameImageRef.isValid())
      imgsToDestroy.push_back(_volLightingBufferPrevFrameImageRef);
    if (_volLightingScatteringBufferImageRef.isValid())
      imgsToDestroy.push_back(_volLightingScatteringBufferImageRef);

    ComputeCallManager::destroyComputeCallsAndResources(computeCallsToDestroy);
    ImageManager::destroyImagesAndResources(imgsToDestroy);
  }

  glm::uvec3 dim = glm::uvec3(160u, 90u, 128u);

  _volLightingBufferImageRef =
      ImageManager::createImage(_N(VolumetricLightingBuffer));
  {
    ImageManager::resetToDefault(_volLightingBufferImageRef);
    ImageManager::addResourceFlags(
        _volLightingBufferImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ImageManager::_descDimensions(_volLightingBufferImageRef) = dim;
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

    ImageManager::_descDimensions(_volLightingBufferPrevFrameImageRef) = dim;
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

    ImageManager::_descDimensions(_volLightingScatteringBufferImageRef) = dim;
    ImageManager::_descImageFormat(_volLightingScatteringBufferImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_volLightingScatteringBufferImageRef) =
        ImageType::kTexture;
    ImageManager::_descImageFlags(_volLightingScatteringBufferImageRef) =
        ImageFlags::kUsageSampled | ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_volLightingScatteringBufferImageRef);

  ImageManager::createResources(imgsToCreate);

  // Compute calls
  _computeCallRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLighting));
  {
    ComputeCallManager::resetToDefault(_computeCallRef);
    ComputeCallManager::addResourceFlags(
        _computeCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(_computeCallRef) = glm::uvec3(
        Math::divideByMultiple(dim.x, 4u), Math::divideByMultiple(dim.y, 4u),
        Math::divideByMultiple(dim.z, 4u));
    ComputeCallManager::_descPipeline(_computeCallRef) = _pipelineRef;

    ComputeCallManager::bindBuffer(
        _computeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        _computeCallRef, _N(shadowBufferTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
    ComputeCallManager::bindImage(
        _computeCallRef, _N(output0Tex), GpuProgramType::kCompute,
        _volLightingBufferImageRef, Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(
        _computeCallRef, _N(prevVolLightBufferTex), GpuProgramType::kCompute,
        _volLightingBufferPrevFrameImageRef, Samplers::kLinearClamp);
  }

  computeCallsToCreate.push_back(_computeCallRef);

  _computeCallPrevFrameRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLightingPrevFrame));
  {
    ComputeCallManager::resetToDefault(_computeCallPrevFrameRef);
    ComputeCallManager::addResourceFlags(
        _computeCallPrevFrameRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(_computeCallPrevFrameRef) = glm::uvec3(
        Math::divideByMultiple(dim.x, 4u), Math::divideByMultiple(dim.y, 4u),
        Math::divideByMultiple(dim.z, 4u));
    ComputeCallManager::_descPipeline(_computeCallPrevFrameRef) = _pipelineRef;

    ComputeCallManager::bindBuffer(
        _computeCallPrevFrameRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        _computeCallPrevFrameRef, _N(shadowBufferTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
    ComputeCallManager::bindImage(
        _computeCallPrevFrameRef, _N(output0Tex), GpuProgramType::kCompute,
        _volLightingBufferPrevFrameImageRef, Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(
        _computeCallPrevFrameRef, _N(prevVolLightBufferTex),
        GpuProgramType::kCompute, _volLightingBufferImageRef,
        Samplers::kLinearClamp);
  }

  computeCallsToCreate.push_back(_computeCallPrevFrameRef);

  _computeCallScatteringRef =
      ComputeCallManager::createComputeCall(_N(VolumetricLighting));
  {
    ComputeCallManager::resetToDefault(_computeCallScatteringRef);
    ComputeCallManager::addResourceFlags(
        _computeCallScatteringRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ComputeCallManager::_descDimensions(_computeCallScatteringRef) =
        glm::uvec3(Math::divideByMultiple(dim.x, 8u),
                   Math::divideByMultiple(dim.y, 8u), 1u);
    ComputeCallManager::_descPipeline(_computeCallScatteringRef) =
        _pipelineScatteringRef;

    ComputeCallManager::bindBuffer(
        _computeCallScatteringRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceData));
    ComputeCallManager::bindImage(
        _computeCallScatteringRef, _N(volLightBufferTex),
        GpuProgramType::kCompute, _volLightingBufferImageRef,
        Samplers::kLinearClamp);
    ComputeCallManager::bindImage(
        _computeCallScatteringRef, _N(volLightScatterBufferTex),
        GpuProgramType::kCompute, _volLightingScatteringBufferImageRef,
        Samplers::kInvalidSampler);
  }

  computeCallsToCreate.push_back(_computeCallScatteringRef);

  ComputeCallManager::createResources(computeCallsToCreate);
}

// <-

void VolumetricLighting::destroy() {}

// <-

void VolumetricLighting::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Volumetric Lighting");
  _INTR_PROFILE_GPU("Render Volumetric Lighting");

  ComputeCallRef accumComputeCallRefToUse = _computeCallRef;
  if ((TaskManager::_frameCounter % 2u) != 0u)
  {
    accumComputeCallRefToUse = _computeCallPrevFrameRef;
  }

  // Update per instance data
  {
    Components::CameraRef activeCamRef = World::getActiveCamera();
    Components::NodeRef camNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamRef));

    _perInstanceData.prevViewProjMatrix = _perInstanceData.viewProjMatrix;
    _perInstanceData.viewProjMatrix =
        Components::CameraManager::_viewProjectionMatrix(activeCamRef);
    _perInstanceData.projMatrix =
        Components::CameraManager::_projectionMatrix(activeCamRef);
    _perInstanceData.viewMatrix =
        Components::CameraManager::_viewMatrix(activeCamRef);
    _perInstanceData.invViewMatrix =
        Components::CameraManager::_inverseViewMatrix(activeCamRef);

    _perInstanceData.camPos =
        glm::vec4(Components::NodeManager::_worldPosition(camNodeRef),
                  TaskManager::_frameCounter);

    _perInstanceData.eyeVSVectorX = glm::vec4(
        glm::vec3(1.0 / _perInstanceData.projMatrix[0][0], 0.0, 0.0), 0.0);
    _perInstanceData.eyeVSVectorY = glm::vec4(
        glm::vec3(0.0, 1.0 / _perInstanceData.projMatrix[1][1], 0.0), 0.0);
    _perInstanceData.eyeVSVectorZ = glm::vec4(glm::vec3(0.0, 0.0, -1.0), 0.0);

    _perInstanceData.eyeWSVectorX =
        _perInstanceData.invViewMatrix * _perInstanceData.eyeVSVectorX;
    _perInstanceData.eyeWSVectorY =
        _perInstanceData.invViewMatrix * _perInstanceData.eyeVSVectorY;
    _perInstanceData.eyeWSVectorZ =
        _perInstanceData.invViewMatrix * _perInstanceData.eyeVSVectorZ;

    _perInstanceData.eyeWSVectorX.w = TaskManager::_totalTimePassed;

    // Post effect data
    _perInstanceData.data0.x =
        Core::Resources::PostEffectManager::_descVolumetricLightingScattering(
            Core::Resources::PostEffectManager::_blendTargetRef);

    for (uint32_t i = 0u; i < RenderPass::Shadow::_shadowFrustums.size(); ++i)
    {
      Core::Resources::FrustumRef shadowFrustumRef =
          RenderPass::Shadow::_shadowFrustums[i];

      // Transform from camera view space => light proj. space
      _perInstanceData.shadowViewProjMatrix[i] =
          Core::Resources::FrustumManager::_viewProjectionMatrix(
              shadowFrustumRef) *
          Components::CameraManager::_inverseViewMatrix(activeCamRef);
    }

    ComputeCallRefArray computeCallsToUpdate = {accumComputeCallRefToUse};
    ComputeCallManager::updateUniformMemory(
        computeCallsToUpdate, &_perInstanceData, sizeof(PerInstanceData));
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ImageManager::insertImageMemoryBarrier(
      _volLightingBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  {
    RenderSystem::dispatchComputeCall(accumComputeCallRefToUse,
                                      primaryCmdBuffer);
  }
  ImageManager::insertImageMemoryBarrier(
      _volLightingBufferImageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  ImageManager::insertImageMemoryBarrier(
      _volLightingScatteringBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  {
    RenderSystem::dispatchComputeCall(_computeCallScatteringRef,
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
