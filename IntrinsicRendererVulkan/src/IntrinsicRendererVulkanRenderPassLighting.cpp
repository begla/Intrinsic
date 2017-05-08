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

#define MAX_LIGHT_COUNT_PER_CLUSTER 128u
#define GRID_DEPTH_SLICE_COUNT 24u

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
Resources::BufferRef _lightBuffer;
Resources::BufferRef _lightIndexBuffer;

struct Light
{
  glm::vec4 posAndRadius;
  glm::vec4 color;
};

uint32_t* _lightIndexBufferGpuMemory = nullptr;
Light* _lightBufferGpuMemory = nullptr;
Light* _lightBufferMemory = nullptr;

// <-

struct PerInstanceData
{
  glm::mat4 viewMatrix;
  glm::mat4 invProjectionMatrix;
  glm::mat4 invViewMatrix;

  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];

  glm::vec4 nearFarWidthHeight;
  glm::vec4 nearFar;
} _perInstanceData;

// <-

// Have to match the values in the shader
const float _gridDepth = 10000.0f;
const glm::uvec3 _gridRes = glm::uvec3(16u, 8u, GRID_DEPTH_SLICE_COUNT);
const float _gridDepthSliceScale =
    _gridDepth * (1.0f / ((_gridRes.z - 1u) * (_gridRes.z - 1u)));
const float _gridDepthSliceScaleRcp = 1.0f / _gridDepthSliceScale;

const uint32_t _totalClusterCount = _gridRes.x * _gridRes.y * _gridRes.z;
const uint32_t _totalGridSize =
    _totalClusterCount * MAX_LIGHT_COUNT_PER_CLUSTER;

// <-

_INTR_INLINE uint32_t calcClusterIndex(glm::uvec3 p_GridPos)
{
  return p_GridPos.x * MAX_LIGHT_COUNT_PER_CLUSTER +
         p_GridPos.y * _gridRes.x * MAX_LIGHT_COUNT_PER_CLUSTER +
         p_GridPos.z * _gridRes.y * _gridRes.x * MAX_LIGHT_COUNT_PER_CLUSTER;
}

_INTR_INLINE float calcGridDepthSlice(uint32_t p_DepthSliceIdx)
{
  return (p_DepthSliceIdx * p_DepthSliceIdx) * _gridDepthSliceScale;
}

_INTR_INLINE Math::AABB2 calcAABBForDepthSlice(uint32_t p_DepthSlice,
                                               glm::vec4 p_NearFarWidthHeight,
                                               glm::vec4 p_NearFar)
{
  const float gridStartDepth = calcGridDepthSlice(p_DepthSlice);
  const float gridEndDepth = calcGridDepthSlice(p_DepthSlice + 1u);

  const float gridDepth = gridEndDepth - gridStartDepth;
  const float gridHalfDepth = gridDepth * 0.5f;

  const float rayPos =
      (gridEndDepth - p_NearFar.x) / (p_NearFar.y - p_NearFar.x);

  const glm::vec2 gridHalfWidthHeight =
      glm::mix(glm::vec2(p_NearFarWidthHeight.x, p_NearFarWidthHeight.y),
               glm::vec2(p_NearFarWidthHeight.z, p_NearFarWidthHeight.w),
               rayPos) *
      0.5f;

  const glm::vec3 gridCenter =
      glm::vec3(0.0f, 0.0f, -gridStartDepth - gridHalfDepth);
  const glm::vec3 gridHalfExtent =
      glm::vec3(gridHalfWidthHeight.x, gridHalfWidthHeight.y, gridHalfDepth);

  return {gridCenter, gridHalfExtent};
}

