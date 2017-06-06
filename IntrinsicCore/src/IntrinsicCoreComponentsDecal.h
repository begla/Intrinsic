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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref DecalRef;
typedef _INTR_ARRAY(DecalRef) DecalRefArray;

struct DecalData : Dod::Components::ComponentDataBase
{
  DecalData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_DECAL_COMPONENT_COUNT)
  {
    descHalfExtent.resize(_INTR_MAX_DECAL_COMPONENT_COUNT);
  }

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

  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
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
        "halfExtent",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Decal), _N(vec3),
                          _descHalfExtent(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(DecalRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("halfExtent"))
      _descHalfExtent(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["halfExtent"]);
  }

  // <-

  // Getter/Setter
  // ->

  // Description
  _INTR_INLINE static glm::vec3& _descHalfExtent(DecalRef p_Ref)
  {
    return _data.descHalfExtent[p_Ref._id];
  }
};
}
}
}
