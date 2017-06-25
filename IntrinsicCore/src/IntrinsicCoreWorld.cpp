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
Components::NodeRef World::_rootNode;
Components::CameraRef World::_activeCamera;
_INTR_STRING World::_filePath;
uint32_t World::_flags = 0u;

float World::_currentTime = 0.1f;
float World::_currentDayNightFactor = 0.0f;
glm::quat World::_currentSunLightOrientation = glm::quat();
glm::vec4 World::_currentSunLightColorAndIntensity = glm::vec4();

namespace
{
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
  return offsetToParent;
}
}

// <-

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
          propCompIt->second.initFunction(newCompRef, false, properties);
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
  loadNodeResources(clonedNodes[0]);

  return clonedNodes[0];
}

// <-

void World::alignNodeWithGround(Components::NodeRef p_NodeRef)
{
  Entity::EntityRef entityRef = Components::NodeManager::_entity(p_NodeRef);

  physx::PxRaycastHit hit;
  Math::Ray ray = {glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f)};

  Components::DecalRef decalRef =
      Components::DecalManager::getComponentForEntity(entityRef);

  // Special handling for decals
  if (decalRef.isValid())
  {
    ray.d = Components::NodeManager::_worldOrientation(p_NodeRef) *
            glm::vec3(0.0f, 0.0f, -1.0f);
  }
  ray.o = Components::NodeManager::_worldPosition(p_NodeRef) - ray.d * 5.0f;

  if (PhysicsHelper::raycast(ray, hit, 1000.0f))
  {
    if (!decalRef.isValid())
    {
      const glm::vec3 hitNormal =
          glm::vec3(hit.normal.x, hit.normal.y, hit.normal.z);

      Components::NodeManager::updateFromWorldPosition(
          p_NodeRef, glm::vec3(hit.position.x, hit.position.y, hit.position.z));
      const glm::vec3 currentNormal =
          Components::NodeManager::_worldOrientation(p_NodeRef) *
          glm::vec3(0.0f, 1.0f, 0.0f);
      Components::NodeManager::updateFromWorldOrientation(
          p_NodeRef, glm::normalize(glm::rotation(currentNormal, hitNormal) *
                                    Components::NodeManager::_worldOrientation(
                                        p_NodeRef)));
    }
    else
    {
      const glm::vec3 hitNormal =
          glm::vec3(hit.normal.x, hit.normal.y, hit.normal.z);

      Components::NodeManager::updateFromWorldPosition(
          p_NodeRef,
          glm::vec3(hit.position.x, hit.position.y, hit.position.z) +
              hitNormal *
                  Components::DecalManager::_descHalfExtent(decalRef).z);

      const glm::vec3 currentNormal =
          Components::NodeManager::_worldOrientation(p_NodeRef) *
          glm::vec3(0.0f, 0.0f, 1.0f);
      Components::NodeManager::updateFromWorldOrientation(
          p_NodeRef, glm::normalize(glm::rotation(currentNormal, hitNormal) *
                                    Components::NodeManager::_worldOrientation(
                                        p_NodeRef)));
    }

    Components::NodeManager::updateTransforms(p_NodeRef);
  }
}

// <-

