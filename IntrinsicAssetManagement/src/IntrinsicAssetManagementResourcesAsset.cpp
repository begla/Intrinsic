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
    assetEntry.createFunction = Resources::AssetManager::createAsset;
    assetEntry.destroyFunction = Resources::AssetManager::destroyAsset;
    assetEntry.getActiveResourceAtIndexFunction =
        Resources::AssetManager::getActiveResourceAtIndex;
    assetEntry.getActiveResourceCountFunction =
        Resources::AssetManager::getActiveResourceCount;
    assetEntry.loadFromSingleFileFunction =
        Resources::AssetManager::loadFromSingleFile;
    assetEntry.saveToSingleFileFunction =
        Resources::AssetManager::saveToSingleFile;

    Application::_resourceManagerMapping[_N(Asset)] = assetEntry;
  }

  Dod::PropertyCompilerEntry assetPropertyEntry;
  {
    assetPropertyEntry.compileFunction =
        Resources::AssetManager::compileDescriptor;
    assetPropertyEntry.initFunction =
        Resources::AssetManager::initFromDescriptor;
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
      ImporterFbx::importMeshesFromFile("assets/meshes/" +
                                        _descAssetFileName(assetRef));
      ImporterFbx::destroy();

      Core::Resources::MeshManager::saveToMultipleFiles("managers/meshes/",
                                                        ".mesh.json");
      Renderer::Vulkan::Resources::MaterialManager::saveToSingleFile(
          "managers/Material.manager.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kColorTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importTextureFromFile("assets/textures/" +
                                             _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToSingleFile(
          "managers/Image.manager.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kAlphaTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importAlphaTextureFromFile("assets/textures/" +
                                                  _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToSingleFile(
          "managers/Image.manager.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kNormalTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importNormalMapTextureFromFile(
          "assets/textures/" + _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToSingleFile(
          "managers/Image.manager.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kHdrTexture)
    {
      ImporterTexture::init();
      ImporterTexture::importHdrCubemapFromFile("assets/textures/" +
                                                _descAssetFileName(assetRef));
      ImporterTexture::destroy();

      Renderer::Vulkan::Resources::ImageManager::saveToSingleFile(
          "managers/Image.manager.json");
    }
  }
}
}
}
}
