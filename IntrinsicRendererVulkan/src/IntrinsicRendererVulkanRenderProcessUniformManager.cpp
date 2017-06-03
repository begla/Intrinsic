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

// Sky model
#include "IntrinsicCoreSkyModel.h"

#define HALTON_SAMPLE_COUNT 1024

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderProcess
{
// Static members
UniformManager::UniformDataSource
    UniformManager::UniformManager::_uniformDataSource;

// Uniform data decl.
namespace
{
glm::vec3 _haltonSamples[HALTON_SAMPLE_COUNT];

_INTR_INLINE void initStaticUniformData()
{
  // Generate Halton Samples
  {
    for (uint32_t i = 0u; i < HALTON_SAMPLE_COUNT; ++i)
    {
      _haltonSamples[i] = glm::vec3(Math::calcHaltonSequence(i, 2u),
                                    Math::calcHaltonSequence(i, 3u),
                                    Math::calcHaltonSequence(i, 4u));
    }
  }

  UniformManager::_uniformDataSource.blurParamsXNormal =
      glm::vec4(3.0f, 0.0f, 1.0f, 0.0f);
  UniformManager::_uniformDataSource.blurParamsYNormal =
      glm::vec4(3.0f, 0.0f, 0.0f, 1.0f);
  UniformManager::_uniformDataSource.blurParamsXMedium =
      glm::vec4(6.0f, 0.0f, 1.0f, 0.0f);
  UniformManager::_uniformDataSource.blurParamsYMedium =
      glm::vec4(6.0f, 0.0f, 0.0f, 1.0f);
  UniformManager::_uniformDataSource.blurParamsXLow =
      glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
  UniformManager::_uniformDataSource.blurParamsYLow =
      glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
}

namespace
{
struct UniformDataRef
{
  UniformDataRef() { memset(this, 0x00u, sizeof(UniformDataRef)); }

  UniformDataRef(const void* p_Ptr, uint32_t p_Size)
  {
    const uint64_t address = (uint64_t)p_Ptr;
    offset =
        (uint16_t)(address - (uint64_t)&UniformManager::_uniformDataSource);
    size = (uint16_t)p_Size;
  }

  uint16_t offset;
  uint16_t size;
};

struct UniformBuffer
{
  UniformBuffer() { memset(this, 0x00u, sizeof(UniformBuffer)); }

  UniformDataRef refs[32u];
  uint8_t refCount;

  uint32_t dataSize;
  void* data;
};

_INTR_HASH_MAP(Name, UniformDataRef)
_uniformOffsetMapping = {
    {"ProjectionMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.projectionMatrix,
                    sizeof(glm::mat4))},
    {"ViewMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.viewMatrix,
                    sizeof(glm::mat4))},
    {"InverseViewMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.inverseViewMatrix,
                    sizeof(glm::mat4))},
    {"PrevViewMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.prevViewMatrix,
                    sizeof(glm::mat4))},
    {"InverseProjectionMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.inverseProjectionMatrix,
                    sizeof(glm::mat4))},
    {"InverseViewProjectionMatrix",
     UniformDataRef(
         &UniformManager::_uniformDataSource.inverseViewProjectionMatrix,
         sizeof(glm::mat4))},
    {"CameraWorldPosition",
     UniformDataRef(&UniformManager::_uniformDataSource.cameraWorldPosition,
                    sizeof(glm::vec4))},
    {"BlurParamsXNormal",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsXNormal,
                    sizeof(glm::vec4))},
    {"BlurParamsYNormal",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsYNormal,
                    sizeof(glm::vec4))},
    {"BlurParamsXMedium",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsXMedium,
                    sizeof(glm::vec4))},
    {"BlurParamsYMedium",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsYMedium,
                    sizeof(glm::vec4))},
    {"BlurParamsXLow",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsXLow,
                    sizeof(glm::vec4))},
    {"BlurParamsYLow",
     UniformDataRef(&UniformManager::_uniformDataSource.blurParamsYLow,
                    sizeof(glm::vec4))},
    {"CameraParameters",
     UniformDataRef(&UniformManager::_uniformDataSource.cameraParameters,
                    sizeof(glm::vec4))},
    {"PostParams0",
     UniformDataRef(&UniformManager::_uniformDataSource.postParams0,
                    sizeof(glm::vec4))},
    {"BackbufferSize",
     UniformDataRef(&UniformManager::_uniformDataSource.backbufferSize,
                    sizeof(glm::vec4))},
    {"HaltonSamples",
     UniformDataRef(&UniformManager::_uniformDataSource.haltonSamples,
                    sizeof(glm::ivec4))}};

_INTR_HASH_MAP(Name, UniformBuffer) _uniformBuffers;
uint8_t* _uniformBufferMemory = nullptr;
LinearOffsetAllocator _uniformBufferMemoryAllocator;
const uint32_t _uniformBufferMemorySizeInBytes = 2u * 1024u * 1024u;
}

// <-

