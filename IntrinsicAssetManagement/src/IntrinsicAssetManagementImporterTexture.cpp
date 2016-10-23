// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
    ImageManager::_descFileName(imageRef) = p_TextureName + ".dds";
    ImageManager::_descImageFormat(imageRef) = p_Format;

    imagesToCreate.push_back(imageRef);
  }

  ImageManager::createResources(imagesToCreate);

  return imageRef;
}

void updateDrawCalls(ImageRef p_ImageRef)
{
  _INTR_ARRAY(DrawCallRef) drawCallsToUpdate;

  const uint32_t activeResourceCount =
      DrawCallManager::getActiveResourceCount();

  for (uint32_t dcIdx = 0u; dcIdx < activeResourceCount; ++dcIdx)
  {
    DrawCallRef dcRef = DrawCallManager::getActiveResourceAtIndex(dcIdx);

    _INTR_ARRAY(BindingInfo)& bindInfos =
        DrawCallManager::_descBindInfos(dcRef);

    for (uint32_t biIdx = 0u; biIdx < (uint32_t)bindInfos.size(); ++biIdx)
    {
      BindingInfo& bindInfo = bindInfos[biIdx];

      if (bindInfo.resource == p_ImageRef)
      {
        drawCallsToUpdate.push_back(dcRef);
      }
    }
  }

  DrawCallManager::destroyResources(drawCallsToUpdate);
  DrawCallManager::createResources(drawCallsToUpdate);
}

void ImporterTexture::importTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc1 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC1RGBUNorm);
  updateDrawCalls(imgRef);
}

// <-

void ImporterTexture::importAlphaTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-bc2 " + p_FilePath + " " + mediaPath + "/" + fileName +
                  ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC2UNorm);
  updateDrawCalls(imgRef);
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
  updateDrawCalls(imgRef);
}

// <-

void ImporterTexture::importHdrCubemapFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  copyFile(p_FilePath, mediaPath + "/" + fileName + ".dds");
  ImageRef imgRef =
      createTexture(fileName, Renderer::Vulkan::Format::kBC6UFloat);
  updateDrawCalls(imgRef);
}
}
}
