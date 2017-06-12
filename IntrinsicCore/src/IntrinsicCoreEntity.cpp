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
