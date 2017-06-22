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
namespace Core
{
namespace Components
{
// Typedefs
typedef Dod::Ref DecalRef;
typedef _INTR_ARRAY(DecalRef) DecalRefArray;

struct DecalData : Dod::Components::ComponentDataBase
{
  DecalData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_DECAL_COMPONENT_COUNT)
  {
    descAlbedoTextureName.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);
    descNormalTextureName.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);
    descPBRTextureName.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);

    descUVTransform.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);
    descHalfExtent.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);
  }

  // Description
  _INTR_ARRAY(Name) descAlbedoTextureName;
  _INTR_ARRAY(Name) descNormalTextureName;
  _INTR_ARRAY(Name) descPBRTextureName;

  _INTR_ARRAY(glm::vec4) descUVTransform;
  _INTR_ARRAY(glm::vec3) descHalfExtent;
};

struct DecalManager
    : Dod::Components::ComponentManagerBase<DecalData,
                                            _INTR_MAX_DECAL_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static DecalRef createDecal(Entity::EntityRef p_ParentEntity)
  {
    DecalRef ref = Dod::Components::ComponentManagerBase<
        DecalData,
        _INTR_MAX_DECAL_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(DecalRef p_Ref)
  {
    _descAlbedoTextureName(p_Ref) = _N(checkerboard);
    _descNormalTextureName(p_Ref) = _N(default_NRM);
    _descPBRTextureName(p_Ref) = _N(checkerboard_PBR);

    _descUVTransform(p_Ref) = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
    _descHalfExtent(p_Ref) = glm::vec3(2.0f, 2.0f, 2.0f);
  }

  // <-

  _INTR_INLINE static void destroyDecal(DecalRef p_Decal)
  {
    Dod::Components::ComponentManagerBase<
        DecalData, _INTR_MAX_DECAL_COMPONENT_COUNT>::_destroyComponent(p_Decal);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(DecalRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "albedoTextureName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(string),
                          _descAlbedoTextureName(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "normalTextureName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(string),
                          _descNormalTextureName(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "pbrTextureName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(string),
                          _descPBRTextureName(p_Ref), false, false),
        p_Document.GetAllocator());

    p_Properties.AddMember(
        "uvTransform",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(vec4),
                          _descUVTransform(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "halfExtent",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(vec3),
                          _descHalfExtent(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(DecalRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("albedoTextureName"))
      _descAlbedoTextureName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["albedoTextureName"]);
    if (p_Properties.HasMember("normalTextureName"))
      _descNormalTextureName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["normalTextureName"]);
    if (p_Properties.HasMember("pbrTextureName"))
      _descPBRTextureName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["pbrTextureName"]);

    if (p_Properties.HasMember("uvTransform"))
      _descUVTransform(p_Ref) =
          JsonHelper::readPropertyVec4(p_Properties["uvTransform"]);
    if (p_Properties.HasMember("halfExtent"))
      _descHalfExtent(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["halfExtent"]);
  }

  // <-

  // Description
  _INTR_INLINE static Name& _descAlbedoTextureName(DecalRef p_Ref)
  {
    return _data.descAlbedoTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descNormalTextureName(DecalRef p_Ref)
  {
    return _data.descNormalTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descPBRTextureName(DecalRef p_Ref)
  {
    return _data.descPBRTextureName[p_Ref._id];
  }

  _INTR_INLINE static glm::vec4& _descUVTransform(DecalRef p_Ref)
  {
    return _data.descUVTransform[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3& _descHalfExtent(DecalRef p_Ref)
  {
    return _data.descHalfExtent[p_Ref._id];
  }
};
}
}
}
