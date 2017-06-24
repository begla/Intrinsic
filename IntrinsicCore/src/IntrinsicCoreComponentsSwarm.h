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
typedef Dod::Ref SwarmRef;
typedef _INTR_ARRAY(SwarmRef) SwarmRefArray;

/**
 * Stores all the relevant data for the Swarm Component in a data
 * oriented fashion.
 */
struct SwarmData : Dod::Components::ComponentDataBase
{
  /**
   * Struct containing all data for a single Boid.
   */
  struct Boid
  {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
  };

  SwarmData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_SWARM_COMPONENT_COUNT)
  {
    descBoidMeshName.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);

    boids.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);
    nodes.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);
    lights.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);
    meshes.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);

    currentAverageVelocity.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);
    currentCenterOfMass.resize(_INTR_MAX_SWARM_COMPONENT_COUNT);
  }

  // Description
  _INTR_ARRAY(Name) descBoidMeshName;

  // Resources
  _INTR_ARRAY(_INTR_ARRAY(Boid)) boids;
  _INTR_ARRAY(_INTR_ARRAY(NodeRef)) nodes;
  _INTR_ARRAY(_INTR_ARRAY(Dod::Ref)) lights;
  _INTR_ARRAY(_INTR_ARRAY(Dod::Ref)) meshes;

  _INTR_ARRAY(glm::vec3) currentAverageVelocity;
  _INTR_ARRAY(glm::vec3) currentCenterOfMass;
};

/**
 * The manager for all Swarm Components.
 */
struct SwarmManager
    : Dod::Components::ComponentManagerBase<SwarmData,
                                            _INTR_MAX_SWARM_COMPONENT_COUNT>
{
  /**
   * Initializes the Swarm Manager.
   */
  static void init();

  // <-

  /**
   * Requests a new reference for a Swarm Component.
   */
  _INTR_INLINE static SwarmRef createSwarm(Entity::EntityRef p_ParentEntity)
  {
    SwarmRef ref = Dod::Components::ComponentManagerBase<
        SwarmData,
        _INTR_MAX_SWARM_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  /**
   * Resets the given Swarm Component to the default values.
   */
  _INTR_INLINE static void resetToDefault(SwarmRef p_Ref)
  {
    _descBoidMeshName(p_Ref) = _N(sphere);
  }

  // <-

  /**
   * Destroys the given Swarm Component by putting the reference
   * back in to the pool.
   */
  _INTR_INLINE static void destroySwarm(SwarmRef p_Swarm)
  {
    Dod::Components::ComponentManagerBase<
        SwarmData, _INTR_MAX_SWARM_COMPONENT_COUNT>::_destroyComponent(p_Swarm);
  }

  // <-

  /**
   * Compiles all exposed properties to a JSON descriptor.
   */
  _INTR_INLINE static void compileDescriptor(SwarmRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember("boidMeshName",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Mesh), _N(meshSelector),
                                             _descBoidMeshName(p_Ref), false,
                                             false),
                           p_Document.GetAllocator());
  }

  // <-

  /**
   * Initializes all properties from a JSON descriptor.
   */
  _INTR_INLINE static void initFromDescriptor(SwarmRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("boidMeshName"))
    {
      _descBoidMeshName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["boidMeshName"]);
    }
  }

  // <-

  /**
   * Creates all resources for the given Swarm Component.
   */
  _INTR_INLINE static void createResources(SwarmRef p_Swarm)
  {
    SwarmRefArray swarms = {p_Swarm};
    createResources(swarms);
  }

  // <-

  /**
   * Destroys all resources for the given Swarm Component.
   */
  _INTR_INLINE static void destroyResources(SwarmRef p_Swarm)
  {
    SwarmRefArray swarms = {p_Swarm};
    destroyResources(swarms);
  }

  // <-

  /**
   * Creates all resources for the given set of Swarm Components.
   */
  static void createResources(const SwarmRefArray& p_Swarms);

  /**
   * Destroys all resources for the given set of Swarm Components.
   */
  static void destroyResources(const SwarmRefArray& p_Swarms);

  // <-

  /**
   * Simulates all given Swarm Components for a single time step.
   */
  static void simulateSwarms(const SwarmRefArray& p_Swarms, float p_DeltaT);

  // <-

  // Description

  /**
   * The name of the Mesh used for rendering the Boids of a swarm.
   */
  _INTR_INLINE static Name& _descBoidMeshName(MeshRef p_Ref)
  {
    return _data.descBoidMeshName[p_Ref._id];
  }

  // Resources

  /**
   * Array of Boids owned by each of the Swarm Components.
   */
  _INTR_INLINE static _INTR_ARRAY(SwarmData::Boid) & _boids(SwarmRef p_Ref)
  {
    return _data.boids[p_Ref._id];
  }

  /**
   * Array of Node Components where each one is used for positioning a single
   * Boid in the world.
   */
  _INTR_INLINE static _INTR_ARRAY(NodeRef) & _nodes(SwarmRef p_Ref)
  {
    return _data.nodes[p_Ref._id];
  }

  /**
   * Array of Light Components attached to the Boids of swarm.
   */
  _INTR_INLINE static _INTR_ARRAY(NodeRef) & _lights(SwarmRef p_Ref)
  {
    return _data.lights[p_Ref._id];
  }

  /**
   * Array of Mesh Components used for rendering the Boids of a swarm.
   */
  _INTR_INLINE static _INTR_ARRAY(NodeRef) & _meshes(SwarmRef p_Ref)
  {
    return _data.meshes[p_Ref._id];
  }

  /**
   * The center of mass of the swarm the Boids are attracted to.
   */
  _INTR_INLINE static glm::vec3& _currentCenterOfMass(SwarmRef p_Ref)
  {
    return _data.currentCenterOfMass[p_Ref._id];
  }

  /**
   * The current average velocity of a single swarm.
   */
  _INTR_INLINE static glm::vec3& _currentAverageVelocity(SwarmRef p_Ref)
  {
    return _data.currentAverageVelocity[p_Ref._id];
  }
};
}
}
}
