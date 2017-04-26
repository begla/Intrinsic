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
void GenericFullscreen::init(const rapidjson::Value& p_RenderPassDesc)
{
  using namespace Resources;

  Base::init(p_RenderPassDesc);

  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  DrawCallRefArray drawCallsToCreate;

  const rapidjson::Value& inputs = p_RenderPassDesc["inputs"];

  _perInstanceDataBufferName =
      p_RenderPassDesc["perInstanceDataBufferName"].GetString();
  RenderProcess::UniformBufferDataEntry bufferDataEntry =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);

  const _INTR_STRING fragGpuProgramName =
      p_RenderPassDesc["fragmentGpuProgram"].GetString();

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(_name);
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName(fragGpuProgramName)},
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
        GpuProgramManager::getResourceByName("fullscreen_triangle.vert");
    PipelineManager::_descRenderPass(_pipelineRef) = _renderPassRef;
    PipelineManager::_descPipelineLayout(_pipelineRef) = pipelineLayout;
    PipelineManager::_descVertexLayout(_pipelineRef) = Dod::Ref();
    PipelineManager::_descDepthStencilState(_pipelineRef) =
        DepthStencilStates::kDefaultNoDepthTestAndWrite;
    PipelineManager::_descViewportRenderSize(_pipelineRef) =
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

    DrawCallManager::bindBuffer(
        _drawCallRef, _N(PerInstance), GpuProgramType::kFragment,
        UniformManager::_perInstanceUniformBuffer,
        UboType::kPerInstanceFragment, bufferDataEntry.size);

    for (uint32_t i = 0u; i < inputs.Size(); ++i)
    {
      const rapidjson::Value& input = inputs[i];
      if (strcmp(input[0].GetString(), "Image") == 0u)
      {
        DrawCallManager::bindImage(
            _drawCallRef, input[2].GetString(),
            Helper::mapGpuProgramType(input[3].GetString()),
            ImageManager::getResourceByName(input[1].GetString()),
            Helper::mapSampler(input[4].GetString()));
      }
      else if (strcmp(input[0].GetString(), "Buffer") == 0u)
      {
        BufferRef bufferRef =
            Resources::BufferManager::getResourceByName(input[1].GetString());

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
  using namespace Resources;

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

void GenericFullscreen::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Renderer", _name.c_str());
  _INTR_PROFILE_GPU(_name.c_str());

  Components::CameraRef camRef = World::getActiveCamera();
  Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(camRef));

  const RenderProcess::UniformBufferDataEntry uniformData =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, uniformData.uniformData, uniformData.size);

  RenderSystem::beginRenderPass(
      _renderPassRef,
      _framebufferRefs[RenderSystem::_backbufferIndex %
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