void UniformManager::load(const rapidjson::Value& p_UniformBuffers)
{
  if (_uniformBufferMemory)
  {
    free(_uniformBufferMemory);
    _uniformBufferMemory = nullptr;
  }
  _uniformBufferMemory = (uint8_t*)malloc(_uniformBufferMemorySizeInBytes);
  _uniformBufferMemoryAllocator.init(_uniformBufferMemorySizeInBytes);
  _uniformBuffers.clear();

  for (uint32_t bufferIdx = 0u; bufferIdx < p_UniformBuffers.Size();
       ++bufferIdx)
  {
    const rapidjson::Value& uniformBufferDesc = p_UniformBuffers[bufferIdx];
    const rapidjson::Value& entryDescs = uniformBufferDesc["entries"];

    UniformBuffer uniformBuffer;
    for (uint32_t entryIdx = 0u; entryIdx < entryDescs.Size(); ++entryIdx)
    {
      const rapidjson::Value& entryDesc = entryDescs[entryIdx];
      UniformDataRef entry = _uniformOffsetMapping[entryDesc.GetString()];

      uniformBuffer.refs[uniformBuffer.refCount] = entry;

      ++uniformBuffer.refCount;
      uniformBuffer.dataSize += entry.size;
    }

    _uniformBuffers[uniformBufferDesc["name"].GetString()] = uniformBuffer;
  }

  initStaticUniformData();
}

// <-

void UniformManager::updatePerFrameUniformBufferData(Dod::Ref p_Camera)
{
  _INTR_PROFILE_CPU("General", "Update Per Frame Uniform Buffer Data");

  // Uniforms for the render passes
  {
    UniformManager::_uniformDataSource.postParams0.x =
        RenderPass::Lighting::_globalAmbientFactor;
    UniformManager::_uniformDataSource.postParams0.y =
        World::_currentDayNightFactor;
    UniformManager::_uniformDataSource.postParams0.z =
        Core::Resources::PostEffectManager::_descDoFStartDistance(
            Core::Resources::PostEffectManager::_blendTargetRef);

    UniformManager::_uniformDataSource.cameraParameters.x =
        Components::CameraManager::_descNearPlane(p_Camera);
    UniformManager::_uniformDataSource.cameraParameters.y =
        Components::CameraManager::_descFarPlane(p_Camera);
    UniformManager::_uniformDataSource.cameraParameters.z =
        1.0f / UniformManager::_uniformDataSource.cameraParameters.x;
    UniformManager::_uniformDataSource.cameraParameters.w =
        1.0f / UniformManager::_uniformDataSource.cameraParameters.y;
    UniformManager::_uniformDataSource.inverseViewMatrix =
        Components::CameraManager::_inverseViewMatrix(p_Camera);
    UniformManager::_uniformDataSource.projectionMatrix =
        Components::CameraManager::_projectionMatrix(p_Camera);
    UniformManager::_uniformDataSource.prevViewMatrix =
        Components::CameraManager::_prevViewMatrix(p_Camera);
    UniformManager::_uniformDataSource.viewMatrix =
        Components::CameraManager::_viewMatrix(p_Camera);
    UniformManager::_uniformDataSource.inverseProjectionMatrix =
        Components::CameraManager::_inverseProjectionMatrix(p_Camera);
    UniformManager::_uniformDataSource.inverseViewProjectionMatrix =
        Components::CameraManager::_inverseViewProjectionMatrix(p_Camera);

    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(p_Camera));
    UniformManager::_uniformDataSource.cameraWorldPosition =
        glm::vec4(Components::NodeManager::_worldPosition(cameraNode), 0.0f);

    UniformManager::_uniformDataSource.haltonSamples = glm::vec4(
        _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].x,
        _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].y,
        _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].z,
        0.0f);
    UniformManager::_uniformDataSource.haltonSamples32 =
        glm::vec4(_haltonSamples[TaskManager::_frameCounter % 32].x,
                  _haltonSamples[TaskManager::_frameCounter % 32].y,
                  _haltonSamples[TaskManager::_frameCounter % 32].z, 0.0f);

    glm::vec2 backbufferSize = glm::vec2(RenderSystem::_backbufferDimensions);
    UniformManager::_uniformDataSource.backbufferSize =
        glm::vec4(backbufferSize, 1.0f / backbufferSize);
  }

  // Global per frame data
  {
    static const uint32_t perFrameRangeSizeInBytes =
        sizeof(RenderProcess::PerFrameDataVertex) +
        sizeof(RenderProcess::PerFrameDataFrament);

    uint8_t* perFrameVertexMemory =
        Vulkan::UniformManager::_perFrameMemory +
        RenderProcess::UniformManager::getDynamicOffsetForPerFrameDataVertex();
    ;
    uint8_t* perFrameFragmentMemory =
        Vulkan::UniformManager::_perFrameMemory +
        RenderProcess::UniformManager::
            getDynamicOffsetForPerFrameDataFragment();

    _INTR_ASSERT(sizeof(RenderProcess::PerFrameDataVertex) <
                     _INTR_VK_PER_FRAME_BLOCK_SIZE_IN_BYTES &&
                 sizeof(RenderProcess::PerFrameDataFrament) <
                     _INTR_VK_PER_FRAME_BLOCK_SIZE_IN_BYTES &&
                 "Per frame memory block too small");

    PerFrameDataVertex vertexData;
    {
      // TODO: Nothing here yet
    }
    memcpy(perFrameVertexMemory, &vertexData, sizeof(vertexData));

    PerFrameDataFrament fragmentData;
    {
      fragmentData.viewMatrix = UniformManager::_uniformDataSource.viewMatrix;
      fragmentData.invProjectionMatrix =
          UniformManager::_uniformDataSource.inverseProjectionMatrix;
      fragmentData.invViewMatrix =
          UniformManager::_uniformDataSource.inverseViewMatrix;

      // Sky
      const glm::vec3 sunDir =
          Core::Resources::PostEffectManager::calcActualSunOrientation(
              Core::Resources::PostEffectManager::_blendTargetRef) *
          glm::vec3(0.0f, 0.0f, 1.0f);
      fragmentData.sunLightDirWS = glm::vec4(sunDir, 0.0f);
      fragmentData.sunLightDirVS =
          UniformManager::_uniformDataSource.viewMatrix *
          fragmentData.sunLightDirWS;
      fragmentData.sunLightColorAndIntensity =
          World::_currentSunLightColorAndIntensity;
      fragmentData.sunLightColorAndIntensity.w *=
          Core::Resources::PostEffectManager::_descSunIntensity(
              Core::Resources::PostEffectManager::_blendTargetRef);

      const float elevation =
          glm::half_pi<float>() -
          std::acos(std::max(glm::dot(sunDir, glm::vec3(0.0f, 1.0f, 0.0f)),
                             0.00001f));

      // TODO: Move params to post effect
      SkyModel::ArHosekSkyModelState skyModel =
          SkyModel::createSkyModelStateRGB(
              Core::Resources::PostEffectManager::_descSkyTurbidity(
                  Core::Resources::PostEffectManager::_blendTargetRef),
              Core::Resources::PostEffectManager::_descSkyAlbedo(
                  Core::Resources::PostEffectManager::_blendTargetRef),
              elevation);
      const float radianceFactor =
          World::_currentDayNightFactor *
          Core::Resources::PostEffectManager::_descSkyLightIntensity(
              Core::Resources::PostEffectManager::_blendTargetRef);
      skyModel.radiances[0] *= radianceFactor;
      skyModel.radiances[1] *= radianceFactor;
      skyModel.radiances[2] *= radianceFactor;

      // Project sky light SH
      {
        Irradiance::SH9 skyLightSH = SkyModel::project(skyModel, sunDir, 64u);
        memcpy(fragmentData.skyLightSH, &skyLightSH, sizeof(Irradiance::SH9));
      }

      // Pack sky model configs/radiance for sky sampling
      for (uint32_t channelIdx = 0u; channelIdx < 3u; ++channelIdx)
      {
        for (uint32_t i = 0u; i < 9u; ++i)
        {
          const uint32_t idx = i + channelIdx * 9u;
          const glm::uvec2 packedDataOffset = glm::uvec2(idx / 4u, idx % 4u);

          fragmentData.skyModelConfigs[packedDataOffset.x][packedDataOffset.y] =
              (float)skyModel.configs[channelIdx][i];
        }

        fragmentData.skyModelRadiances[channelIdx] =
            (float)skyModel.radiances[channelIdx];
      }
      memcpy(perFrameFragmentMemory, &fragmentData, sizeof(fragmentData));
    }
  }
}