void World::destroyNodeFull(Components::NodeRef p_NodeRef)
{
  // Collect entities
  Entity::EntityRefArray entities;
  Components::NodeManager::collectEntities(p_NodeRef, entities);

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

// <-

void World::destroy()
{
  _flags |= WorldFlags::kLoadingUnloading;
  destroyNodeFull(_rootNode);
  _rootNode = Components::NodeRef();
  _flags &= ~WorldFlags::kLoadingUnloading;
}

// <-

void World::save(const _INTR_STRING& p_FilePath)
{
  _INTR_LOG_INFO("Saving world to file '%s'...", p_FilePath.c_str());

  saveNodeHierarchy(p_FilePath, _rootNode);
}

// <-

void World::saveNodeHierarchy(const _INTR_STRING& p_FilePath,
                              Components::NodeRef p_RootNodeRef)
{
  _INTR_ASSERT(p_RootNodeRef.isValid() && "Invalid node provided");

  rapidjson::Document saveDesc = rapidjson::Document(rapidjson::kArrayType);

  _INTR_ARRAY(Components::NodeRef) storedNodes;

  Components::NodeRef nodeStack[64];
  uint32_t nodeStackCount = 1u;
  nodeStack[0] = p_RootNodeRef;

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
    if (p_RootNodeRef != currentNodeRef && nextSibling.isValid())
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
          saveDesc.GetAllocator());

      node.AddMember("name", name, saveDesc.GetAllocator());

      const int32_t offsetToParent =
          parent.isValid() ? calcOffsetToParent(storedNodes, parent,
                                                (uint32_t)storedNodes.size())
                           : 0;
      node.AddMember("offsetToParent", offsetToParent, saveDesc.GetAllocator());

      rapidjson::Value propertyEntries =
          rapidjson::Value(rapidjson::kArrayType);

      for (auto propCompIt =
               Application::_componentPropertyCompilerMapping.begin();
           propCompIt != Application::_componentPropertyCompilerMapping.end();
           ++propCompIt)
      {
        rapidjson::Value componentType = rapidjson::Value(
            propCompIt->first.getString().c_str(), saveDesc.GetAllocator());

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
                properties, saveDesc);

            rapidjson::Value propertyEntry =
                rapidjson::Value(rapidjson::kObjectType);
            propertyEntry.AddMember("type", componentType,
                                    saveDesc.GetAllocator());
            propertyEntry.AddMember("properties", properties,
                                    saveDesc.GetAllocator());
            propertyEntries.PushBack(propertyEntry, saveDesc.GetAllocator());
          }
        }
      }

      node.AddMember("propertyEntries", propertyEntries,
                     saveDesc.GetAllocator());
      saveDesc.PushBack(node, saveDesc.GetAllocator());
      storedNodes.push_back(currentNodeRef);
    }
  }

  FILE* fp = fopen(p_FilePath.c_str(), "wb");

  if (fp == nullptr)
  {
    _INTR_LOG_ERROR("Failed to save node hierarchy to file '%s'...",
                    p_FilePath.c_str());
    return;
  }

  char* writeBuffer = (char*)Memory::Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileWriteStream os(fp, writeBuffer, 65536u);
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    saveDesc.Accept(writer);
    fclose(fp);
  }
  Memory::Tlsf::MainAllocator::free(writeBuffer);
}

// <-