_INTR_INLINE Math::AABB2 calcAABBForGridPos(glm::uvec3 p_GridPos,
                                            glm::vec4 p_NearFarWidthHeight,
                                            glm::vec4 p_NearFar)
{
  const float gridStartDepth = calcGridDepthSlice(p_GridPos.z);
  const float gridEndDepth = calcGridDepthSlice(p_GridPos.z + 1u);

  const float gridDepth = gridEndDepth - gridStartDepth;
  const float gridHalfDepth = gridDepth * 0.5f;

  const float rayPos =
      (gridEndDepth - p_NearFar.x) / (p_NearFar.y - p_NearFar.x);

  const glm::vec2 gridHalfWidthHeight =
      glm::mix(glm::vec2(p_NearFarWidthHeight.x, p_NearFarWidthHeight.y),
               glm::vec2(p_NearFarWidthHeight.z, p_NearFarWidthHeight.w),
               rayPos) *
      0.5f;

  const glm::vec3 gridCenter = glm::vec3(0.0f, 0.0f, gridStartDepth);
  const glm::vec3 gridHalfExtent =
      glm::vec3(gridHalfWidthHeight.x, gridHalfWidthHeight.y, gridHalfDepth);
  const glm::vec3 gridDimWithoutZ = glm::vec3(_gridRes.x, _gridRes.y, 1.0);

  const glm::vec3 aabbHalfExtent = gridHalfExtent / gridDimWithoutZ;
  glm::vec3 aabbCenter =
      gridCenter + aabbHalfExtent +
      (glm::vec3(p_GridPos) - gridDimWithoutZ * glm::vec3(0.5, 0.5, 0.0)) *
          aabbHalfExtent * glm::vec3(2.0, 2.0, 0.0);
  aabbCenter.z = -aabbCenter.z;

  return {aabbCenter, aabbHalfExtent};
}

struct LightCullingParallelTaskSet : enki::ITaskSet
{
  virtual ~LightCullingParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Lighting", "Cull Lights For Depth Slice");

    for (uint32_t y = 0u; y < _gridRes.y; ++y)
    {
      for (uint32_t x = 0u; x < _gridRes.x; ++x)
      {
        const glm::uvec3 gridPos = glm::uvec3(x, y, _z);

        const Math::AABB2 clusterAABB =
            calcAABBForGridPos(gridPos, _perInstanceData.nearFarWidthHeight,
                               _perInstanceData.nearFar);

        const uint32_t clusterIndex = calcClusterIndex(gridPos);
        uint32_t& lightCount = _lightIndexBufferGpuMemory[clusterIndex];
        lightCount = 0u;

        for (uint32_t i = 0u; i < _availableLights.size(); ++i)
        {
          uint32_t lidx = _availableLights[i];
          const Light& light = _lightBufferMemory[lidx];
          if (Math::calcIntersectSphereAABB(
                  {glm::vec3(light.posAndRadius), light.posAndRadius.w},
                  clusterAABB))
          {
            const uint32_t clusterLightIdx = clusterIndex + lightCount + 1u;
            _lightIndexBufferGpuMemory[clusterLightIdx] = lidx;
            ++lightCount;
          }
        }
      }
    }
  }

  uint32_t _z;
  _INTR_ARRAY(uint32_t) _availableLights;
} _cullingTaskSets[GRID_DEPTH_SLICE_COUNT];

_INTR_INLINE void cullLightsAndWriteBuffers(Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Lighting", "Cull Lights And Write Buffers");

  {
    _INTR_PROFILE_CPU("Lighting", "Write Light Buffer");

    for (uint32_t i = 0u; i < Components::LightManager::_activeRefs.size(); ++i)
    {
      Components::LightRef lightRef = Components::LightManager::_activeRefs[i];
      Components::NodeRef lightNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::LightManager::_entity(lightRef));

      const glm::vec3 lightPosVS =
          Components::CameraManager::_viewMatrix(p_CameraRef) *
          glm::vec4(Components::NodeManager::_worldPosition(lightNodeRef), 1.0);
      _lightBufferMemory[i] = {
          glm::vec4(lightPosVS,
                    Components::LightManager::_descRadius(lightRef)),
          Components::LightManager::_descIntensity(lightRef) *
              glm::vec4(Components::LightManager::_descColor(lightRef), 0.0f)};
    }
  }

  // Find lights intersecting the depth slices and kick jobs for populated
  // ones
  _INTR_ARRAY(uint32_t) _activeTaskSets;
  _activeTaskSets.reserve(GRID_DEPTH_SLICE_COUNT);
  {
    _INTR_PROFILE_CPU("Lighting", "Find Slice And Kick Jobs");

    for (uint32_t z = 0u; z < _gridRes.z; ++z)
    {
      LightCullingParallelTaskSet& taskSet = _cullingTaskSets[z];
      taskSet._z = z;
      taskSet._availableLights.clear();

      const Math::AABB2 depthSliceAABB = calcAABBForDepthSlice(
          z, _perInstanceData.nearFarWidthHeight, _perInstanceData.nearFar);

      for (uint32_t i = 0u; i < Components::LightManager::_activeRefs.size();
           ++i)
      {
        const Light& light = _lightBufferMemory[i];
        if (Math::calcIntersectSphereAABB(
                {glm::vec3(light.posAndRadius), light.posAndRadius.w},
                depthSliceAABB))
        {
          taskSet._availableLights.push_back(i);
        }
      }

      if (!taskSet._availableLights.empty())
      {
        Application::_scheduler.AddTaskSetToPipe(&taskSet);
        _activeTaskSets.push_back(z);
      }
    }
  }

  memcpy(_lightBufferGpuMemory, _lightBufferMemory,
         Components::LightManager::_activeRefs.size() * sizeof(Light));

  {
    _INTR_PROFILE_CPU("Lighting", "Wait For Jobs");

    for (uint32_t i = 0u; i < _activeTaskSets.size(); ++i)
    {
      LightCullingParallelTaskSet& taskSet =
          _cullingTaskSets[_activeTaskSets[i]];
      Application::_scheduler.WaitforTaskSet(&taskSet);
    }
  }
}

