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
  kAlbedoTexture,
  kAlbedoAlphaTexture,
  kLinearColorTexture,
  kNormalTexture,
  kHdrTexture,

  kCount,
  kTexturesBegin = kAlbedoTexture,
  kTexturesEnd = kHdrTexture
};
}

struct AssetData : Dod::Resources::ResourceDataBase
{
  AssetData() : Dod::Resources::ResourceDataBase(_INTR_MAX_ASSET_COUNT)
  {
    descAssetFileName.resize(_INTR_MAX_ASSET_COUNT);
    descAssetType.resize(_INTR_MAX_ASSET_COUNT);
  }

  // Description
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
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        AssetData, _INTR_MAX_ASSET_COUNT>::_compileDescriptor(p_Ref,
                                                              p_GenerateDesc,
                                                              p_Properties,
                                                              p_Document);

    p_Properties.AddMember(
        "assetFileName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Asset), _N(string),
                          _descAssetFileName(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "assetType",
        _INTR_CREATE_PROP_ENUM(p_Document, p_GenerateDesc, _N(Asset), _N(enum),
                               _descAssetType(p_Ref),
                               "None,Mesh,AlbedoTexture,AlbedoAlphaTexture,"
                               "LinearColorTexture,NormalTexture,HDRTexture",
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
      _descAssetType(p_Ref) = (AssetType::Enum)JsonHelper::readPropertyEnumUint(
          p_Properties["assetType"]);
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<AssetData, _INTR_MAX_ASSET_COUNT>::
        _saveToMultipleFiles<
            rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<AssetData, _INTR_MAX_ASSET_COUNT>::
        _loadFromMultipleFiles(p_Path, p_Extension, initFromDescriptor,
                               resetToDefault);
  }

  // <-

  static void compileAssets(AssetRefArray& p_Refs);

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
