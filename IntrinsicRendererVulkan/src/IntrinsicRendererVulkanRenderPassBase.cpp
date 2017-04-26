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
void Base::init(const rapidjson::Value& p_RenderPassDesc)
{
  using namespace Resources;

  ImageRefArray imagesToCreate;
  RenderPassRefArray renderpassesToCreate;
  FramebufferRefArray framebuffersToCreate;

  const rapidjson::Value& outputs = p_RenderPassDesc["outputs"];
  const rapidjson::Value& images = p_RenderPassDesc["images"];
  _name = p_RenderPassDesc["name"].GetString();
  _perInstanceDataBufferName =
      p_RenderPassDesc["perInstanceDataBufferName"].GetString();

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

        ImageManager::_descDimensions(imageRef) = glm::uvec3(
            RenderSystem::getAbsoluteRenderSize(
                Helper::mapRenderSize(image["renderSize"].GetString())),
            1u);
        ImageManager::_descImageFormat(imageRef) =
            Helper::mapFormat(image["imageFormat"].GetString());
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

  RenderSize::Enum viewportRenderSize =
      Helper::mapRenderSize(p_RenderPassDesc["viewportRenderSize"].GetString());

  RenderPassManager::createResources(renderpassesToCreate);

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
}

// <-

void Base::destroy()
{
  using namespace Resources;

  ImageRefArray imagesToDestroy;
  FramebufferRefArray framebuffersToDestroy;
  RenderPassRefArray renderPassesToDestroy;

  if (!_framebufferRefs.empty())
    framebuffersToDestroy.insert(framebuffersToDestroy.end(),
                                 _framebufferRefs.begin(),
                                 _framebufferRefs.end());
  _framebufferRefs.clear();

  if (!_imageRefs.empty())
    imagesToDestroy.insert(imagesToDestroy.end(), _imageRefs.begin(),
                           _imageRefs.end());
  _imageRefs.clear();

  if (_renderPassRef.isValid())
    renderPassesToDestroy.push_back(_renderPassRef);
  _renderPassRef = RenderPassRef();

  FramebufferManager::destroyFramebuffersAndResources(framebuffersToDestroy);
  ImageManager::destroyImagesAndResources(imagesToDestroy);
  RenderPassManager::destroyRenderPassesAndResources(renderPassesToDestroy);
}
}
}
}
}
