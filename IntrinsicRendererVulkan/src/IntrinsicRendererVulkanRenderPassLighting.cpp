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

#define MAX_LIGHT_COUNT_PER_CLUSTER 256u
#define MAX_IRRAD_PROBES_PER_CLUSTER 4u
#define GRID_DEPTH_SLICE_COUNT 24u
#define GRID_SIZE_Y 8u

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
Resources::BufferRef _irradProbeBuffer;
Resources::BufferRef _irradProbeIndexBuffer;

struct Light
{
  glm::vec4 posAndRadius;
  glm::vec4 colorAndIntensity;
  glm::vec4 temp;
};

struct TestLight
{
  glm::vec3 spawnPos;
  Light light;
};

struct IrradProbe
{
  glm::vec4 posAndRadius;
  glm::vec4 data0;
  glm::vec4 data[7]; // Packed SH coefficients
};

_INTR_ARRAY(TestLight) _testLights;

uint32_t _currentLightCount = 0u;
uint32_t _currentIrradProbeCount = 0u;

uint32_t* _lightIndexBufferGpuMemory = nullptr;
Light* _lightBufferGpuMemory = nullptr;
Light* _lightBufferMemory = nullptr;
uint32_t* _irradProbeIndexBufferGpuMemory = nullptr;
IrradProbe* _irradProbeBufferGpuMemory = nullptr;
IrradProbe* _irradProbeBufferMemory = nullptr;

// <-

struct PerInstanceData
{
  glm::mat4 shadowViewProjMatrix[_INTR_MAX_SHADOW_MAP_COUNT];

  glm::vec4 nearFarWidthHeight;
  glm::vec4 nearFar;

  glm::vec4 data0;
} _perInstanceData;

// <-

// Have to match the values in the shader
const float _gridDepth = 10000.0f;
const glm::uvec3 _gridRes =
    glm::uvec3(16u, GRID_SIZE_Y, GRID_DEPTH_SLICE_COUNT);
const float _gridDepthExp = 3.0f;
const float _gridDepthSliceScale =
    _gridDepth / glm::pow((float)(_gridRes.z - 1u), _gridDepthExp);
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
  return glm::pow((float)p_DepthSliceIdx, _gridDepthExp) * _gridDepthSliceScale;
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
    _INTR_PROFILE_CPU("Lighting", "Cull Lights And Probes For Depth Slice");

    for (uint32_t y = p_Range.start; y < p_Range.end; ++y)
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

        uint32_t& irradProbeCount =
            _irradProbeIndexBufferGpuMemory[clusterIndex];
        irradProbeCount = 0u;

        for (uint32_t i = 0u; i < _availableIrradProbes.size(); ++i)
        {
          uint32_t lidx = _availableIrradProbes[i];
          const IrradProbe& probe = _irradProbeBufferMemory[lidx];
          if (Math::calcIntersectSphereAABB(
                  {glm::vec3(probe.posAndRadius), probe.posAndRadius.w},
                  clusterAABB))
          {
            const uint32_t irradProbeIdx = clusterIndex + irradProbeCount + 1u;
            _irradProbeIndexBufferGpuMemory[irradProbeIdx] = lidx;
            ++irradProbeCount;
          }
        }
      }
    }
  }

  uint32_t _z;
  _INTR_ARRAY(uint32_t) _availableLights;
  _INTR_ARRAY(uint32_t) _availableIrradProbes;
} _cullingTaskSets[GRID_DEPTH_SLICE_COUNT];

