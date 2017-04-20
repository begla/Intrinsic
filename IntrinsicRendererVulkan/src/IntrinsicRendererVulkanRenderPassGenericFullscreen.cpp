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
namespace
{
struct PerInstanceDataGenericFullscreen
{
  glm::mat4 invProjMatrix;
  glm::mat4 invViewProjMatrix;
  glm::vec4 camPosition;
};
}

void GenericFullscreen::init(const rapidjson::Value& p_RenderPassDesc)
{
  using namespace Resources;

  ImageRefArray imagesToCreate;
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;
  FramebufferRefArray framebuffersToCreate;
  DrawCallRefArray drawCallsToCreate;

  const rapidjson::Value& inputs = p_RenderPassDesc["inputs"];
  const rapidjson::Value& outputs = p_RenderPassDesc["outputs"];
  const rapidjson::Value& images = p_RenderPassDesc["images"];
  const _INTR_STRING renderPassName = p_RenderPassDesc["name"].GetString();

  // Images
  {
    for (uint32_t i = 0u; i < images.Size(); ++i)
    {
      const rapidjson::Value& image = images[i];

      ImageRef imageRef = ImageManager::createImage(image["name"].GetString());
      {
        ImageManager::resetToDefault(imageRef);
        ImageManager::addResourceFlags(
            imageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
        ImageManager::_descMemoryPoolType(imageRef) =
            MemoryPoolType::kResolutionDependentImages;

        ImageManager::_descDimensions(imageRef) =
            glm::uvec3(RenderSystem::getAbsoluteRenderSize(
                           Helper::mapStringRenderSizeToRenderSize(
                               image["renderSize"].GetString())),
                       1u);
        ImageManager::_descImageFormat(imageRef) =
            Helper::mapStringFormatToFormat(image["imageFormat"].GetString());
        ImageManager::_descImageType(imageRef) = ImageType::kTexture;
      }
      _imageRefs.push_back(imageRef);
    }

    imagesToCreate.insert(imagesToCreate.end(), _imageRefs.begin(),
                          _imageRefs.end());
  }
  ImageManager::createResources(imagesToCreate);

  const _INTR_STRING fragGpuProgramName =
      p_RenderPassDesc["fragmentGpuProgram"].GetString();

  // Pipeline layout
  PipelineLayoutRef pipelineLayout;
  {
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(
        renderPassName + +"PipelineLayout");
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName(fragGpuProgramName)},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef =
        RenderPassManager::createRenderPass(renderPassName + "RenderPass");
    RenderPassManager::resetToDefault(_renderPassRef);

    for (uint32_t i = 0u; i < outputs.Size(); ++i)
    {
      const rapidjson::Value& output = outputs[i];

      ImageRef imageRef =
          ImageManager::_getResourceByName(output[1].GetString());

      AttachmentDescription colorAttachment = {
          (uint8_t)ImageManager::_descImageFormat(imageRef), (uint8_t)i};
      RenderPassManager::_descAttachments(_renderPassRef)
          .push_back(colorAttachment);
    }
  }
  renderpassesToCreate.push_back(_renderPassRef);

  // Pipelines
  {
    _pipelineRef = PipelineManager::createPipeline(renderPassName + "Pipeline");
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
  }
  pipelinesToCreate.push_back(_pipelineRef);

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  RenderPassManager::createResources(renderpassesToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(Generic));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    for (uint32_t i = 0u; i < outputs.Size(); ++i)
    {
      FramebufferManager::_descAttachedImages(_framebufferRef)
          .push_back(Resources::ImageManager::getResourceByName(
              outputs[i][1].GetString()));
    }

    FramebufferManager::_descDimensions(_framebufferRef) =
        glm::uvec3(RenderSystem::getAbsoluteRenderSize(
                       Helper::mapStringRenderSizeToRenderSize(
                           p_RenderPassDesc["renderSize"].GetString())),
                   1u);
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  framebuffersToCreate.push_back(_framebufferRef);

  FramebufferManager::createResources(framebuffersToCreate);

  // Draw calls
  _drawCallRef = DrawCallManager::createDrawCall(_N(Generic));
  {
    Vulkan::Resources::DrawCallManager::resetToDefault(_drawCallRef);
    Vulkan::Resources::DrawCallManager::addResourceFlags(
        _drawCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    Vulkan::Resources::DrawCallManager::_descPipeline(_drawCallRef) =
        _pipelineRef;
    Vulkan::Resources::DrawCallManager::_descVertexCount(_drawCallRef) = 3u;

    DrawCallManager::bindBuffer(_drawCallRef, _N(PerInstance),
                                GpuProgramType::kFragment,
                                UniformManager::_perInstanceUniformBuffer,
                                UboType::kPerInstanceFragment,
                                sizeof(PerInstanceDataGenericFullscreen));

    for (uint32_t i = 0u; i < inputs.Size(); ++i)
    {
      const rapidjson::Value& input = inputs[i];
      if (strcmp(input[0].GetString(), "Image") == 0u)
      {
        DrawCallManager::bindImage(
            _drawCallRef, input[2].GetString(),
            Helper::mapStringGpuProgramTypeToGpuProgramType(
                input[3].GetString()),
            ImageManager::getResourceByName(input[1].GetString()),
            Helper::mapStringSamplerToSampler(input[4].GetString()));
      }
      else if (strcmp(input[0].GetString(), "Buffer") == 0u)
      {
        BufferRef bufferRef =
            Resources::BufferManager::getResourceByName(input[1].GetString());

        DrawCallManager::bindBuffer(
            _drawCallRef, input[2].GetString(),
            Helper::mapStringGpuProgramTypeToGpuProgramType(
                input[3].GetString()),
            bufferRef, UboType::kInvalidUbo,
            BufferManager::_descSizeInBytes(bufferRef));
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

  DrawCallRefArray imagesToDestroy;
  DrawCallRefArray framebuffersToDestroy;
  DrawCallRefArray drawCallsToDestroy;

  if (_drawCallRef.isValid())
    drawCallsToDestroy.push_back(_drawCallRef);
  if (_framebufferRef.isValid())
    framebuffersToDestroy.push_back(_framebufferRef);
  if (!_imageRefs.empty())
    imagesToDestroy.insert(imagesToDestroy.end(), _imageRefs.begin(),
                           _imageRefs.end());

  DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
  FramebufferManager::destroyFramebuffersAndResources(framebuffersToDestroy);
  ImageManager::destroyImagesAndResources(imagesToDestroy);
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

  // Update per instance data
  PerInstanceDataGenericFullscreen perInstanceData = {
      Components::CameraManager::_inverseProjectionMatrix(camRef),
      Components::CameraManager::_inverseViewProjectionMatrix(camRef),
      glm::vec4(Components::NodeManager::_worldPosition(camNodeRef), 0.0f)};

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, &perInstanceData,
      sizeof(PerInstanceDataGenericFullscreen));

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_INLINE);
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