// <-

void UniformManager::updateUniformBuffers()
{
  for (auto it = _uniformBuffers.begin(); it != _uniformBuffers.end(); ++it)
  {
    UniformBuffer& uniformBuffer = it->second;

    uint32_t currentOffset = 0u;
    uint8_t* bufferData =
        (uint8_t*)_uniformBufferMemory +
        _uniformBufferMemoryAllocator.allocate(uniformBuffer.dataSize, 8u);

    for (uint32_t refIdx = 0u; refIdx < uniformBuffer.refCount; ++refIdx)
    {
      const UniformDataRef dataRef = uniformBuffer.refs[refIdx];
      memcpy(bufferData + currentOffset,
             ((uint8_t*)&UniformManager::_uniformDataSource) + dataRef.offset,
             dataRef.size);
      currentOffset += dataRef.size;
    }

    uniformBuffer.data = bufferData;
  }
}

// <-

void UniformManager::resetAllocator() { _uniformBufferMemoryAllocator.reset(); }

// <-

UniformBufferDataEntry
UniformManager::requestUniformBufferData(const Name& p_Name)
{
  auto it = _uniformBuffers.find(p_Name);
  if (it != _uniformBuffers.end())
  {
    const UniformBuffer& buffer = it->second;
    return UniformBufferDataEntry(buffer.data, buffer.dataSize);
  }

  return UniformBufferDataEntry(nullptr, 0u);
}
}
}
}
}
