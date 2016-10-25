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
namespace Resources
{
typedef Dod::Ref PipelineRef;
typedef _INTR_ARRAY(PipelineRef) PipelineRefArray;

struct PipelineData : Dod::Resources::ResourceDataBase
{
  PipelineData() : Dod::Resources::ResourceDataBase(_INTR_MAX_PIPELINE_COUNT)
  {
    descVertexLayout.resize(_INTR_MAX_PIPELINE_COUNT);
    descPipelineLayout.resize(_INTR_MAX_PIPELINE_COUNT);
    descRenderPass.resize(_INTR_MAX_PIPELINE_COUNT);
    descVertexProgram.resize(_INTR_MAX_PIPELINE_COUNT);
    descFragmentProgram.resize(_INTR_MAX_PIPELINE_COUNT);
    descGeometryProgram.resize(_INTR_MAX_PIPELINE_COUNT);
    descComputeProgram.resize(_INTR_MAX_PIPELINE_COUNT);

    descDepthStencilState.resize(_INTR_MAX_PIPELINE_COUNT);
    descInputAssemblyState.resize(_INTR_MAX_PIPELINE_COUNT);
    descRasterizationState.resize(_INTR_MAX_PIPELINE_COUNT);
    descBlendStates.resize(_INTR_MAX_PIPELINE_COUNT);

    descScissorRenderSize.resize(_INTR_MAX_PIPELINE_COUNT);
    descViewportRenderSize.resize(_INTR_MAX_PIPELINE_COUNT);
    descAbsoluteScissorDimensions.resize(_INTR_MAX_PIPELINE_COUNT);
    descAbsoluteViewportDimensions.resize(_INTR_MAX_PIPELINE_COUNT);

    vkPipeline.resize(_INTR_MAX_RENDER_PASS_COUNT);
  }

  // Desc.
  _INTR_ARRAY(VertexLayoutRef) descVertexLayout;
  _INTR_ARRAY(PipelineLayoutRef) descPipelineLayout;
  _INTR_ARRAY(RenderPassRef) descRenderPass;
  _INTR_ARRAY(GpuProgramRef) descVertexProgram;
  _INTR_ARRAY(GpuProgramRef) descFragmentProgram;
  _INTR_ARRAY(GpuProgramRef) descGeometryProgram;
  _INTR_ARRAY(GpuProgramRef) descComputeProgram;

  _INTR_ARRAY(uint8_t) descDepthStencilState;
  _INTR_ARRAY(uint8_t) descInputAssemblyState;
  _INTR_ARRAY(uint8_t) descRasterizationState;
  _INTR_ARRAY(_INTR_ARRAY(uint8_t)) descBlendStates;

  _INTR_ARRAY(uint8_t) descScissorRenderSize;
  _INTR_ARRAY(uint8_t) descViewportRenderSize;
  _INTR_ARRAY(glm::uvec2) descAbsoluteScissorDimensions;
  _INTR_ARRAY(glm::uvec2) descAbsoluteViewportDimensions;

  // GPU resources
  _INTR_ARRAY(VkPipeline) vkPipeline;
};

struct PipelineManager
    : Dod::Resources::ResourceManagerBase<PipelineData,
                                          _INTR_MAX_PIPELINE_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Pipeline Manager...");

