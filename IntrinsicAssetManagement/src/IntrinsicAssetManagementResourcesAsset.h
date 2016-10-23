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

#pragma once

namespace Intrinsic
{
namespace AssetManagement
{
namespace Resources
{
typedef Dod::Ref AssetRef;
typedef _INTR_ARRAY(AssetRef) AssetRefArray;

namespace AssetType
{
enum Enum
{
  kNone,
  kMesh,
  kColorTexture,
  kAlphaTexture,
  kNormalTexture,
  kHdrTexture
};
}

struct AssetData : Dod::Resources::ResourceDataBase
{
  AssetData() : Dod::Resources::ResourceDataBase(_INTR_MAX_ASSET_COUNT)
  {
    descAssetFileName.resize(_INTR_MAX_ASSET_COUNT);
    descAssetType.resize(_INTR_MAX_ASSET_COUNT);
  }

  // <-

  _INTR_ARRAY(_INTR_STRING) descAssetFileName;
  _INTR_ARRAY(AssetType::Enum) descAssetType;
};

struct AssetManager
    : Dod::Resources::ResourceManagerBase<AssetData, _INTR_MAX_ASSET_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static AssetRef createAsset(const Name& p_Name)
  {
    AssetRef ref = Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(AssetRef p_Ref)
  {
    _descAssetFileName(p_Ref) = "";
    _descAssetType(p_Ref) = AssetType::kNone;
  }

  // <-

  _INTR_INLINE static void destroyAsset(AssetRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(AssetRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_compileDescriptor(p_Ref,
                                                              p_Properties,
                                                              p_Document);

    p_Properties.AddMember("assetFileName",
                           _INTR_CREATE_PROP(p_Document, _N(Asset), _N(string),
                                             _descAssetFileName(p_Ref), false,
                                             false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "assetType",
        _INTR_CREATE_PROP_ENUM(
            p_Document, _N(Asset), _N(enum), _descAssetType(p_Ref),
            "None,Mesh,ColorTexture,AlphaTexture,NormalTexture,HdrTexture",
            false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(AssetRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_initFromDescriptor(p_Ref,
                                                               p_Properties);

    if (p_Properties.HasMember("assetFileName"))
      _descAssetFileName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["assetFileName"]);
    if (p_Properties.HasMember("assetType"))
      _descAssetType(p_Ref) = (AssetType::Enum)JsonHelper::readPropertyEnum(
          p_Properties["assetType"]);
  }

  // <-

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_saveToSingleFile(p_FileName,
                                                             compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<AssetData, _INTR_MAX_ASSET_COUNT>::
        _loadFromSingleFile(p_FileName, initFromDescriptor, resetToDefault);
  }

  // <-

  static void compileAssets(AssetRefArray& p_Refs);

  // Getter/Setter
  // Intrinsic

  // <-

  // Description
  _INTR_INLINE static _INTR_STRING& _descAssetFileName(AssetRef p_Ref)
  {
    return _data.descAssetFileName[p_Ref._id];
  }
  _INTR_INLINE static AssetType::Enum& _descAssetType(AssetRef p_Ref)
  {
    return _data.descAssetType[p_Ref._id];
  }
};
}
}
}
