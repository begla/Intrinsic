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
namespace Entity
{
typedef Dod::Ref EntityRef;
typedef _INTR_ARRAY(EntityRef) EntityRefArray;

struct EntityData
{
  EntityData() { name.resize(_INTR_MAX_ENTITY_COUNT); }

  _INTR_ARRAY(Name) name;
};

struct EntityManager : Dod::ManagerBase<_INTR_MAX_ENTITY_COUNT, EntityData>
{
  static void init();

  _INTR_INLINE static EntityRef createEntity(const Name& p_Name = 0u)
  {
    EntityRef ref = allocate();
    _name(ref) = p_Name;
    _nameResourceMap[p_Name] = ref;
    return ref;
  }

  _INTR_INLINE static void destroyEntity(EntityRef p_Ref)
  {
    _nameResourceMap.erase(_name(p_Ref));
    release(p_Ref);
  }

  static void destroyAllComponents(EntityRefArray p_Refs);
  static void destroyAllResources(EntityRefArray p_Refs);
  static void createAllResources(EntityRefArray p_Refs);

  _INTR_INLINE static void compileDescriptor(EntityRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember("name",
                           _INTR_CREATE_PROP(p_Document, _N(Entity), _N(string),
                                             _name(p_Ref), false, false),
                           p_Document.GetAllocator());
  }

  _INTR_INLINE static void initFromDescriptor(EntityRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("name"))
      _name(p_Ref) = JsonHelper::readPropertyName(p_Properties["name"]);
  }

  _INTR_INLINE static EntityRef getEntityByName(const Name& p_Name)
  {
    return _nameResourceMap[p_Name];
  }

  _INTR_INLINE static Name makeNameUnique(const char* p_Name)
  {
    Name newEntityName = p_Name;
    uint32_t nodexIndex = 1u;

    while (true)
    {
      if (_nameResourceMap.find(newEntityName) != _nameResourceMap.end())
      {
        const _INTR_STRING nameWithoutSuffix =
            StringUtil::stripNumberSuffix(p_Name);
        newEntityName =
            nameWithoutSuffix +
            Intrinsic::Core::StringUtil::toString<uint32_t>(nodexIndex++)
                .c_str();
      }
      else
      {
        break;
      }
    }

    return newEntityName;
  }

  // Getter/Setter
  // Intrinsic

  _INTR_INLINE static Name& _name(EntityRef p_Ref)
  {
    return _data.name[p_Ref._id];
  }

  // Intrinsic

  static EntityData _data;
  static _INTR_HASH_MAP(Name, Dod::Ref) _nameResourceMap;
};
}
}
}
