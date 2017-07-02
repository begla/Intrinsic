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

using namespace RResources;
using namespace CResources;

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

    if (_descAssetType(assetRef) == AssetType::kMesh ||
        _descAssetType(assetRef) == AssetType::kMeshAndPhysicsMesh)
    {
      Importers::Fbx::init();

      _INTR_ARRAY(MeshRef) importedMeshes;
      Importers::Fbx::importMeshesFromFile(Settings::Manager::_assetMeshPath +
                                               "/" +
                                               _descAssetFileName(assetRef),
                                           importedMeshes);
      Importers::Fbx::destroy();

      // Create mesh resources
      CResources::MeshManager::createResources(importedMeshes);

      for (uint32_t i = 0u; i < importedMeshes.size(); ++i)
      {
        MeshManager::saveToMultipleFilesSingleResource(
            importedMeshes[i], "managers/meshes/", ".mesh.json");
      }

      MaterialManager::saveToMultipleFiles("managers/materials/",
                                           ".material.json");

      if (_descAssetType(assetRef) == AssetType::kMeshAndPhysicsMesh)
      {
        Processors::Physics::createPhysicsTriangleMeshes(importedMeshes);
        Processors::Physics::createPhysicsConvexMeshes(importedMeshes);

        // Recreate mesh resources again to init. physics resources
        CResources::MeshManager::destroyResources(importedMeshes);
        CResources::MeshManager::createResources(importedMeshes);
      }

      Components::MeshRefArray meshComponentsToRecreate;
      Components::RigidBodyRefArray rigidBodyComponentsToRecreate;

      for (MeshRef importedMeshRef : importedMeshes)
      {
        for (uint32_t i = 0u;
             i < CComponents::MeshManager::getActiveResourceCount(); ++i)
        {
          Components::MeshRef meshCompRef =
              CComponents::MeshManager::getActiveResourceAtIndex(i);

          if (CComponents::MeshManager::_descMeshName(meshCompRef) ==
              MeshManager::_name(importedMeshRef))
          {
            meshComponentsToRecreate.push_back(meshCompRef);

            CComponents::RigidBodyRef rigidBodyRef =
                CComponents::RigidBodyManager::getComponentForEntity(
                    CComponents::MeshManager::_entity(meshCompRef));

            if (rigidBodyRef.isValid())
            {
              rigidBodyComponentsToRecreate.push_back(rigidBodyRef);
            }
          }
        }
      }

      CComponents::MeshManager::destroyResources(meshComponentsToRecreate);
      CComponents::MeshManager::createResources(meshComponentsToRecreate);
      CComponents::RigidBodyManager::destroyResources(
          rigidBodyComponentsToRecreate);
      CComponents::RigidBodyManager::createResources(
          rigidBodyComponentsToRecreate);
    }
    else if (_descAssetType(assetRef) == AssetType::kLinearColorTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importLinearColorTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kPbrTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importPbrTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kAlbedoTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importAlbedoTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kAlbedoAlphaTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importAlbedoAlphaTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kNormalTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importNormalMapTextureFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
    else if (_descAssetType(assetRef) == AssetType::kHdrTexture)
    {
      Importers::Texture::init();
      Importers::Texture::importHdrCubemapFromFile(
          Settings::Manager::_assetTexturePath + "/" +
          _descAssetFileName(assetRef));
      Importers::Texture::destroy();

      ImageManager::saveToMultipleFiles("managers/images/", ".image.json");
    }
  }
}
}
}
}
