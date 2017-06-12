// Copyright 2017 Benjamin Glatzel
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

using namespace RVResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
void GenericFullscreen::init(const rapidjson::Value& p_RenderPassDesc)
{
  Base::init(p_RenderPassDesc);

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  DrawCallRefArray drawCallsToCreate;

  const rapidjson::Value& inputs = p_RenderPassDesc["inputs"];

  _perInstanceDataBufferName =
      p_RenderPassDesc.HasMember("perInstanceDataBufferName")
          ? p_RenderPassDesc["perInstanceDataBufferName"].GetString()
          : "";
  _perInstanceDataVertexBufferName =
      p_RenderPassDesc.HasMember("perInstanceDataVertexBufferName")
          ? p_RenderPassDesc["perInstanceDataVertexBufferName"].GetString()
          : "";

  RenderProcess::UniformBufferDataEntry bufferDataEntry =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);
  RenderProcess::UniformBufferDataEntry bufferDataVertexEntry =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataVertexBufferName);

  const _INTR_STRING fragGpuProgramName =
      p_RenderPassDesc["fragmentGpuProgram"].GetString();
  const _INTR_STRING vertGpuProgramName =
      p_RenderPassDesc.HasMember("vertexGpuProgram")
          ? p_RenderPassDesc["vertexGpuProgram"].GetString()
          : "fullscreen_triangle.vert";

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(_name);
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u, {GpuProgramManager::getResourceByName(vertGpuProgramName),
             GpuProgramManager::getResourceByName(fragGpuProgramName)},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  RenderSize::Enum viewportRenderSize =
      Helper::mapRenderSize(p_RenderPassDesc["viewportRenderSize"].GetString());

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(_name);
    PipelineManager::resetToDefault(_pipelineRef);

    PipelineManager::_descFragmentProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName(fragGpuProgramName);
    PipelineManager::_descVertexProgram(_pipelineRef) =
        GpuProgramManager::getResourceByName(vertGpuProgramName);
    PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
    PipelineManager::_descPipelineLayout(_pipelineRef) = pipelineLayout;
    PipelineManager::_descVertexLayout(_pipelineRef) = Dod::Ref();
    PipelineManager::_descDepthStencilState(_pipelineRef) =
        DepthStencilStates::kDefaultNoDepthTestAndWrite;
    PipelineManager::_descViewportRenderSize(_pipelineRef) =
        (uint8_t)viewportRenderSize;
    PipelineManager::_descScissorRenderSize(_pipelineRef) =
        (uint8_t)viewportRenderSize;
  }
  pipelinesToCreate.push_back(_pipelineRef);

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_name);
  {
    DrawCallManager::resetToDefault(_drawCallRef);
    DrawCallManager::addResourceFlags(
        _drawCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    DrawCallManager::_descPipeline(_drawCallRef) = _pipelineRef;
    DrawCallManager::_descVertexCount(_drawCallRef) = 3u;

    if (bufferDataEntry.size > 0u)
    {
      DrawCallManager::bindBuffer(
          _drawCallRef, _N(PerInstance), GpuProgramType::kFragment,
          UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceFragment, bufferDataEntry.size);
    }
    if (bufferDataVertexEntry.size > 0u)
    {
      DrawCallManager::bindBuffer(
          _drawCallRef, _N(PerInstance), GpuProgramType::kVertex,
          UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceVertex, bufferDataVertexEntry.size);
    }
    if (p_RenderPassDesc.HasMember("needsPerFrameBufferFragment") &&
        p_RenderPassDesc["needsPerFrameBufferFragment"].GetBool())
    {
      DrawCallManager::bindBuffer(
          _drawCallRef, _N(PerFrame), GpuProgramType::kFragment,
          UniformManager::_perFrameUniformBuffer, UboType::kPerFrameFragment,
          sizeof(RenderProcess::PerFrameDataFrament));
    }
    if (p_RenderPassDesc.HasMember("needsPerFrameBufferVertex") &&
        p_RenderPassDesc["needsPerFrameBufferVertex"].GetBool())
    {
      DrawCallManager::bindBuffer(
          _drawCallRef, _N(PerFrame), GpuProgramType::kVertex,
          UniformManager::_perFrameUniformBuffer, UboType::kPerFrameVertex,
          sizeof(RenderProcess::PerFrameDataVertex));
      ;
    }

    for (uint32_t i = 0u; i < inputs.Size(); ++i)
    {
      const rapidjson::Value& input = inputs[i];
      if (strcmp(input[0].GetString(), "Image") == 0u)
      {
        const uint32_t bindingFlags =
            input.Size() >= 6u ? input[5].GetUint() : false;

        DrawCallManager::bindImage(
            _drawCallRef, input[2].GetString(),
            Helper::mapGpuProgramType(input[3].GetString()),
            ImageManager::getResourceByName(input[1].GetString()),
            Helper::mapSampler(input[4].GetString()), bindingFlags);
      }
      else if (strcmp(input[0].GetString(), "Buffer") == 0u)
      {
        BufferRef bufferRef =
            BufferManager::getResourceByName(input[1].GetString());

        DrawCallManager::bindBuffer(
            _drawCallRef, input[2].GetString(),
            Helper::mapGpuProgramType(input[3].GetString()), bufferRef,
            UboType::kInvalidUbo, BufferManager::_descSizeInBytes(bufferRef));
      }
    }

    drawCallsToCreate.push_back(_drawCallRef);
  }
  DrawCallManager::createResources(drawCallsToCreate);
}

// <-

void GenericFullscreen::destroy()
{
  Base::destroy();

  DrawCallRefArray drawCallsToDestroy;
  PipelineLayoutRefArray pipelineLayoutsToDestroy;
  PipelineRefArray pipelinesToDestroy;

  if (_drawCallRef.isValid())
    drawCallsToDestroy.push_back(_drawCallRef);
  _drawCallRef = DrawCallRef();
  if (_pipelineLayoutRef.isValid())
    pipelineLayoutsToDestroy.push_back(_pipelineLayoutRef);
  _pipelineLayoutRef = PipelineLayoutRef();
  if (_pipelineRef.isValid())
    pipelinesToDestroy.push_back(_pipelineRef);
  _pipelineRef = PipelineRef();

  PipelineManager::destroyPipelinesAndResources(pipelinesToDestroy);
  PipelineLayoutManager::destroyPipelineLayoutsAndResources(
      pipelineLayoutsToDestroy);
  DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
}

// <-

void GenericFullscreen::render(float p_DeltaT,
                               Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU_DEFINE(GenericFullscreenCPU, "Render Pass", _name.c_str());
  _INTR_PROFILE_CPU_CUSTOM(GenericFullscreenCPU);
  _INTR_PROFILE_GPU_DEFINE(GenericFullscreenGPU, _name.c_str());
  _INTR_PROFILE_GPU_CUSTOM(GenericFullscreenGPU, _name.c_str());

  const RenderProcess::UniformBufferDataEntry uniformData =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);
  const RenderProcess::UniformBufferDataEntry uniformDataVertex =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataVertexBufferName);

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, uniformDataVertex.uniformData, uniformDataVertex.size,
      uniformData.uniformData, uniformData.size);

  RenderSystem::beginRenderPass(
      _renderPassRef, _framebufferRefs[RenderSystem::_backbufferIndex %
                                       _framebufferRefs.size()],
      VK_SUBPASS_CONTENTS_INLINE, (uint32_t)_clearValues.size(),
      _clearValues.data());
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
