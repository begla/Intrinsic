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
#include "stdafx_assets.h"
#include "stdafx.h"
#include "stdafx_vulkan.h"

using namespace Intrinsic::Renderer::Vulkan::Resources;
using namespace Intrinsic::Renderer::Vulkan;

namespace Intrinsic
{
namespace AssetManagement
{
_INTR_STRING mediaPath = "media/textures";

void ImporterTexture::init() {}

// <-

void ImporterTexture::destroy() {}

// <-

void compressTexture(const _INTR_STRING& p_Args)
{
  const _INTR_STRING cmd = "tools\\nvtt\\nvcompress.exe " + p_Args;
  std::system(cmd.c_str());
}

// <-

void copyFile(const _INTR_STRING& p_Source, _INTR_STRING& p_Target)
{
  std::ifstream src(p_Source.c_str(), std::ios::binary);
  std::ofstream dst(p_Target.c_str(), std::ios::binary);

  dst << src.rdbuf();
}

ImageRef createTexture(const _INTR_STRING& p_TextureName,
                       Renderer::Vulkan::Format::Enum p_Format)
{
  ImageRefArray imagesToCreate;
  ImageRefArray imagesToDestroy;

  ImageRef imageRef = ImageManager::_getResourceByName(p_TextureName);
  if (imageRef.isValid())
  {
    imagesToDestroy.push_back(imageRef);
  }
  else
  {
    imageRef = ImageManager::createImage(p_TextureName);
  }

  ImageManager::destroyResources(imagesToDestroy);

  {
    ImageManager::resetToDefault(imageRef);

    ImageManager::_descImageType(imageRef) =
        Renderer::Vulkan::ImageType::kTextureFromFile;
	ImageManager::_descImageFormat(imageRef) = p_Format;
    ImageManager::_descFileName(imageRef) = p_TextureName + ".dds";

    imagesToCreate.push_back(imageRef);
  }

  ImageManager::createResources(imagesToCreate);

  return imageRef;
}

// <-

void ImporterTexture::importColorTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc1 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC1RGBUNorm);
}

// <-

void ImporterTexture::importAlebdoTextureFromFile(
    const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc1 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC1RGBSrgb);
}

// <-

void ImporterTexture::importAlebdoAlphaTextureFromFile(
    const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc2 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef = createTexture(fileName, Renderer::Vulkan::Format::kBC2Srgb);
}

// <-

void ImporterTexture::importNormalMapTextureFromFile(
    const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc1 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC1RGBUNorm);
}

// <-

void ImporterTexture::importHdrCubemapFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  copyFile(p_FilePath, mediaPath + "/" + fileName + ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC6UFloat);
}
}
}
