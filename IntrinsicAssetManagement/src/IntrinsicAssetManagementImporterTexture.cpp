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
#include "stdafx_assets.h"

using namespace R::Resources;
using namespace R;

namespace Intrinsic
{
namespace AssetManagement
{
namespace Importers
{
_INTR_STRING mediaPath = "media/textures";

void Texture::init() {}

// <-

void Texture::destroy() {}

// <-

void compressTexture(const _INTR_STRING& p_Args)
{
  _INTR_STRING adjustedArgs = p_Args;
  StringUtil::replace(adjustedArgs, "/", "\\\\");

  const _INTR_STRING cmd = "tools\\dxtexconv\\texconv.exe -y " + adjustedArgs;
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
                       R::Format::Enum p_Format)
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

    ImageManager::_descImageType(imageRef) = R::ImageType::kTextureFromFile;
    ImageManager::_descImageFormat(imageRef) = p_Format;
    ImageManager::_descFileName(imageRef) = p_TextureName + ".dds";

    imagesToCreate.push_back(imageRef);
  }

  ImageManager::createResources(imagesToCreate);

  return imageRef;
}

// <-

void Texture::importLinearColorTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-f BC1_UNORM -o " + mediaPath + " " + p_FilePath);
  ImageRef imgRef = createTexture(fileName, R::Format::kBC1RGBUNorm);
}

void Texture::importPbrTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-f BC5_UNORM -o " + mediaPath + " " + p_FilePath);
  ImageRef imgRef = createTexture(fileName, R::Format::kBC5UNorm);
}

// <-

void Texture::importAlbedoTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-f BC1_UNORM_SRGB -srgbi -o " + mediaPath + " " +
                  p_FilePath);
  ImageRef imgRef = createTexture(fileName, R::Format::kBC1RGBSrgb);
}

// <-

void Texture::importAlbedoAlphaTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-f BC2_UNORM_SRGB -srgbi -o " + mediaPath + " " +
                  p_FilePath);
  ImageRef imgRef = createTexture(fileName, R::Format::kBC2Srgb);
}

// <-

void Texture::importNormalMapTextureFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  compressTexture("-f BC5_UNORM -o " + mediaPath + " " + p_FilePath);
  ImageRef imgRef = createTexture(fileName, R::Format::kBC5UNorm);

  // Calc. avg. normal length for specular AA
  float avgNormalLength = 1.0f;
  {
    _INTR_STRING texturePath = "media/textures/" + fileName + ".dds";

    gli::texture2d normalTexture =
        gli::texture2d(gli::load(texturePath.c_str()));
    gli::texture2d normalTexDec =
        gli::convert(normalTexture, gli::FORMAT_RGB32_SFLOAT_PACK32);

    glm::vec3 avgNormal = glm::vec3(0.0f);
    for (int32_t y = 0u; y < normalTexDec.extent().y; ++y)
    {
      for (int32_t x = 0u; x < normalTexDec.extent().x; ++x)
      {
        gli::vec2 packedNormal =
            normalTexDec.load<gli::vec3>(gli::extent2d(x, y), 0u);
        packedNormal = packedNormal * 2.0f - 1.0f;
        gli::vec3 normal = glm::vec3(packedNormal, 0.0f);
        normal.z = std::sqrt(
            std::max(1.0f - glm::dot(packedNormal, packedNormal), 0.0f));
        _INTR_ASSERT(!glm::isnan(normal.z));

        avgNormal += normal;
      }
    }

    avgNormal /= normalTexDec.extent().x * normalTexDec.extent().y;
    avgNormalLength = glm::length(avgNormal);
  }
  ImageManager::_descAvgNormLength(imgRef) = avgNormalLength;
}

// <-

void Texture::importHdrCubemapFromFile(const _INTR_STRING& p_FilePath)
{
  _INTR_STRING fileName, extension;
  StringUtil::extractFileNameAndExtension(p_FilePath, fileName, extension);

  copyFile(p_FilePath, mediaPath + "/" + fileName + ".dds");
  ImageRef imgRef = createTexture(fileName, R::Format::kBC6UFloat);
}
}
}
}