// <--

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

  _INTR_ARRAY(Resources::BufferRef) buffersToCreate;

  // Buffers
  {
    _lightBuffer = BufferManager::createBuffer(_N(LightBuffer));
    {
      BufferManager::resetToDefault(_lightBuffer);
      BufferManager::addResourceFlags(
          _lightBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_lightBuffer) = BufferType::kStorage;
      BufferManager::_descMemoryPoolType(_lightBuffer) =
          MemoryPoolType::kStaticStagingBuffers;
      BufferManager::_descSizeInBytes(_lightBuffer) =
          MAX_LIGHT_COUNT_PER_CLUSTER * _gridRes.x * _gridRes.y * _gridRes.z *
          sizeof(Light);
      buffersToCreate.push_back(_lightBuffer);
    }

    _lightIndexBuffer = BufferManager::createBuffer(_N(LightIndexBuffer));
    {
      BufferManager::resetToDefault(_lightIndexBuffer);
      BufferManager::addResourceFlags(
          _lightIndexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_lightIndexBuffer) = BufferType::kStorage;
      BufferManager::_descMemoryPoolType(_lightIndexBuffer) =
          MemoryPoolType::kStaticStagingBuffers;
      BufferManager::_descSizeInBytes(_lightIndexBuffer) =
          _totalGridSize * sizeof(uint32_t);
      buffersToCreate.push_back(_lightIndexBuffer);
    }
  }

  Resources::BufferManager::createResources(buffersToCreate);

  _lightBufferGpuMemory =
      (Light*)Resources::BufferManager::getGpuMemory(_lightBuffer);
  _lightIndexBufferGpuMemory =
      (uint32_t*)Resources::BufferManager::getGpuMemory(_lightIndexBuffer);

  _lightBufferMemory = (Light*)malloc(MAX_LIGHT_COUNT_PER_CLUSTER * _gridRes.x *
                                      _gridRes.y * _gridRes.z * sizeof(Light));
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
    DrawCallManager::bindImage(
        _drawCallRef, _N(ssaoTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(SSAO)), Samplers::kLinearClamp);
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
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(LightBuffer), GpuProgramType::kFragment, _lightBuffer,
        UboType::kInvalidUbo, BufferManager::_descSizeInBytes(_lightBuffer));
    DrawCallManager::bindBuffer(
        _drawCallRef, _N(LightIndexBuffer), GpuProgramType::kFragment,
        _lightIndexBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(_lightIndexBuffer));
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
    DrawCallManager::bindImage(
        _drawCallTransparentsRef, _N(ssaoTex), GpuProgramType::kFragment,
        ImageManager::getResourceByName(_N(SSAO)), Samplers::kLinearClamp);
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
    DrawCallManager::bindBuffer(_drawCallTransparentsRef, _N(LightBuffer),
                                GpuProgramType::kFragment, _lightBuffer,
                                UboType::kInvalidUbo,
                                BufferManager::_descSizeInBytes(_lightBuffer));
    DrawCallManager::bindBuffer(
        _drawCallTransparentsRef, _N(LightIndexBuffer),
        GpuProgramType::kFragment, _lightIndexBuffer, UboType::kInvalidUbo,
        BufferManager::_descSizeInBytes(_lightIndexBuffer));
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

  // Update global per instance data
  {
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
  }

  cullLightsAndWriteBuffers(p_CameraRef);

  renderLighting(_framebufferRef, _drawCallRef, _lightingBufferImageRef,
                 p_CameraRef);
  renderLighting(_framebufferTransparentsRef, _drawCallTransparentsRef,
                 _lightingBufferTransparentsImageRef, p_CameraRef);
}
}
}
}
}