_INTR_INLINE void cullLightsAndWriteBuffers(Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Lighting", "Cull Lights And Write Buffers");

  // TODO: Add coarse frustum culling pre-pass
  {
    _INTR_PROFILE_CPU("Lighting", "Write Light And Irrad Buffer");

    _currentLightCount = 0u;
    for (uint32_t i = 0u; i < Components::LightManager::_activeRefs.size(); ++i)
    {
      Components::LightRef lightRef = Components::LightManager::_activeRefs[i];
      Components::NodeRef lightNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::LightManager::_entity(lightRef));

      const glm::vec3 lightPosVS =
          Components::CameraManager::_viewMatrix(p_CameraRef) *
          glm::vec4(Components::NodeManager::_worldPosition(lightNodeRef), 1.0);
      _lightBufferMemory[_currentLightCount] = {
          glm::vec4(lightPosVS,
                    Components::LightManager::_descRadius(lightRef)),
          glm::vec4(Components::LightManager::_descColor(lightRef),
                    Components::LightManager::_descIntensity(lightRef)),
          glm::vec4(Components::LightManager::_descTemperature(lightRef))};
      ++_currentLightCount;
    }

    // Write test lights
    for (uint32_t i = 0u; i < _testLights.size(); ++i)
    {
      _lightBufferMemory[_currentLightCount] = _testLights[i].light;
      ++_currentLightCount;
    }

    // Sort probes by priority
    // TODO: Could be done once if a priority changes
    _currentIrradProbeCount = 0u;
    Components::IrradianceProbeManager::sortByPriority(
        Components::IrradianceProbeManager::_activeRefs);

    for (uint32_t i = 0u;
         i < Components::IrradianceProbeManager::_activeRefs.size(); ++i)
    {
      Components::IrradianceProbeRef irradProbeRef =
          Components::IrradianceProbeManager::_activeRefs[i];
      Components::NodeRef irradNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::IrradianceProbeManager::_entity(irradProbeRef));

      const glm::vec3 irradProbePosVS =
          Components::CameraManager::_viewMatrix(p_CameraRef) *
          glm::vec4(Components::NodeManager::_worldPosition(irradNodeRef), 1.0);
      _irradProbeBufferMemory[_currentIrradProbeCount].posAndRadius = glm::vec4(
          irradProbePosVS,
          Components::IrradianceProbeManager::_descRadius(irradProbeRef));
      _irradProbeBufferMemory[_currentIrradProbeCount].data0 = glm::vec4(
          Components::IrradianceProbeManager::_descFalloffRangePerc(
              irradProbeRef),
          Components::IrradianceProbeManager::_descFalloffExp(irradProbeRef),
          0.0f, 0.0f);

      const _INTR_ARRAY(Irradiance::SH9)& shs =
          Components::IrradianceProbeManager::_descSHs(irradProbeRef);

      // Blend SHs according to the time of day
      Irradiance::SH9 blendedSH;
      if (!shs.empty())
      {
        if (shs.size() >= 2u)
        {
          const uint32_t leftIdx =
              std::min((uint32_t)(World::_currentTime * shs.size()),
                       (uint32_t)shs.size() - 2u);

          const float leftPerc = leftIdx / (float)shs.size();
          const float rightPerc = (leftIdx + 1u) / (float)shs.size();

          const float interp =
              (World::_currentTime - leftPerc) / (rightPerc - leftPerc);

          const Irradiance::SH9& left = shs[leftIdx];
          const Irradiance::SH9& right = shs[leftIdx + 1u];

          blendedSH = Irradiance::blend(left, right, interp);
        }
        else
        {
          blendedSH = shs[0];
        }
      }

      memcpy(_irradProbeBufferMemory[_currentIrradProbeCount].data, &blendedSH,
             sizeof(Irradiance::SH9));
      ++_currentIrradProbeCount;
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
      taskSet.m_SetSize = GRID_SIZE_Y;
      taskSet._availableLights.clear();
      taskSet._availableIrradProbes.clear();

      const Math::AABB2 depthSliceAABB = calcAABBForDepthSlice(
          z, _perInstanceData.nearFarWidthHeight, _perInstanceData.nearFar);

      for (uint32_t i = 0u; i < _currentLightCount; ++i)
      {
        const Light& light = _lightBufferMemory[i];
        if (Math::calcIntersectSphereAABB(
                {glm::vec3(light.posAndRadius), light.posAndRadius.w},
                depthSliceAABB))
        {
          taskSet._availableLights.push_back(i);
        }
      }

      for (uint32_t i = 0u; i < _currentIrradProbeCount; ++i)
      {
        const IrradProbe& irradProbe = _irradProbeBufferMemory[i];
        if (Math::calcIntersectSphereAABB(
                {glm::vec3(irradProbe.posAndRadius), irradProbe.posAndRadius.w},
                depthSliceAABB))
        {
          taskSet._availableIrradProbes.push_back(i);
        }
      }

      if (!taskSet._availableLights.empty() ||
          !taskSet._availableIrradProbes.empty())
      {
        Application::_scheduler.AddTaskSetToPipe(&taskSet);
        _activeTaskSets.push_back(z);
      }
      else
      {
        // Reset counts
        // TODO: Suboptimal
        for (uint32_t y = 0u; y < _gridRes.y; ++y)
        {
          for (uint32_t x = 0u; x < _gridRes.x; ++x)
          {
            const glm::uint32_t clusterIdx =
                calcClusterIndex(glm::uvec3(x, y, z));
            _lightIndexBufferGpuMemory[clusterIdx] = 0u;
            _irradProbeIndexBufferGpuMemory[clusterIdx] = 0u;
          }
        }
      }
    }
  }

  memcpy(_lightBufferGpuMemory, _lightBufferMemory,
         _currentLightCount * sizeof(Light));
  memcpy(_irradProbeBufferGpuMemory, _irradProbeBufferMemory,
         _currentIrradProbeCount * sizeof(IrradProbe));

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