Components::NodeRef World::loadNodeHierarchy(const _INTR_STRING& p_FilePath)
{
  rapidjson::Document saveDesc;
  {
    FILE* fp = fopen(p_FilePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_ERROR("Failed to load node hierarchy from file '%s'...",
                      p_FilePath.c_str());
      return Components::NodeRef();
    }

    char* readBuffer = (char*)Memory::Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      saveDesc.ParseStream(is);
      fclose(fp);
    }
    Memory::Tlsf::MainAllocator::free(readBuffer);
  }

  _INTR_ARRAY(Components::NodeRef) loadedNodes;

  // Initializes nodes
  {
    for (uint32_t i = 0u; i < saveDesc.Size(); ++i)
    {
      rapidjson::Value& node = saveDesc[i];
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

          compEntryIt->second.initFunction(componentRef, false,
                                           propertyEntry["properties"]);

          if (strcmp(componentType.GetString(), "Node") == 0u)
          {
            loadedNodes.push_back(componentRef);
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
    for (uint32_t i = 0u; i < loadedNodes.size(); ++i)
    {
      const Components::NodeRef nodeRef = loadedNodes[i];
      rapidjson::Value& node = saveDesc[i];

      const int32_t offsetToParent = node["offsetToParent"].GetInt();

      if (offsetToParent != 0)
      {
        Components::NodeManager::attachChildIgnoreParent(
            loadedNodes[i + offsetToParent], nodeRef);
      }
    }
  }

  return loadedNodes[0];
}

// <-

void World::loadNodeResources(Components::NodeRef p_RootNodeRef)
{
  Components::NodeRefArray nodeRefs;
  Components::NodeManager::collectNodes(p_RootNodeRef, nodeRefs);

  // Load component resources in order
  {
    for (uint32_t i = 0u; i < nodeRefs.size(); ++i)
    {
      const Components::NodeRef nodeRef = nodeRefs[i];
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
}

// <-

void World::load(const _INTR_STRING& p_FilePath)
{
  _INTR_LOG_INFO("Loading world from file '%s'...", p_FilePath.c_str());

  // Destroy the current world
  destroy();

  _flags |= WorldFlags::kLoadingUnloading;

  // Load world and set root node
  _rootNode = loadNodeHierarchy(p_FilePath);
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  loadNodeResources(_rootNode);

  // Set default camera
  _activeCamera = Components::CameraManager::getComponentForEntity(
      Entity::EntityManager::getEntityByName(_N(MainCamera)));
  _currentTime = 0.1f;
  _filePath = p_FilePath;

  GameStates::Editing::_currentlySelectedEntity =
      Components::NodeManager::_entity(_rootNode);
  Resources::EventManager::queueEventIfNotExisting(
      _N(CurrentlySelectedEntityChanged));

  _flags &= ~WorldFlags::kLoadingUnloading;
}

// <-

void World::updateDayNightCycle(float p_DeltaT)
{
  static Math::Gradient<glm::vec4, 7u> sunColorGradient = {
      {glm::vec4(glm::vec3(1.0f), 0.025f),
       glm::vec4(1.0f, 0.643f, 0.376f, 0.5f), glm::vec4(1.0f, 0.6f, 0.5f, 1.0f),
       glm::vec4(1.0f, 0.6f, 0.5f, 1.0f), glm::vec4(1.0f, 0.643f, 0.376f, 0.5f),
       glm::vec4(glm::vec3(1.0f), 0.025f), glm::vec4(glm::vec3(1.0f), 0.025f)},
      {0.0f, /* Sunrise */ 0.05f, 0.1f, 0.4f, 0.45f /* Dawn */,
       0.5f /* Night */, 1.0f}};
  static const float dayNightCycleDurationInS = 20.0f * 60.0f;
  static const float dayNightFadeInPerc = 0.05f;
  static const float nightLightIntens = 0.05f;

  _currentTime += p_DeltaT / dayNightCycleDurationInS;
  _currentTime = glm::mod(_currentTime, 1.0f);

  // Quant the current time to avoid shadow flickering due to the constant sun
  // light movement
  static const float quantInterval = 0.01f;
  static const float quantIntervalFadePerc = 0.05f;

  float quantCurrentTime =
      glm::trunc(_currentTime * (1.0f / quantInterval)) * quantInterval;

  // Smoothly animate to the next quant step
  const float fadeStart =
      quantCurrentTime + quantInterval - quantIntervalFadePerc * quantInterval;
  if (_currentTime > fadeStart)
  {
    const float fadePerc =
        (_currentTime - fadeStart) / (quantIntervalFadePerc * quantInterval);
    quantCurrentTime =
        glm::mix(quantCurrentTime, quantCurrentTime + quantInterval, fadePerc);
  }

  float currentDayNightFactor = 0.0f;
  if (quantCurrentTime < 0.5f)
  {
    const float perc = quantCurrentTime / 0.5f;
    currentDayNightFactor =
        1.0f - glm::smoothstep<float>(1.0f - dayNightFadeInPerc, 1.0f, perc);
    currentDayNightFactor *=
        glm::smoothstep<float>(0.0f, dayNightFadeInPerc, perc);
  }

  float sunPerc = 0.0f;
  if (quantCurrentTime <= 0.5f)
    sunPerc = quantCurrentTime / 0.5f;
  else
    sunPerc = 1.0f - (quantCurrentTime - 0.5f) / 0.5f;

  const float sunSin =
      glm::clamp(std::sin(sunPerc * glm::pi<float>()), -0.9f, 0.9f);
  const float sunCos =
      glm::clamp(std::cos(sunPerc * glm::pi<float>()), -0.9f, 0.9f);
  const glm::vec3 lightVec =
      glm::normalize(glm::vec3(sunCos, 0.75f * sunSin, -sunSin));

  _currentSunLightOrientation =
      glm::rotation(glm::vec3(0.0, 0.0, 1.0), lightVec);
  _currentDayNightFactor = glm::mix(0.05f, 1.0f, currentDayNightFactor);
  _currentSunLightColorAndIntensity = Math::interpolateGradient<glm::vec4, 7u>(
      sunColorGradient, quantCurrentTime);
}
}
}
