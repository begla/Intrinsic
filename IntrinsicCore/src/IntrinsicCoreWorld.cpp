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

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
Components::NodeRef World::_rootNode;
Components::CameraRef World::_activeCamera;

uint32_t calcOffsetToParent(const Components::NodeRefArray& p_Nodes,
                            Components::NodeRef p_Parent,
                            uint32_t p_CurrentIndex)
{
  int32_t offsetToParent = 0;
  for (uint32_t i = 0u; i < p_Nodes.size(); ++i)
  {
    if (p_Nodes[i] == p_Parent)
    {
      offsetToParent = i - p_CurrentIndex;
      break;
    }
  }
  _INTR_ASSERT(offsetToParent != 0);

  return offsetToParent;
}

void World::init()
{
  _INTR_ASSERT(!_rootNode.isValid() && "World already init.");

  // Create root node
  Entity::EntityRef entityRef =
      Entity::EntityManager::createEntity(_N(WorldRoot));
  {
    _rootNode = Components::NodeManager::createNode(entityRef);
  }
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

Components::NodeRef World::cloneNodeFull(Components::NodeRef p_Ref)
{
  rapidjson::Document doc;

  // Collect entities
  Components::NodeRefArray referenceNodes;
  Components::NodeManager::collectNodes(p_Ref, referenceNodes);

  Components::NodeRefArray clonedNodes;

  for (uint32_t i = 0u; i < referenceNodes.size(); ++i)
  {
    Components::NodeRef referenceNodeRef = referenceNodes[i];
    Entity::EntityRef referenceEntityRef =
        Components::NodeManager::_entity(referenceNodes[i]);

    Entity::EntityRef clonedEntityRef = Entity::EntityManager::createEntity(
        Entity::EntityManager::_name(referenceEntityRef).getString().c_str());

    for (auto propCompIt =
             Application::_componentPropertyCompilerMapping.begin();
         propCompIt != Application::_componentPropertyCompilerMapping.end();
         ++propCompIt)
    {
      rapidjson::Value componentType = rapidjson::Value(
          propCompIt->first.getString().c_str(), doc.GetAllocator());

      auto compManagerEntryIt =
          Application::_componentManagerMapping.find(componentType.GetString());
      if (compManagerEntryIt != Application::_componentManagerMapping.end())
      {
        Dod::Components::ComponentManagerEntry& managerEntry =
            compManagerEntryIt->second;

        _INTR_ASSERT(managerEntry.getComponentForEntityFunction);

        if (managerEntry.getComponentForEntityFunction(referenceEntityRef)
                .isValid())
        {
          // Compile reference component
          rapidjson::Value properties =
              rapidjson::Value(rapidjson::kObjectType);
          _INTR_ASSERT(propCompIt->second.compileFunction);
          propCompIt->second.compileFunction(
              managerEntry.getComponentForEntityFunction(referenceEntityRef),
              false, properties, doc);

          // Create new component and init from reference
          _INTR_ASSERT(managerEntry.createFunction);
          Dod::Ref newCompRef = managerEntry.createFunction(clonedEntityRef);

          if (managerEntry.resetToDefaultFunction)
            managerEntry.resetToDefaultFunction(newCompRef);

          _INTR_ASSERT(propCompIt->second.initFunction);
          propCompIt->second.initFunction(newCompRef, properties);
        }
      }
    }

    Components::NodeRef clonedNodeRef =
        Components::NodeManager::getComponentForEntity(clonedEntityRef);

    // Create hierarchy
    if (Components::NodeManager::_parent(referenceNodeRef).isValid())
    {
      if (i == 0u)
      {
        Components::NodeManager::attachChildIgnoreParent(
            Components::NodeManager::_parent(referenceNodeRef), clonedNodeRef);
      }
      else
      {
        uint32_t offsetToParent = calcOffsetToParent(
            referenceNodes, Components::NodeManager::_parent(referenceNodeRef),
            (uint32_t)clonedNodes.size());
        Components::NodeManager::attachChildIgnoreParent(
            clonedNodes[i + offsetToParent], clonedNodeRef);
      }
    }

    clonedNodes.push_back(clonedNodeRef);
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  // Load resources in order
  {
    for (uint32_t i = 0u; i < clonedNodes.size(); ++i)
    {
      const Components::NodeRef nodeRef = clonedNodes[i];
      const Entity::EntityRef entityRef =
          Components::NodeManager::_entity(nodeRef);

      for (uint32_t managerIdx = 0u;
           managerIdx < Application::_orderedComponentManagers.size();
           ++managerIdx)
      {
        Dod::Components::ComponentManagerEntry& managerEntry =
            Application::_orderedComponentManagers[managerIdx];
        Dod::Ref compRef =
            managerEntry.getComponentForEntityFunction(entityRef);

        if (compRef.isValid() && managerEntry.createResourcesFunction)
        {
          Dod::RefArray componentsToInit = {compRef};
          managerEntry.createResourcesFunction(componentsToInit);
        }
      }
    }
  }

  return clonedNodes[0];
}

void World::destroyNodeFull(Components::NodeRef p_Ref)
{
  // Collect entities
  Entity::EntityRefArray entities;
  Components::NodeManager::collectEntities(p_Ref, entities);

  // Cleanup components and entities
  {
    Entity::EntityManager::destroyAllResources(entities);
    Entity::EntityManager::destroyAllComponents(entities);

    for (uint32_t i = 0u; i < entities.size(); ++i)
    {
      Entity::EntityManager::destroyEntity(entities[i]);
    }
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void World::destroy()
{
  Components::NodeRef currentRootNode = _rootNode;
  _rootNode = Components::NodeRef();
  destroyNodeFull(currentRootNode);
}

void World::save(const _INTR_STRING& p_FilePath)
{
  _INTR_LOG_INFO("Saving world to file '%s'...", p_FilePath.c_str());

  rapidjson::Document worldDesc = rapidjson::Document(rapidjson::kArrayType);

  _INTR_ARRAY(Components::NodeRef) storedNodes;

  Components::NodeRef nodeStack[64];
  uint32_t nodeStackCount = 1u;
  nodeStack[0] = _rootNode;

  while (nodeStackCount > 0u)
  {
    _INTR_ASSERT(nodeStackCount <= 64u);

    --nodeStackCount;
    Components::NodeRef currentNodeRef = nodeStack[nodeStackCount];

    Entity::EntityRef entityRef =
        Components::NodeManager::_entity(currentNodeRef);

    Components::NodeRef parent =
        Components::NodeManager::_parent(currentNodeRef);
    Components::NodeRef firstChild =
        Components::NodeManager::_firstChild(currentNodeRef);
    Components::NodeRef nextSibling =
        Components::NodeManager::_nextSibling(currentNodeRef);

    if (firstChild.isValid())
    {
      nodeStack[nodeStackCount] = firstChild;
      ++nodeStackCount;
    }
    if (_rootNode != currentNodeRef && nextSibling.isValid())
    {
      nodeStack[nodeStackCount] = nextSibling;
      ++nodeStackCount;
    }

    // Don't serialize spawned objects
    if ((Components::NodeManager::_flags(currentNodeRef) &
         Components::NodeFlags::Flags::kSpawned) == 0u)
    {
      rapidjson::Value node = rapidjson::Value(rapidjson::kObjectType);
      rapidjson::Value name = rapidjson::Value(
          Entity::EntityManager::_name(entityRef).getString().c_str(),
          worldDesc.GetAllocator());

      node.AddMember("name", name, worldDesc.GetAllocator());

      const int32_t offsetToParent =
          parent.isValid() ? calcOffsetToParent(storedNodes, parent,
                                                (uint32_t)storedNodes.size())
                           : 0;
      node.AddMember("offsetToParent", offsetToParent,
                     worldDesc.GetAllocator());

      rapidjson::Value propertyEntries =
          rapidjson::Value(rapidjson::kArrayType);

      for (auto propCompIt =
               Application::_componentPropertyCompilerMapping.begin();
           propCompIt != Application::_componentPropertyCompilerMapping.end();
           ++propCompIt)
      {
        rapidjson::Value componentType = rapidjson::Value(
            propCompIt->first.getString().c_str(), worldDesc.GetAllocator());

        auto compManagerEntryIt = Application::_componentManagerMapping.find(
            componentType.GetString());
        if (compManagerEntryIt != Application::_componentManagerMapping.end())
        {
          Dod::Components::ComponentManagerEntry& managerEntry =
              compManagerEntryIt->second;

          _INTR_ASSERT(managerEntry.getComponentForEntityFunction);

          if (managerEntry.getComponentForEntityFunction(entityRef).isValid())
          {
            rapidjson::Value properties =
                rapidjson::Value(rapidjson::kObjectType);
            propCompIt->second.compileFunction(
                managerEntry.getComponentForEntityFunction(entityRef), false,
                properties, worldDesc);

            rapidjson::Value propertyEntry =
                rapidjson::Value(rapidjson::kObjectType);
            propertyEntry.AddMember("type", componentType,
                                    worldDesc.GetAllocator());
            propertyEntry.AddMember("properties", properties,
                                    worldDesc.GetAllocator());
            propertyEntries.PushBack(propertyEntry, worldDesc.GetAllocator());
          }
        }
      }

      node.AddMember("propertyEntries", propertyEntries,
                     worldDesc.GetAllocator());
      worldDesc.PushBack(node, worldDesc.GetAllocator());
      storedNodes.push_back(currentNodeRef);
    }
  }

  FILE* fp = fopen(p_FilePath.c_str(), "wb");

  if (fp == nullptr)
  {
    _INTR_LOG_ERROR("Failed to save world to file '%s'...", p_FilePath.c_str());
    return;
  }

  char* writeBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileWriteStream os(fp, writeBuffer, 65536u);
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    worldDesc.Accept(writer);
    fclose(fp);
  }
  Tlsf::MainAllocator::free(writeBuffer);
}

void World::load(const _INTR_STRING& p_FilePath)
{
  _INTR_LOG_INFO("Loading world from file '%s'...", p_FilePath.c_str());

  // Parse world
  rapidjson::Document worldDesc;
  {
    FILE* fp = fopen(p_FilePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_ERROR("Failed to load world from file '%s'...",
                      p_FilePath.c_str());
      return;
    }

    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      worldDesc.ParseStream(is);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(readBuffer);
  }

  destroy();

  _INTR_ARRAY(Components::NodeRef) worldNodes;

  // Init. nodes
  {
    for (uint32_t i = 0u; i < worldDesc.Size(); ++i)
    {
      rapidjson::Value& node = worldDesc[i];
      Entity::EntityRef entityRef =
          Entity::EntityManager::createEntity(node["name"].GetString());
      rapidjson::Value& propertyEntries = node["propertyEntries"];

      for (auto it = propertyEntries.Begin(); it != propertyEntries.End(); ++it)
      {
        rapidjson::Value& propertyEntry = *it;
        rapidjson::Value& componentType = propertyEntry["type"];

        auto compEntryIt = Application::_componentPropertyCompilerMapping.find(
            componentType.GetString());
        if (compEntryIt != Application::_componentPropertyCompilerMapping.end())
        {
          Dod::Components::ComponentManagerEntry& managerEntry =
              Application::_componentManagerMapping[componentType.GetString()];

          Dod::Ref componentRef = managerEntry.createFunction(entityRef);

          if (managerEntry.resetToDefaultFunction)
          {
            managerEntry.resetToDefaultFunction(componentRef);
          }

          compEntryIt->second.initFunction(componentRef,
                                           propertyEntry["properties"]);

          if (strcmp(componentType.GetString(), "Node") == 0u)
          {
            worldNodes.push_back(componentRef);
          }
        }
        else
        {
          _INTR_LOG_WARNING(
              "Unknown component type %s encountered. Skipping...",
              componentType.GetString());
        }
      }
    }
  }

  // Restore hierarchy
  {
    for (uint32_t i = 0u; i < worldNodes.size(); ++i)
    {
      const Components::NodeRef nodeRef = worldNodes[i];
      rapidjson::Value& node = worldDesc[i];

      const int32_t offsetToParent = node["offsetToParent"].GetInt();

      if (offsetToParent != 0)
      {
        Components::NodeManager::attachChildIgnoreParent(
            worldNodes[i + offsetToParent], nodeRef);
      }
    }
  }
  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  // Finally set the root node
  _rootNode = worldNodes[0];

  // Set default camera
  _activeCamera = Components::CameraManager::getComponentForEntity(
      Entity::EntityManager::getEntityByName(_N(MainCamera)));

  // Load component resources in order
  {
    for (uint32_t i = 0u; i < worldNodes.size(); ++i)
    {
      const Components::NodeRef nodeRef = worldNodes[i];
      const Entity::EntityRef entityRef =
          Components::NodeManager::_entity(nodeRef);

      for (uint32_t managerIdx = 0u;
           managerIdx < Application::_orderedComponentManagers.size();
           ++managerIdx)
      {
        Dod::Components::ComponentManagerEntry& managerEntry =
            Application::_orderedComponentManagers[managerIdx];
        Dod::Ref compRef =
            managerEntry.getComponentForEntityFunction(entityRef);

        if (compRef.isValid() && managerEntry.createResourcesFunction)
        {
          Dod::RefArray componentsToInit = {compRef};
          managerEntry.createResourcesFunction(componentsToInit);
        }
      }
    }
  }

  GameStates::Editing::_currentlySelectedEntity =
      Components::NodeManager::_entity(_rootNode);
  Resources::EventManager::queueEventIfNotExisting(
      _N(CurrentlySelectedEntityChanged));
}
}
}
