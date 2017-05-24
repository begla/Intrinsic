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

namespace Intrinsic
{
namespace AssetManagement
{
namespace Resources
{
void AssetManager::init()
{
  _INTR_LOG_INFO("Inititializing Asset Manager...");

  Dod::Resources::ResourceManagerBase<
      AssetData, _INTR_MAX_ASSET_COUNT>::_initResourceManager();

  Dod::Resources::ResourceManagerEntry assetEntry;
  {
    assetEntry.createFunction = AssetManager::createAsset;
    assetEntry.destroyFunction = AssetManager::destroyAsset;
    assetEntry.getActiveResourceAtIndexFunction =
        AssetManager::getActiveResourceAtIndex;
    assetEntry.getActiveResourceCountFunction =
        AssetManager::getActiveResourceCount;
    assetEntry.loadFromMultipleFilesFunction =
        AssetManager::loadFromMultipleFiles;
    assetEntry.saveToMultipleFilesFunction = AssetManager::saveToMultipleFiles;
    assetEntry.getResourceFlagsFunction = AssetManager::_resourceFlags;

    Application::_resourceManagerMapping[_N(Asset)] = assetEntry;
  }

  Dod::PropertyCompilerEntry assetPropertyEntry;
  {
    assetPropertyEntry.compileFunction = AssetManager::compileDescriptor;
    assetPropertyEntry.initFunction = AssetManager::initFromDescriptor;
    assetPropertyEntry.ref = Dod::Ref();

    Application::_resourcePropertyCompilerMapping[_N(Asset)] =
        assetPropertyEntry;
  }
}

// <-

void AssetManager::compileAssets(AssetRefArray& p_Refs)
{
  for (uint32_t assetIdx = 0u; assetIdx < p_Refs.size(); ++assetIdx)
  {
    AssetRef assetRef = p_Refs[assetIdx];

    if (_descAssetType(assetRef) == AssetType::kMesh)
    {
      ImporterFbx::init();

      _INTR_ARRAY(Core::Resources::MeshRef) importedMeshes;
      ImporterFbx::importMeshesFromFile(Settings::Manager::_assetMeshPath +
                                            "/" + _descAssetFileName(assetRef),
                                        importedMeshes);
      ImporterFbx::destroy();

      for (uint32_t i = 0u; i < importedMeshes.size(); ++i)
      {
        Core::Resources::MeshManager::saveToMultipleFilesSingleResource(
            importedMeshes[i], "managers/meshes/", ".mesh.json");
      }

      Renderer::Vulkan::Resources::MaterialManager::saveToMultipleFiles(
          "managers/materials/", ".material.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kLinearColorTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importColorTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToMultipleFiles(
          "managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kAlbedoTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importAlebdoAlphaTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToMultipleFiles(
          "managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kAlbedoAlphaTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importAlebdoAlphaTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToMultipleFiles(
          "managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kNormalTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importNormalMapTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToMultipleFiles(
          "managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kHdrTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importHdrCubemapFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToMultipleFiles(
          "managers/images/", ".image.json");
    }
  }
}
}
}
}
