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
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Entity
{
// Static members
EntityData EntityManager::_data;
_INTR_HASH_MAP(Name, Dod::Ref) EntityManager::_nameResourceMap;

// <-

void EntityManager::init()
{
  _INTR_LOG_INFO("Inititializing Entity Manager...");

  Dod::ManagerBase<_INTR_MAX_ENTITY_COUNT, EntityData>::_initManager();

  Dod::PropertyCompilerEntry propCompilerEntity;
  {
    propCompilerEntity.compileFunction =
        Entity::EntityManager::compileDescriptor;
    propCompilerEntity.initFunction = Entity::EntityManager::initFromDescriptor;
    propCompilerEntity.ref = Dod::Ref();

    Application::_componentPropertyCompilerMapping[_N(Entity)] =
        propCompilerEntity;
  }
}

// <-

void EntityManager::destroyAllComponents(EntityRefArray p_Refs)
{
  for (uint32_t i = 0u; i < p_Refs.size(); ++i)
  {
    EntityRef entity = p_Refs[i];

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      Dod::Components::ComponentManagerEntry& managerEntry = it->second;
      _INTR_ASSERT(managerEntry.getComponentForEntityFunction);

      if (managerEntry.destroyFunction)
      {
        Dod::Ref componentRef =
            managerEntry.getComponentForEntityFunction(entity);

        if (componentRef.isValid())
        {
          managerEntry.destroyFunction(componentRef);
        }
      }
    }
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

// <-

void EntityManager::destroyAllResources(EntityRefArray p_Refs)
{
  for (uint32_t i = 0u; i < p_Refs.size(); ++i)
  {
    EntityRef entity = p_Refs[i];

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      Dod::Components::ComponentManagerEntry& managerEntry = it->second;
      _INTR_ASSERT(managerEntry.getComponentForEntityFunction);

      if (managerEntry.destroyResourcesFunction)
      {
        Dod::Ref componentRef =
            managerEntry.getComponentForEntityFunction(entity);

        if (componentRef.isValid())
        {
          Dod::RefArray refs;
          refs.push_back(componentRef);

          managerEntry.destroyResourcesFunction(refs);
        }
      }
    }
  }
}

// <-

void EntityManager::createAllResources(EntityRefArray p_Refs)
{
  for (uint32_t i = 0u; i < p_Refs.size(); ++i)
  {
    EntityRef entity = p_Refs[i];

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      Dod::Components::ComponentManagerEntry& managerEntry = it->second;
      _INTR_ASSERT(managerEntry.getComponentForEntityFunction);

      if (managerEntry.createResourcesFunction)
      {
        Dod::Ref componentRef =
            managerEntry.getComponentForEntityFunction(entity);

        if (componentRef.isValid())
        {
          Dod::RefArray refs = {componentRef};
          managerEntry.createResourcesFunction(refs);
        }
      }
    }
  }
}
}
}
}
