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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderProcess
{
struct UniformBufferDataEntry
{
  UniformBufferDataEntry(void* p_UniformData, uint32_t p_Size)
      : uniformData(p_UniformData), size(p_Size)
  {
  }

  void* uniformData;
  uint32_t size;
};

struct UniformManager
{
  static void load(const rapidjson::Value& p_UniformBuffers);
  static void updatePerFrameUniformBufferData(Dod::Ref p_Camera);
  static void updateUniformBuffers();
  static void resetAllocator();
  static UniformBufferDataEntry requestUniformBufferData(const Name& p_Name);

  static struct UniformDataSource
  {
    glm::mat4 projectionMatrix;
    glm::mat4 inverseProjectionMatrix;
    glm::mat4 inverseViewProjectionMatrix;
    glm::vec4 cameraWorldPosition;
    glm::mat4 normalMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 prevViewMatrix;
    glm::vec4 haltonSamples;
    glm::vec4 haltonSamples32;
    glm::vec4 blurParamsXNormal;
    glm::vec4 blurParamsYNormal;
    glm::vec4 blurParamsXMedium;
    glm::vec4 blurParamsYMedium;
    glm::vec4 blurParamsXLow;
    glm::vec4 blurParamsYLow;
    glm::vec4 cameraParameters;
    glm::vec4 postParams0;
    glm::vec4 backbufferSize;

  } _uniformDataSource;
};
}
}
}
}
