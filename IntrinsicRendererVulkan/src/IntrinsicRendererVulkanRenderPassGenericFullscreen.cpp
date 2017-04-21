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

  ImageRefArray imagesToCreate;
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  RenderPassRefArray renderpassesToCreate;
  FramebufferRefArray framebuffersToCreate;
  DrawCallRefArray drawCallsToCreate;

  const rapidjson::Value& inputs = p_RenderPassDesc["inputs"];
  const rapidjson::Value& outputs = p_RenderPassDesc["outputs"];
  const rapidjson::Value& images = p_RenderPassDesc["images"];
  _name = p_RenderPassDesc["name"].GetString();
  _perInstanceDataBufferName =
      p_RenderPassDesc["perInstanceDataBufferName"].GetString();
  RenderProcess::UniformBufferDataEntry bufferDataEntry =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);

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
    pipelineLayout = PipelineLayoutManager::createPipelineLayout(_name);
    PipelineLayoutManager::resetToDefault(pipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {Resources::GpuProgramManager::getResourceByName(fragGpuProgramName)},
        pipelineLayout);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayout);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_name);
    RenderPassManager::resetToDefault(_renderPassRef);

    if (outputs[0][1] != "Backbuffer")
    {
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
    else
    {
      AttachmentDescription sceneAttachment = {Format::kB8G8R8A8Srgb, 0u};
      RenderPassManager::_descAttachments(_renderPassRef)
          .push_back(sceneAttachment);
    }
  }
  renderpassesToCreate.push_back(_renderPassRef);

  RenderSize::Enum viewportRenderSize = Helper::mapStringRenderSizeToRenderSize(
      p_RenderPassDesc["viewportRenderSize"].GetString());

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
  RenderPassManager::createResources(renderpassesToCreate);
  PipelineManager::createResources(pipelinesToCreate);

  // Create framebuffer
  if (outputs[0][1] != "Backbuffer")
  {
    FramebufferRef framebufferRef =
        FramebufferManager::createFramebuffer(_name);
    {
      FramebufferManager::resetToDefault(framebufferRef);
      FramebufferManager::addResourceFlags(
          framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

      glm::uvec3 dim = glm::vec3(0u);
      for (uint32_t i = 0u; i < outputs.Size(); ++i)
      {
        ImageRef outputImage =
            ImageManager::getResourceByName(outputs[i][1].GetString());
        FramebufferManager::_descAttachedImages(framebufferRef)
            .push_back(outputImage);

        _INTR_ASSERT((dim == glm::uvec3(0u) ||
                      dim == ImageManager::_descDimensions(outputImage)) &&
                     "Dimensions of all outputs must be identical");
        dim = ImageManager::_descDimensions(outputImage);
      }

      FramebufferManager::_descDimensions(framebufferRef) = dim;
      FramebufferManager::_descRenderPass(framebufferRef) = _renderPassRef;
    }
    framebuffersToCreate.push_back(framebufferRef);
    _framebufferRefs.push_back(framebufferRef);
  }
  else
  {
    for (uint32_t i = 0u; i < (uint32_t)RenderSystem::_vkSwapchainImages.size();
         ++i)
    {
      FramebufferRef fbRef = FramebufferManager::createFramebuffer(_name);
      {
        FramebufferManager::resetToDefault(fbRef);
        FramebufferManager::addResourceFlags(
            fbRef, Dod::Resources::ResourceFlags::kResourceVolatile);

        FramebufferManager::_descAttachedImages(fbRef).push_back(
            ImageManager::getResourceByName(_INTR_STRING("Backbuffer") +
                                            StringUtil::toString<uint32_t>(i)));

        FramebufferManager::_descDimensions(fbRef) =
            glm::uvec2(RenderSystem::_backbufferDimensions.x,
                       RenderSystem::_backbufferDimensions.y);
        FramebufferManager::_descRenderPass(fbRef) = _renderPassRef;
      }

      framebuffersToCreate.push_back(fbRef);
      _framebufferRefs.push_back(fbRef);
    }
  }

  FramebufferManager::createResources(framebuffersToCreate);

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
  if (!_framebufferRefs.empty())
    framebuffersToDestroy.insert(framebuffersToDestroy.end(),
                                 _framebufferRefs.begin(),
                                 _framebufferRefs.end());
  _framebufferRefs.clear();
  if (!_imageRefs.empty())
    imagesToDestroy.insert(imagesToDestroy.end(), _imageRefs.begin(),
                           _imageRefs.end());
  _imageRefs.clear();

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

  const RenderProcess::UniformBufferDataEntry uniformData =
      RenderProcess::UniformManager::requestUniformBufferData(
          _perInstanceDataBufferName);

  DrawCallManager::allocateAndUpdateUniformMemory(
      {_drawCallRef}, nullptr, 0u, uniformData.uniformData, uniformData.size);

  RenderSystem::beginRenderPass(
      _renderPassRef,
      _framebufferRefs[RenderSystem::_backbufferIndex %
                       _framebufferRefs.size()],
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
