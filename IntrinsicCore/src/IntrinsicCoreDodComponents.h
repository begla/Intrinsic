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
namespace Core
{
namespace Dod
{
namespace Components
{
typedef Ref (*ManagerGetComponentForEntityFunction)(Ref);

// <-

struct ComponentDataBase
{
  ComponentDataBase(uint32_t p_Size) { entity.resize(p_Size); }

  _INTR_ARRAY(Entity::EntityRef) entity;
};

// <-

struct ComponentManagerEntry : ManagerEntry
{
  ComponentManagerEntry()
      : ManagerEntry(), getComponentForEntityFunction(nullptr)
  {
  }

  ManagerGetComponentForEntityFunction getComponentForEntityFunction;
};

// <-

template <class DataType, uint32_t IdCount>
struct ComponentManagerBase : Dod::ManagerBase<IdCount, DataType>
{
  typedef _INTR_HASH_MAP(uint32_t, Ref) EntityComponentMap;

  _INTR_INLINE static Ref getComponentForEntity(Entity::EntityRef p_Entity)
  {
    auto componentIt = _entityComponentMap.find(p_Entity._id);

    if (componentIt == _entityComponentMap.end())
    {
      return Dod::Ref();
    }

    return componentIt->second;
  }

  // Setter/getter
  _INTR_INLINE static Entity::EntityRef& _entity(Ref p_Ref)
  {
    return _data.entity[p_Ref._id];
  }

protected:
  _INTR_INLINE static void _initComponentManager()
  {
    Dod::ManagerBase<IdCount, DataType>::_initManager();
    _entityComponentMap.reserve(IdCount);
  }

  _INTR_INLINE static Ref _createComponent(Entity::EntityRef p_ParentEntity)
  {
    Ref ref = Dod::ManagerBase<IdCount, DataType>::allocate();
    _data.entity[ref._id] = p_ParentEntity;
    _entityComponentMap[p_ParentEntity._id] = ref;
    return ref;
  }

  _INTR_INLINE static void _destroyComponent(Ref p_Ref)
  {
    Entity::EntityRef entity = _entity(p_Ref);
    _entityComponentMap.erase(entity._id);

    Dod::ManagerBase<IdCount, DataType>::release(p_Ref);
  }

  static EntityComponentMap _entityComponentMap;
  static DataType _data;
};

template <class DataType, uint32_t IdCount>
DataType ComponentManagerBase<DataType, IdCount>::_data;
template <class DataType, uint32_t IdCount>
_INTR_HASH_MAP(uint32_t, Ref)
ComponentManagerBase<DataType, IdCount>::_entityComponentMap;
}
}
}
}