    Dod::Resources::ResourceManagerBase<
        PipelineData, _INTR_MAX_PIPELINE_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static PipelineRef createPipeline(const Name& p_Name)
  {
    PipelineRef ref = Dod::Resources::ResourceManagerBase<
        PipelineData, _INTR_MAX_PIPELINE_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(BufferRef p_Ref)
  {
    _descVertexLayout(p_Ref) = VertexLayoutRef();
    _descPipelineLayout(p_Ref) = PipelineLayoutRef();
    _descRenderPass(p_Ref) = RenderPassRef();
    _descVertexProgram(p_Ref) = GpuProgramRef();
    _descFragmentProgram(p_Ref) = GpuProgramRef();
    _descGeometryProgram(p_Ref) = GpuProgramRef();
    _descComputeProgram(p_Ref) = GpuProgramRef();

    _descDepthStencilState(p_Ref) = DepthStencilStates::kDefault;
    _descInputAssemblyState(p_Ref) = InputAssemblyStates::kTriangleList;
    _descRasterizationState(p_Ref) = RasterizationStates::kDefault;

    _descScissorRenderSize(p_Ref) = RenderSize::kFull;
    _descViewportRenderSize(p_Ref) = RenderSize::kFull;
    _descAbsoluteScissorDimensions(p_Ref) = glm::uvec2(0u);
    _descAbsoluteViewportDimensions(p_Ref) = glm::uvec2(0u);

    _descBlendStates(p_Ref).clear();
    _descBlendStates(p_Ref).push_back(BlendStates::kDefault);
  }

  _INTR_INLINE static void destroyPipeline(PipelineRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineData, _INTR_MAX_PIPELINE_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(PipelineRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineData,
        _INTR_MAX_PIPELINE_COUNT>::_compileDescriptor(p_Ref, p_Properties,
                                                      p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(PipelineRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineData,
        _INTR_MAX_PIPELINE_COUNT>::_initFromDescriptor(p_Ref, p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineData,
        _INTR_MAX_PIPELINE_COUNT>::_saveToSingleFile(p_FileName,
                                                     compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        PipelineData,
        _INTR_MAX_PIPELINE_COUNT>::_loadFromSingleFile(p_FileName,
                                                       initFromDescriptor,
                                                       resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  static void createResources(const PipelineRefArray& p_Pipelines);

  // <-

  _INTR_INLINE static void destroyResources(const PipelineRefArray& p_Pipelines)
  {
    for (uint32_t i = 0u; i < p_Pipelines.size(); ++i)
    {
      PipelineRef ref = p_Pipelines[i];
      VkPipeline& pipeline = _vkPipeline(ref);

      if (pipeline != VK_NULL_HANDLE)
      {
        RenderSystem::releaseResource(_N(VkPipeline), (void*)pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
      }
    }
  }

  // <-

  _INTR_INLINE static void
  destroyPipelinesAndResources(const FramebufferRefArray& p_Pipelines)
  {
    destroyResources(p_Pipelines);

    for (uint32_t i = 0u; i < p_Pipelines.size(); ++i)
    {
      destroyPipeline(p_Pipelines[i]);
    }
  }

  // Getter/Setter
  // ->

  _INTR_INLINE static VertexLayoutRef& _descVertexLayout(PipelineRef p_Ref)
  {
    return _data.descVertexLayout[p_Ref._id];
  }
  _INTR_INLINE static PipelineLayoutRef& _descPipelineLayout(PipelineRef p_Ref)
  {
    return _data.descPipelineLayout[p_Ref._id];
  }
  _INTR_INLINE static RenderPassRef& _descRenderPass(PipelineRef p_Ref)
  {
    return _data.descRenderPass[p_Ref._id];
  }
  _INTR_INLINE static GpuProgramRef& _descVertexProgram(PipelineRef p_Ref)
  {
    return _data.descVertexProgram[p_Ref._id];
  }
  _INTR_INLINE static GpuProgramRef& _descFragmentProgram(PipelineRef p_Ref)
  {
    return _data.descFragmentProgram[p_Ref._id];
  }
  _INTR_INLINE static GpuProgramRef& _descGeometryProgram(PipelineRef p_Ref)
  {
    return _data.descGeometryProgram[p_Ref._id];
  }
  _INTR_INLINE static GpuProgramRef& _descComputeProgram(PipelineRef p_Ref)
  {
    return _data.descComputeProgram[p_Ref._id];
  }

  _INTR_INLINE static uint8_t& _descDepthStencilState(PipelineRef p_Ref)
  {
    return _data.descDepthStencilState[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descInputAssemblyState(PipelineRef p_Ref)
  {
    return _data.descInputAssemblyState[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descRasterizationState(PipelineRef p_Ref)
  {
    return _data.descRasterizationState[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(uint8_t) & _descBlendStates(PipelineRef p_Ref)
  {
    return _data.descBlendStates[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descScissorRenderSize(PipelineRef p_Ref)
  {
    return _data.descScissorRenderSize[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descViewportRenderSize(PipelineRef p_Ref)
  {
    return _data.descViewportRenderSize[p_Ref._id];
  }
  _INTR_INLINE static glm::uvec2&
  _descAbsoluteScissorDimensions(PipelineRef p_Ref)
  {
    return _data.descAbsoluteScissorDimensions[p_Ref._id];
  }
  _INTR_INLINE static glm::uvec2&
  _descAbsoluteViewportDimensions(PipelineRef p_Ref)
  {
    return _data.descAbsoluteViewportDimensions[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkPipeline& _vkPipeline(PipelineRef p_Ref)
  {
    return _data.vkPipeline[p_Ref._id];
  }
};
}
}
}
}
