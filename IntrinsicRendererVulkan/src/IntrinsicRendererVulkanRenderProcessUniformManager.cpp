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
    {"PrevViewMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.prevViewMatrix,
                    sizeof(glm::mat4))},
    {"NormalMatrix",
     UniformDataRef(&UniformManager::_uniformDataSource.normalMatrix,
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
  UniformManager::_uniformDataSource.postParams0.x =
      Core::Resources::PostEffectManager::_descAmbientFactor(
          Core::Resources::PostEffectManager::_blendTargetRef);

  UniformManager::_uniformDataSource.cameraParameters.x =
      Components::CameraManager::_descNearPlane(p_Camera);
  UniformManager::_uniformDataSource.cameraParameters.y =
      Components::CameraManager::_descFarPlane(p_Camera);
  UniformManager::_uniformDataSource.cameraParameters.z =
      1.0f / UniformManager::_uniformDataSource.cameraParameters.x;
  UniformManager::_uniformDataSource.cameraParameters.w =
      1.0f / UniformManager::_uniformDataSource.cameraParameters.y;
  UniformManager::_uniformDataSource.projectionMatrix =
      Components::CameraManager::_projectionMatrix(p_Camera);
  UniformManager::_uniformDataSource.prevViewMatrix =
      Components::CameraManager::_prevViewMatrix(p_Camera);
  UniformManager::_uniformDataSource.viewMatrix =
      Components::CameraManager::_viewMatrix(p_Camera);
  UniformManager::_uniformDataSource.normalMatrix =
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
      _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].z, 0.0f);
  UniformManager::_uniformDataSource.haltonSamples32 =
      glm::vec4(_haltonSamples[TaskManager::_frameCounter % 32].x,
                _haltonSamples[TaskManager::_frameCounter % 32].y,
                _haltonSamples[TaskManager::_frameCounter % 32].z, 0.0f);
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
  const UniformBuffer& buffer = _uniformBuffers[p_Name];
  return UniformBufferDataEntry(buffer.data, buffer.dataSize);
}
}
}
}
}
