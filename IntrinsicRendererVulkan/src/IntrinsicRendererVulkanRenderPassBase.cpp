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

  for (uint32_t backBufferIdx = 0u; backBufferIdx < outputs.Size();
       ++backBufferIdx)
  {
    const rapidjson::Value& outputDesc = outputs[backBufferIdx];

    VkClearValue clearValue = {};
    if (outputDesc.Size() > 1u)
    {
      const rapidjson::Value& clearColorDesc = outputDesc[1u];

      if (clearColorDesc.Size() > 1u)
      {
        clearValue.color.float32[0] = clearColorDesc[0].GetFloat();
        clearValue.color.float32[1] = clearColorDesc[1].GetFloat();
        clearValue.color.float32[2] = clearColorDesc[2].GetFloat();
        clearValue.color.float32[3] = clearColorDesc[3].GetFloat();
      }
      else
      {
        clearValue.depthStencil.depth = clearColorDesc[0].GetFloat();
      }
    }

    _clearValues.push_back(clearValue);
  }

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

        const rapidjson::Value& imageFormat = image["imageFormat"];
        if (imageFormat == "Depth")
          ImageManager::_descImageFormat(imageRef) =
              RenderSystem::_depthStencilFormatToUse;
        else
          ImageManager::_descImageFormat(imageRef) =
              Helper::mapFormat(imageFormat.GetString());
        ImageManager::_descImageType(imageRef) = ImageType::kTexture;
      }
      _imageRefs.push_back(imageRef);
    }

    imagesToCreate.insert(imagesToCreate.end(), _imageRefs.begin(),
                          _imageRefs.end());
  }
  ImageManager::createResources(imagesToCreate);

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_name);
    RenderPassManager::resetToDefault(_renderPassRef);

    for (uint32_t i = 0u; i < outputs.Size(); ++i)
    {
      const rapidjson::Value& outputDesc = outputs[i];

      ImageRef imageRef =
          ImageManager::_getResourceByName(outputDesc[0].GetString());

      if (outputs[i][0] != "Backbuffer")
      {
        AttachmentDescription colorAttachment = {
            (uint8_t)ImageManager::_descImageFormat(imageRef),
            outputDesc.Size() > 1u ? AttachmentFlags::kClearOnLoad : 0u};
        RenderPassManager::_descAttachments(_renderPassRef)
            .push_back(colorAttachment);
      }
      else
      {
        AttachmentDescription sceneAttachment = {
            Format::kB8G8R8A8Srgb,
            outputs[0].Size() > 1u ? AttachmentFlags::kClearOnLoad : 0u};
        RenderPassManager::_descAttachments(_renderPassRef)
            .push_back(sceneAttachment);
      }
    }
  }
  renderpassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderpassesToCreate);

  // Create framebuffer

  for (uint32_t backBufferIdx = 0u;
       backBufferIdx < (uint32_t)RenderSystem::_vkSwapchainImages.size();
       ++backBufferIdx)
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
        if (outputs[i][0] != "Backbuffer")
        {
          ImageRef outputImage =
              ImageManager::getResourceByName(outputs[i][0].GetString());
          FramebufferManager::_descAttachedImages(framebufferRef)
              .push_back(outputImage);

          _INTR_ASSERT((dim == glm::uvec3(0u) ||
                        dim == ImageManager::_descDimensions(outputImage)) &&
                       "Dimensions of all outputs must be identical");
          dim = ImageManager::_descDimensions(outputImage);
        }
        else
        {
          ImageRef outputImage = ImageManager::getResourceByName(
              _INTR_STRING("Backbuffer") +
              StringUtil::toString<uint32_t>(backBufferIdx));
          FramebufferManager::_descAttachedImages(framebufferRef)
              .push_back(outputImage);

          _INTR_ASSERT((dim == glm::uvec3(0u) ||
                        dim == ImageManager::_descDimensions(outputImage)) &&
                       "Dimensions of all outputs must be identical");
          dim = ImageManager::_descDimensions(outputImage);
        }
      }

      FramebufferManager::_descDimensions(framebufferRef) = dim;
      FramebufferManager::_descRenderPass(framebufferRef) = _renderPassRef;
    }
    framebuffersToCreate.push_back(framebufferRef);
    _framebufferRefs.push_back(framebufferRef);
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

  _clearValues.clear();
}
}
}
}
}
