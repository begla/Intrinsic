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

// TODO: Expose properties for swarms
#define BOID_COUNT 64u

namespace Intrinsic
{
namespace Core
{
namespace Components
{
void SwarmManager::init()
{
  _INTR_LOG_INFO("Inititializing Swarm Component Manager...");

  Dod::Components::ComponentManagerBase<
      SwarmData, _INTR_MAX_SWARM_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry SwarmEntry;
  {
    SwarmEntry.createFunction = Components::SwarmManager::createSwarm;
    SwarmEntry.destroyFunction = Components::SwarmManager::destroySwarm;
    SwarmEntry.createResourcesFunction =
        Components::SwarmManager::createResources;
    SwarmEntry.destroyResourcesFunction =
        Components::SwarmManager::destroyResources;
    SwarmEntry.getComponentForEntityFunction =
        Components::SwarmManager::getComponentForEntity;
    SwarmEntry.resetToDefaultFunction =
        Components::SwarmManager::resetToDefault;

    Application::_componentManagerMapping[_N(Swarm)] = SwarmEntry;
    Application::_orderedComponentManagers.push_back(SwarmEntry);
  }

  Dod::PropertyCompilerEntry propCompilerSwarm;
  {
    propCompilerSwarm.compileFunction =
        Components::SwarmManager::compileDescriptor;
    propCompilerSwarm.initFunction =
        Components::SwarmManager::initFromDescriptor;
    propCompilerSwarm.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(Swarm)] =
        propCompilerSwarm;
  }
}

// <-

void SwarmManager::simulateSwarms(const SwarmRefArray& p_Swarms, float p_DeltaT)
{
  for (uint32_t swarmIdx = 0u; swarmIdx < p_Swarms.size(); ++swarmIdx)
  {
    SwarmRef swarmRef = p_Swarms[swarmIdx];

    _INTR_ARRAY(SwarmData::Boid)& boids =
        Components::SwarmManager::_boids(swarmRef);
    const NodeRefArray& nodes = Components::SwarmManager::_nodes(swarmRef);
    const LightRefArray& lights = Components::SwarmManager::_lights(swarmRef);
    const MeshRefArray& meshes = Components::SwarmManager::_meshes(swarmRef);
    _INTR_ASSERT(boids.size() == nodes.size() &&
                 "Node vs boid count does not match");
    glm::vec3& currentCenterOfMass =
        Components::SwarmManager::_currentCenterOfMass(swarmRef);
    glm::vec3& currentAverageVelocity =
        Components::SwarmManager::_currentAverageVelocity(swarmRef);

    Components::NodeRef swarmNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::SwarmManager::_entity(swarmRef));

    float groundPlaneHeight = -10000000.0f;

    physx::PxRaycastHit hit;
    const Math::Ray ray = {currentCenterOfMass, glm::vec3(0.0f, -1.0f, 0.0f)};
    if (PhysicsHelper::raycast(ray, hit, 1000.0f))
    {
      groundPlaneHeight = (ray.o + hit.distance * ray.d).y + 1.0f;
    }

    // Simulate boids and apply position to node
    glm::vec3 newCenterOfMass = glm::vec3(0.0f);
    glm::vec3 newAvgVelocity = glm::vec3(0.0f);
    {
      for (uint32_t boidIdx = 0u; boidIdx < boids.size(); ++boidIdx)
      {
        NodeRef nodeRef = nodes[boidIdx];
        SwarmData::Boid& boid = boids[boidIdx];

        // TODO: Add properties for those
        static const float boidAcc = 10.0f;
        static const float boidMinDistSqr = 8.0f * 8.0f;
        static const float distanceRuleWeight = 0.3f;
        static const float targetRuleWeight = 0.9f;
        static const float minTargetDist = 4.0f;
        static const float matchVelWeight = 0.025f;
        static const float centerOfMassWeight = 0.8f;
        static const float groundPlaneWeight = 5.0f;
        static const float maxVel = 30.0f;
        static uint32_t boidsToCheck = 10u;

        // Fly towards center of mass
        const glm::vec3 boidToCenter = currentCenterOfMass - boid.pos;
        const float boidToCenterDist = glm::length(boidToCenter);
        {
          if (boidToCenterDist > _INTR_EPSILON)
            boid.vel += boidToCenter / boidToCenterDist * p_DeltaT * boidAcc *
                        centerOfMassWeight;
        }

        // Keep a distance to other boids
        for (uint32_t boidIdx = 0u;
             boidIdx < std::min(32u, (uint32_t)boids.size()); ++boidIdx)
        {
          // Very rough approx. to work around O(n^2)
          const uint32_t boidToCompareIdx =
              Math::calcRandomNumber() % boids.size();

          if (boidToCompareIdx == boidIdx)
            continue;

          const SwarmData::Boid& boidToCompare = boids[boidToCompareIdx];
          const float distSqr = glm::distance2(boidToCompare.pos, boid.pos);

          if (distSqr < boidMinDistSqr && distSqr > _INTR_EPSILON)
          {
            boid.vel += boidAcc * glm::normalize(boidToCompare.pos - boid.pos) *
                        -1.0f * p_DeltaT * distanceRuleWeight;
          }
        }

        // Match velocities
        {
          boid.vel +=
              (currentAverageVelocity - boid.vel) * matchVelWeight * p_DeltaT;
        }

        // Target the node of the component
        {
          const glm::vec3 boidToComponent =
              Math::calcAABBCenter(
                  Components::NodeManager::_worldAABB(swarmNodeRef)) -
              boid.pos;
          const float boidToComponentDist = glm::length(boidToComponent);

          if (boidToComponentDist > _INTR_EPSILON &&
              boidToComponentDist > minTargetDist)
            boid.vel += boidToComponent / boidToComponentDist * boidAcc *
                        p_DeltaT * targetRuleWeight;
        }

        // Fly towards characters
        for (CharacterControllerRef cctRef :
             CharacterControllerManager::_activeRefs)
        {
          const NodeRef cctNodeRef = NodeManager::getComponentForEntity(
              CharacterControllerManager::_entity(cctRef));

          const glm::vec3 targetPos =
              Math::calcAABBCenter(NodeManager::_worldAABB(cctNodeRef));

          const float distSqr = glm::distance2(targetPos, boid.pos);

          if (distSqr < 15.0f * 15.0f && distSqr > _INTR_EPSILON)
          {
            boid.vel += boidAcc * glm::normalize(targetPos - boid.pos) *
                        p_DeltaT * 5.0f;
          }
        }

        // Avoid the ground plane
        {
          if (boid.pos.y < groundPlaneHeight)
          {
            boid.vel.y = 0.0f;
            boid.pos.y = groundPlaneHeight;
          }
        }

        newCenterOfMass += boid.pos;

        float velLen = glm::length(boid.vel);
        if (velLen > maxVel)
        {
          boid.vel = boid.vel / velLen * maxVel;
        }

        newAvgVelocity += boid.vel;

        // Integrate
        boid.pos += boid.vel * p_DeltaT;

        // Update node
        Components::NodeManager::_position(nodeRef) = boid.pos;
        Components::NodeManager::_orientation(nodeRef) = glm::rotation(
            glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(boid.vel + 0.01f));

        // Update lights and mesh color
        glm::vec4 boidColor = glm::vec4(boid.color, 1.0f);
        boidColor *=
            (std::sin(2.0f * TaskManager::_totalTimePassed * glm::pi<float>() +
                      boidIdx * glm::quarter_pi<float>()) *
                 0.5f +
             0.5f) *
                0.9f +
            0.1f;

        Components::MeshManager::_descColorTint(meshes[boidIdx]) = boidColor;
        Components::LightManager::_descColor(lights[boidIdx]) = boidColor;
      }

      Components::NodeManager::updateTransforms(nodes);
    }

    currentCenterOfMass = newCenterOfMass / (float)boids.size();
    currentAverageVelocity = newAvgVelocity / (float)boids.size();
  }
}

// <-

void SwarmManager::createResources(const SwarmRefArray& p_Swarms)
{
  Components::MeshRefArray meshComponentsToCreate;

  for (uint32_t swarmIdx = 0u; swarmIdx < p_Swarms.size(); ++swarmIdx)
  {
    SwarmRef swarmRef = p_Swarms[swarmIdx];

    for (uint32_t i = 0u; i < BOID_COUNT; ++i)
    {
      Entity::EntityRef entityRef =
          Entity::EntityManager::createEntity(_N(SwarmData::Boid));
      Components::NodeRef nodeRef =
          Components::NodeManager::createNode(entityRef);
      Components::NodeManager::attachChild(World::_rootNode, nodeRef);

      Components::NodeManager::_flags(nodeRef) |=
          Components::NodeFlags::kSpawned;
      Components::NodeManager::_size(nodeRef) = glm::vec3(0.45f, 0.45f, 0.45f);

      const glm::vec3 boidColor =
          glm::vec3(Math::calcRandomFloatMinMax(0.0f, 1.0f),
                    Math::calcRandomFloatMinMax(0.0f, 1.0f),
                    Math::calcRandomFloatMinMax(0.0f, 1.0f));

      Components::MeshRef meshRef =
          Components::MeshManager::createMesh(entityRef);
      Components::MeshManager::resetToDefault(meshRef);
      Components::MeshManager::_descMeshName(meshRef) =
          _descBoidMeshName(swarmRef);

      Components::LightRef lightRef =
          Components::LightManager::createLight(entityRef);
      Components::LightManager::resetToDefault(lightRef);
      Components::LightManager::_descRadius(lightRef) = 10.0f;
      Components::LightManager::_descIntensity(lightRef) = 50.0f;

      Components::NodeRef swarmNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::SwarmManager::_entity(swarmRef));

      Components::SwarmManager::_boids(swarmRef).push_back(
          {Components::NodeManager::_worldPosition(swarmNodeRef),
           glm::vec3(0.0f), boidColor});
      Components::SwarmManager::_nodes(swarmRef).push_back(nodeRef);
      Components::SwarmManager::_lights(swarmRef).push_back(lightRef);
      Components::SwarmManager::_meshes(swarmRef).push_back(meshRef);
      meshComponentsToCreate.push_back(meshRef);
    }
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(meshComponentsToCreate);
}

// <-

void SwarmManager::destroyResources(const SwarmRefArray& p_Swarms)
{
  for (uint32_t swarmIdx = 0u; swarmIdx < p_Swarms.size(); ++swarmIdx)
  {
    SwarmRef swarmRef = p_Swarms[swarmIdx];

    _INTR_ARRAY(SwarmData::Boid)& boids =
        Components::SwarmManager::_boids(swarmRef);
    NodeRefArray& nodes = Components::SwarmManager::_nodes(swarmRef);
    Dod::RefArray& lights = Components::SwarmManager::_lights(swarmRef);
    Dod::RefArray& meshes = Components::SwarmManager::_meshes(swarmRef);

    if ((World::_flags & WorldFlags::kLoadingUnloading) == 0u)
    {
      for (uint32_t i = 0u; i < nodes.size(); ++i)
      {
        World::destroyNodeFull(nodes[i]);
      }
    }

    boids.clear();
    nodes.clear();
    lights.clear();
    meshes.clear();
  }
}
}
}
}
