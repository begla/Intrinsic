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
// Uniform data decl.
namespace
{
struct UniformDataSource
{
  glm::mat4 inverseProjectionMatrix;
  glm::mat4 inverseViewProjectionMatrix;
  glm::vec4 cameraWorldPosition;
  glm::ivec4 haltonSamples;
} _uniformDataSource;

glm::vec2 _haltonSamples[HALTON_SAMPLE_COUNT];

_INTR_INLINE void initStaticUniformData()
{
  // Generate Halton Samples
  {
    for (uint32_t i = 0u; i < HALTON_SAMPLE_COUNT; ++i)
    {
      _haltonSamples[i] = glm::vec2(Math::calcHaltonSequence(i, 2u),
                                    Math::calcHaltonSequence(i, 3u));
    }
  }
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
    offset = (uint16_t)(address - (uint64_t)&_uniformDataSource);
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
    {"InverseProjectionMatrix",
     UniformDataRef(&_uniformDataSource.inverseProjectionMatrix,
                    sizeof(glm::mat4))},
    {"InverseViewProjectionMatrix",
     UniformDataRef(&_uniformDataSource.inverseViewProjectionMatrix,
                    sizeof(glm::mat4))},
    {"CameraWorldPosition",
     UniformDataRef(&_uniformDataSource.cameraWorldPosition,
                    sizeof(glm::vec4))},
    {"HaltonSamples",
     UniformDataRef(&_uniformDataSource.haltonSamples, sizeof(glm::ivec4))}};

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
  _uniformDataSource.inverseProjectionMatrix =
      Components::CameraManager::_inverseProjectionMatrix(p_Camera);
  _uniformDataSource.inverseViewProjectionMatrix =
      Components::CameraManager::_inverseViewProjectionMatrix(p_Camera);

  Components::NodeRef cameraNode =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(p_Camera));
  _uniformDataSource.cameraWorldPosition =
      glm::vec4(Components::NodeManager::_worldPosition(cameraNode), 0.0f);
  _uniformDataSource.haltonSamples = glm::ivec4(
      (int32_t)(
          _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].x *
          255),
      (int32_t)(
          _haltonSamples[TaskManager::_frameCounter % HALTON_SAMPLE_COUNT].y *
          255),
      0, 0);
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
             ((uint8_t*)&_uniformDataSource) + dataRef.offset, dataRef.size);
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