// <-

void renderLighting(Resources::FramebufferRef p_FramebufferRef,
                    Resources::DrawCallRef p_DrawCall,
                    Resources::ImageRef p_LightingBufferRef,
                    Components::CameraRef p_CameraRef)
{
  using namespace Resources;

  // Update per instance data
  {
    // Post effect data
    _perInstanceData.data0.x = TaskManager::_totalTimePassed;
    _perInstanceData.data0.y = Lighting::_globalAmbientFactor;
    _perInstanceData.data0.z = World::_currentDayNightFactor;

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

// <-

float Lighting::_globalAmbientFactor = 1.0f;

// <-

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

    AttachmentDescription colorAttachment = {Format::kB10G11R11UFloat, 0u};

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
          _INTR_MAX_LIGHT_COMPONENT_COUNT * sizeof(Light);
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

    _irradProbeBuffer = BufferManager::createBuffer(_N(IrradProbeBuffer));
    {
      BufferManager::resetToDefault(_irradProbeBuffer);
      BufferManager::addResourceFlags(
          _irradProbeBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_irradProbeBuffer) = BufferType::kStorage;
      BufferManager::_descMemoryPoolType(_irradProbeBuffer) =
          MemoryPoolType::kStaticStagingBuffers;
      BufferManager::_descSizeInBytes(_irradProbeBuffer) =
          MAX_IRRAD_PROBES_PER_CLUSTER * _gridRes.x * _gridRes.y * _gridRes.z *
          sizeof(Light);
      buffersToCreate.push_back(_irradProbeBuffer);
    }

    _irradProbeIndexBuffer =
        BufferManager::createBuffer(_N(IrradProbeIndexBuffer));
    {
      BufferManager::resetToDefault(_irradProbeIndexBuffer);
      BufferManager::addResourceFlags(
          _irradProbeIndexBuffer,
          Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_irradProbeIndexBuffer) =
          BufferType::kStorage;
      BufferManager::_descMemoryPoolType(_irradProbeIndexBuffer) =
          MemoryPoolType::kStaticStagingBuffers;
      BufferManager::_descSizeInBytes(_irradProbeIndexBuffer) =
          _totalGridSize * sizeof(uint32_t);
      buffersToCreate.push_back(_irradProbeIndexBuffer);
    }

    Resources::BufferManager::createResources(buffersToCreate);

    _lightBufferGpuMemory =
        (Light*)Resources::BufferManager::getGpuMemory(_lightBuffer);
    _lightIndexBufferGpuMemory =
        (uint32_t*)Resources::BufferManager::getGpuMemory(_lightIndexBuffer);

    _lightBufferMemory =
        (Light*)malloc(MAX_LIGHT_COUNT_PER_CLUSTER * _gridRes.x * _gridRes.y *
                       _gridRes.z * sizeof(Light));

    _irradProbeBufferGpuMemory =
        (IrradProbe*)Resources::BufferManager::getGpuMemory(_irradProbeBuffer);
    _irradProbeIndexBufferGpuMemory =
        (uint32_t*)Resources::BufferManager::getGpuMemory(
            _irradProbeIndexBuffer);

    _irradProbeBufferMemory =
        (IrradProbe*)malloc(MAX_IRRAD_PROBES_PER_CLUSTER * _gridRes.x *
                            _gridRes.y * _gridRes.z * sizeof(Light));
  }
}

// <-

namespace
{
_INTR_INLINE void setupLightingDrawCall(Resources::DrawCallRef p_DrawCallRef,
                                        bool p_Transparents)
{
  using namespace Resources;

  DrawCallManager::_descVertexCount(p_DrawCallRef) = 3u;

  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerInstance), GpuProgramType::kFragment,
      UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceFragment,
      sizeof(PerInstanceData));
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(PerFrame), GpuProgramType::kFragment,
      UniformManager::_perFrameUniformBuffer, UboType::kPerFrameFragment,
      sizeof(PerInstanceData));
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(albedoTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(
          !p_Transparents ? _N(GBufferAlbedo) : _N(GBufferTransparentsAlbedo)),
      Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(normalTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(
          !p_Transparents ? _N(GBufferNormal) : _N(GBufferTransparentsNormal)),
      Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(parameter0Tex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(!p_Transparents
                                          ? _N(GBufferParameter0)
                                          : _N(GBufferTransparentsParameter0)),
      Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(depthTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(
          !p_Transparents ? _N(GBufferDepth) : _N(GBufferTransparentsDepth)),
      Samplers::kNearestClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(ssaoTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(_N(SSAO)), Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(kelvinLutTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(_N(kelvin_rgb_LUT)),
      Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(specularTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(_N(default_ibl_cube_specular)),
      Samplers::kLinearClamp);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(noiseTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(_N(noise)), Samplers::kLinearRepeat);
  DrawCallManager::bindImage(
      p_DrawCallRef, _N(shadowBufferTex), GpuProgramType::kFragment,
      ImageManager::getResourceByName(_N(ShadowBuffer)), Samplers::kShadow);
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(MaterialBuffer), GpuProgramType::kFragment,
      MaterialBuffer::_materialBuffer, UboType::kInvalidUbo,
      BufferManager::_descSizeInBytes(MaterialBuffer::_materialBuffer));
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(LightBuffer), GpuProgramType::kFragment, _lightBuffer,
      UboType::kInvalidUbo, BufferManager::_descSizeInBytes(_lightBuffer));
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(LightIndexBuffer), GpuProgramType::kFragment,
      _lightIndexBuffer, UboType::kInvalidUbo,
      BufferManager::_descSizeInBytes(_lightIndexBuffer));
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(IrradProbeBuffer), GpuProgramType::kFragment,
      _irradProbeBuffer, UboType::kInvalidUbo,
      BufferManager::_descSizeInBytes(_irradProbeBuffer));
  DrawCallManager::bindBuffer(
      p_DrawCallRef, _N(IrradProbeIndexBuffer), GpuProgramType::kFragment,
      _irradProbeIndexBuffer, UboType::kInvalidUbo,
      BufferManager::_descSizeInBytes(_irradProbeIndexBuffer));
}
}

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
        Format::kB10G11R11UFloat;
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
        Format::kB10G11R11UFloat;
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
    setupLightingDrawCall(_drawCallRef, false);
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
    setupLightingDrawCall(_drawCallTransparentsRef, true);
  }
  drawcallsToCreate.push_back(_drawCallTransparentsRef);

  DrawCallManager::createResources(drawcallsToCreate);
}

// <-

void Lighting::destroy() {}

// <-

namespace
{
void spawnAndSimulateTestLights(Components::CameraRef p_CameraRef)
{
  // Spawn lights if none are there yet
  if (_testLights.empty())
  {
    static uint32_t testLightCount = 4096u * 4u;

    _testLights.resize(testLightCount);
    for (uint32_t i = 0u; i < testLightCount; ++i)
    {
      TestLight& light = _testLights[i];
      light.spawnPos =
          glm::vec3(Math::calcRandomFloatMinMax(-2000.0f, 2000.0f), 0.0f,
                    Math::calcRandomFloatMinMax(-2000.0f, 2000.0f));
      light.light.colorAndIntensity =
          glm::vec4(Math::calcRandomFloat(), Math::calcRandomFloat(),
                    Math::calcRandomFloat(), 5000.0f);
      light.light.temp = glm::vec4(6500.0f);
      light.light.posAndRadius = glm::vec4(glm::vec3(0.0f), 100.0f);
    }
  }

  // Update position and lights
  for (uint32_t i = 0u; i < _testLights.size(); ++i)
  {
    TestLight& light = _testLights[i];
    const glm::vec3 worldPos = glm::vec3(
        light.spawnPos.x,
        light.spawnPos.y + 2000.0f * sin(light.spawnPos.x + light.spawnPos.y +
                                         TaskManager::_totalTimePassed * 0.1f),
        light.spawnPos.z);

    light.light.posAndRadius = glm::vec4(
        glm::vec3(Components::CameraManager::_viewMatrix(p_CameraRef) *
                  glm::vec4(worldPos, 1.0f)),
        light.light.posAndRadius.w);
  }
}
}

void Lighting::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Lighting");
  _INTR_PROFILE_GPU("Render Lighting");

  // Testing code for profiling purposes
  {
    Components::NodeRef rootNodeRef = World::getRootNode();
    Entity::EntityRef rootEntityRef =
        Components::NodeManager::_entity(rootNodeRef);

    if (Entity::EntityManager::_name(rootEntityRef) == _N(LightingTest))
    {
      spawnAndSimulateTestLights(p_CameraRef);
    }
    else
    {
      _testLights.clear();
    }
  }

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

  {
    _INTR_PROFILE_GPU("Opaque");

    renderLighting(_framebufferRef, _drawCallRef, _lightingBufferImageRef,
                   p_CameraRef);
  }

  {
    _INTR_PROFILE_GPU("Transparents");

    renderLighting(_framebufferTransparentsRef, _drawCallTransparentsRef,
                   _lightingBufferTransparentsImageRef, p_CameraRef);
  }
}
}
}
}
}
