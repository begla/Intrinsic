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

// PhysX includes
#include "PxPhysics.h"
#include "PxScene.h"
#include "PxActor.h"
#include "PxRigidBody.h"
#include "PxRigidActor.h"
#include "PxRigidDynamic.h"
#include "PxMaterial.h"
#include "PxRigidStatic.h"
#include "extensions/PxRigidBodyExt.h"

namespace Intrinsic
{
namespace Core
{
namespace Components
{
physx::PxMaterial* RigidBodyManager::_defaultMaterial;

// <-

RigidBodyData::RigidBodyData()
    : Dod::Components::ComponentDataBase(_INTR_MAX_RIGID_BODY_COMPONENT_COUNT)
{
  // Description
  descRigidBodyType.resize(_INTR_MAX_RIGID_BODY_COMPONENT_COUNT);
  descDensity.resize(_INTR_MAX_RIGID_BODY_COMPONENT_COUNT);

  // Resources
  pxRigidActor.resize(_INTR_MAX_RIGID_BODY_COMPONENT_COUNT);
}

// <-

void RigidBodyManager::resetToDefault(RigidBodyRef p_RigidBody)
{
  _descRigidBodyType(p_RigidBody) = RigidBodyType::kBoxKinematic;
  _descDensity(p_RigidBody) = 1.0f;
}

void RigidBodyManager::init()
{
  _INTR_LOG_INFO("Inititializing Rigid Body Component Manager...");

  Dod::Components::ComponentManagerBase<
      RigidBodyData,
      _INTR_MAX_RIGID_BODY_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry rigidBodyEntry;
  {
    rigidBodyEntry.createFunction =
        Components::RigidBodyManager::createRigidBody;
    rigidBodyEntry.destroyFunction =
        Components::RigidBodyManager::destroyRigidBody;
    rigidBodyEntry.createResourcesFunction =
        Components::RigidBodyManager::createResources;
    rigidBodyEntry.destroyResourcesFunction =
        Components::RigidBodyManager::destroyResources;
    rigidBodyEntry.getComponentForEntityFunction =
        Components::RigidBodyManager::getComponentForEntity;
    rigidBodyEntry.resetToDefaultFunction =
        Components::RigidBodyManager::resetToDefault;

    Application::_componentManagerMapping[_N(RigidBody)] = rigidBodyEntry;
    Application::_orderedComponentManagers.push_back(rigidBodyEntry);
  }

  Dod::PropertyCompilerEntry propCompilerRigidBody;
  {
    propCompilerRigidBody.compileFunction =
        Components::RigidBodyManager::compileDescriptor;
    propCompilerRigidBody.initFunction =
        Components::RigidBodyManager::initFromDescriptor;
    propCompilerRigidBody.ref = Dod::Ref();

    Application::_componentPropertyCompilerMapping[_N(RigidBody)] =
        propCompilerRigidBody;
  }

  // Initializes default material
  _defaultMaterial =
      Physics::System::_pxPhysics->createMaterial(0.5f, 0.5f, 0.1f);
}

// <-

physx::PxRigidActor* createSphereDynamicKinematic(RigidBodyRef p_Ref,
                                                  bool p_Kinematic)
{
  NodeRef nodeCompRef =
      NodeManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(nodeCompRef.isValid());

  const glm::vec3& worldPos = NodeManager::_worldPosition(nodeCompRef);
  const glm::vec3& worldSize = NodeManager::_worldSize(nodeCompRef);

  Math::AABB scaledLocalAABB = NodeManager::_localAABB(nodeCompRef);
  Math::scaleAABB(scaledLocalAABB, worldSize);

  const physx::PxTransform transform = PhysicsHelper::convert(
      worldPos, NodeManager::_worldOrientation(nodeCompRef));
  const glm::vec3 halfExtent = Math::calcAABBHalfExtent(scaledLocalAABB);

  physx::PxRigidDynamic* sphereActor =
      Physics::System::_pxPhysics->createRigidDynamic(transform);
  _INTR_ASSERT(sphereActor);

  sphereActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC,
                                p_Kinematic);

  physx::PxSphereGeometry sphereGeometry;
  sphereGeometry.radius = glm::compMin(halfExtent);

  physx::PxShape* shape = sphereActor->createShape(
      sphereGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  physx::PxTransform localTransform = physx::PxTransform(
      PhysicsHelper::convert(Math::calcAABBCenter(scaledLocalAABB)),
      physx::PxQuat(physx::PxIdentity));
  shape->setLocalPose(localTransform);

  physx::PxRigidBodyExt::updateMassAndInertia(
      *sphereActor, RigidBodyManager::_descDensity(p_Ref));
  Physics::System::_pxScene->addActor(*sphereActor);

  return sphereActor;
}

// <-

physx::PxRigidActor* createBoxDynamicKinematic(RigidBodyRef p_Ref,
                                               bool p_Kinematic)
{
  NodeRef nodeCompRef =
      NodeManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(nodeCompRef.isValid());

  const glm::vec3& worldPos = NodeManager::_worldPosition(nodeCompRef);
  const glm::vec3& worldSize = NodeManager::_worldSize(nodeCompRef);

  Math::AABB scaledLocalAABB = NodeManager::_localAABB(nodeCompRef);
  Math::scaleAABB(scaledLocalAABB, worldSize);

  const physx::PxTransform transform = PhysicsHelper::convert(
      worldPos, NodeManager::_worldOrientation(nodeCompRef));
  const glm::vec3 halfExtent = Math::calcAABBHalfExtent(scaledLocalAABB);

  physx::PxRigidDynamic* boxActor =
      Physics::System::_pxPhysics->createRigidDynamic(transform);
  _INTR_ASSERT(boxActor);

  boxActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, p_Kinematic);

  physx::PxBoxGeometry boxGeometry;
  boxGeometry.halfExtents = PhysicsHelper::convert(halfExtent);

  physx::PxShape* shape =
      boxActor->createShape(boxGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  physx::PxTransform localTransform = physx::PxTransform(
      PhysicsHelper::convert(Math::calcAABBCenter(scaledLocalAABB)),
      physx::PxQuat(physx::PxIdentity));
  shape->setLocalPose(localTransform);

  physx::PxRigidBodyExt::updateMassAndInertia(
      *boxActor, RigidBodyManager::_descDensity(p_Ref));
  Physics::System::_pxScene->addActor(*boxActor);

  return boxActor;
}

// <-

physx::PxRigidActor* createTriangleMeshStaticKinematic(RigidBodyRef p_Ref,
                                                       bool p_Kinematic)
{
  NodeRef nodeCompRef =
      NodeManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(nodeCompRef.isValid());
  MeshRef meshCompRef =
      MeshManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(meshCompRef.isValid());
  Resources::MeshRef meshRef = Resources::MeshManager::getResourceByName(
      MeshManager::_descMeshName(meshCompRef));
  _INTR_ASSERT(meshRef.isValid());

  if (Resources::MeshManager::_pxTriangleMesh(meshRef) == nullptr)
  {
    _INTR_LOG_WARNING(
        "No physics triangle mesh available for mesh \"%s\"!",
        Resources::MeshManager::_name(meshRef).getString().c_str());
    return nullptr;
  }

  const physx::PxTransform transform =
      PhysicsHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                             NodeManager::_worldOrientation(nodeCompRef));

  physx::PxTriangleMeshGeometry triangleMeshGeometry;
  triangleMeshGeometry.scale = physx::PxMeshScale(
      PhysicsHelper::convert(NodeManager::_worldSize(nodeCompRef)),
      physx::PxQuat(physx::PxIdentity));
  triangleMeshGeometry.triangleMesh =
      Resources::MeshManager::_pxTriangleMesh(meshRef);

  physx::PxRigidActor* actor = nullptr;

  if (p_Kinematic)
  {
    physx::PxRigidDynamic* kinematicTriMeshActor =
        Physics::System::_pxPhysics->createRigidDynamic(transform);
    _INTR_ASSERT(kinematicTriMeshActor);
    actor = kinematicTriMeshActor;

    kinematicTriMeshActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC,
                                            true);

    physx::PxShape* shape = kinematicTriMeshActor->createShape(
        triangleMeshGeometry, *RigidBodyManager::_defaultMaterial);
    _INTR_ASSERT(shape);

    physx::PxRigidBodyExt::updateMassAndInertia(
        *kinematicTriMeshActor, RigidBodyManager::_descDensity(p_Ref));
  }
  else
  {
    physx::PxRigidStatic* staticTriMeshActor =
        Physics::System::_pxPhysics->createRigidStatic(transform);
    _INTR_ASSERT(staticTriMeshActor);
    actor = staticTriMeshActor;

    physx::PxShape* shape = staticTriMeshActor->createShape(
        triangleMeshGeometry, *RigidBodyManager::_defaultMaterial);
    _INTR_ASSERT(shape);
  }

  Physics::System::_pxScene->addActor(*actor);

  return actor;
}

physx::PxRigidActor* createConvexMeshDynamicKinematic(RigidBodyRef p_Ref,
                                                      bool p_Kinematic)
{
  NodeRef nodeCompRef =
      NodeManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(nodeCompRef.isValid());
  MeshRef meshCompRef =
      MeshManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(meshCompRef.isValid());
  Resources::MeshRef meshRef = Resources::MeshManager::getResourceByName(
      MeshManager::_descMeshName(meshCompRef));
  _INTR_ASSERT(meshRef.isValid());

  if (Resources::MeshManager::_pxConvexMesh(meshRef) == nullptr)
  {
    _INTR_LOG_WARNING(
        "No physics convex mesh available for mesh \"%s\"!",
        Resources::MeshManager::_name(meshRef).getString().c_str());
    return nullptr;
  }

  const physx::PxTransform transform =
      PhysicsHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                             NodeManager::_worldOrientation(nodeCompRef));

  physx::PxConvexMeshGeometry convexMeshGeometry;
  convexMeshGeometry.scale = physx::PxMeshScale(
      PhysicsHelper::convert(NodeManager::_worldSize(nodeCompRef)),
      physx::PxQuat(physx::PxIdentity));
  convexMeshGeometry.convexMesh =
      Resources::MeshManager::_pxConvexMesh(meshRef);

  physx::PxRigidActor* actor = nullptr;

  physx::PxRigidDynamic* convexMeshActor =
      Physics::System::_pxPhysics->createRigidDynamic(transform);
  _INTR_ASSERT(convexMeshActor);
  actor = convexMeshActor;

  if (p_Kinematic)
    convexMeshActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);

  physx::PxShape* shape = convexMeshActor->createShape(
      convexMeshGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  physx::PxRigidBodyExt::updateMassAndInertia(
      *convexMeshActor, RigidBodyManager::_descDensity(p_Ref));

  Physics::System::_pxScene->addActor(*actor);

  return actor;
}

physx::PxRigidActor* createConvexMeshStatic(RigidBodyRef p_Ref)
{
  NodeRef nodeCompRef =
      NodeManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(nodeCompRef.isValid());
  MeshRef meshCompRef =
      MeshManager::getComponentForEntity(RigidBodyManager::_entity(p_Ref));
  _INTR_ASSERT(meshCompRef.isValid());
  Resources::MeshRef meshRef = Resources::MeshManager::getResourceByName(
      MeshManager::_descMeshName(meshCompRef));
  _INTR_ASSERT(meshRef.isValid());

  if (Resources::MeshManager::_pxConvexMesh(meshRef) == nullptr)
  {
    _INTR_LOG_WARNING(
        "No physics convex mesh available for mesh \"%s\"!",
        Resources::MeshManager::_name(meshRef).getString().c_str());
    return nullptr;
  }

  const physx::PxTransform transform =
      PhysicsHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                             NodeManager::_worldOrientation(nodeCompRef));

  physx::PxConvexMeshGeometry convexMeshGeometry;
  convexMeshGeometry.scale = physx::PxMeshScale(
      PhysicsHelper::convert(NodeManager::_worldSize(nodeCompRef)),
      physx::PxQuat(physx::PxIdentity));
  convexMeshGeometry.convexMesh =
      Resources::MeshManager::_pxConvexMesh(meshRef);

  physx::PxRigidActor* actor = nullptr;

  physx::PxRigidStatic* convexMeshActor =
      Physics::System::_pxPhysics->createRigidStatic(transform);
  _INTR_ASSERT(convexMeshActor);
  actor = convexMeshActor;

  physx::PxShape* shape = convexMeshActor->createShape(
      convexMeshGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  Physics::System::_pxScene->addActor(*actor);

  return actor;
}

// <-

void RigidBodyManager::createResources(const RigidBodyRefArray& p_RigidBodies)
{
  RResources::DrawCallRefArray drawCallsToCreate;

  for (uint32_t rigidBodyIdx = 0u; rigidBodyIdx < p_RigidBodies.size();
       ++rigidBodyIdx)
  {
    RigidBodyRef rigidBodyRef = p_RigidBodies[rigidBodyIdx];
    const RigidBodyType::Enum rigidBodyType = _descRigidBodyType(rigidBodyRef);
    _INTR_ASSERT(_pxRigidActor(rigidBodyRef) == nullptr &&
                 "Rigid Body resources already created");

    if (rigidBodyType == RigidBodyType::kBoxDynamic ||
        rigidBodyType == RigidBodyType::kBoxKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createBoxDynamicKinematic(
          rigidBodyRef, rigidBodyType == RigidBodyType::kBoxKinematic);
    }
    else if (rigidBodyType == RigidBodyType::kSphereDynamic ||
             rigidBodyType == RigidBodyType::kSphereKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createSphereDynamicKinematic(
          rigidBodyRef, rigidBodyType == RigidBodyType::kSphereKinematic);
    }
    else if (rigidBodyType == RigidBodyType::kTriangleMeshStatic ||
             rigidBodyType == RigidBodyType::kTriangleMeshKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createTriangleMeshStaticKinematic(
          rigidBodyRef, rigidBodyType == RigidBodyType::kTriangleMeshKinematic);
    }
    else if (rigidBodyType == RigidBodyType::kConvexMeshDynamic ||
             rigidBodyType == RigidBodyType::kConvexMeshKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createConvexMeshDynamicKinematic(
          rigidBodyRef, rigidBodyType == RigidBodyType::kConvexMeshKinematic);
    }
    else if (rigidBodyType == RigidBodyType::kConvexMeshStatic)
    {
      _pxRigidActor(rigidBodyRef) = createConvexMeshStatic(rigidBodyRef);
    }
  }
}

// <-

void RigidBodyManager::destroyResources(const RigidBodyRefArray& p_RigidBodies)
{
  for (uint32_t rigidBodyIdx = 0u; rigidBodyIdx < p_RigidBodies.size();
       ++rigidBodyIdx)
  {
    RigidBodyRef rigidBodyRef = p_RigidBodies[rigidBodyIdx];

    if (_pxRigidActor(rigidBodyRef))
    {
      _pxRigidActor(rigidBodyRef)->release();
      _pxRigidActor(rigidBodyRef) = nullptr;
    }
  }
}

// <-

void RigidBodyManager::updateNodesFromActors(
    const RigidBodyRefArray& p_RigidBodies)
{
  _INTR_PROFILE_CPU("Physics", "Update Nodes From Actors");

  for (uint32_t rigidBodyIdx = 0u; rigidBodyIdx < p_RigidBodies.size();
       ++rigidBodyIdx)
  {
    RigidBodyRef rigidBodyRef = p_RigidBodies[rigidBodyIdx];
    NodeRef nodeCompRef = NodeManager::getComponentForEntity(
        RigidBodyManager::_entity(rigidBodyRef));
    physx::PxRigidActor* actor = RigidBodyManager::_pxRigidActor(rigidBodyRef);

    if (actor && actor->is<physx::PxRigidDynamic>() &&
        !actor->is<physx::PxRigidDynamic>()->getRigidBodyFlags().isSet(
            physx::PxRigidBodyFlag::eKINEMATIC))
    {
      physx::PxTransform globalPose = actor->getGlobalPose();

      glm::vec3 worldPosition;
      glm::quat worldOrientation;
      PhysicsHelper::convert(globalPose, worldPosition, worldOrientation);

      NodeManager::updateFromWorldPosition(nodeCompRef, worldPosition);
      NodeManager::updateFromWorldOrientation(nodeCompRef, worldOrientation);

      NodeManager::updateTransforms(nodeCompRef);
    }
  }
}

// <-

void RigidBodyManager::updateActorsFromNodes(
    const RigidBodyRefArray& p_RigidBodies)
{
  _INTR_PROFILE_CPU("Physics", "Update Actors From Nodes");

  for (uint32_t rigidBodyIdx = 0u; rigidBodyIdx < p_RigidBodies.size();
       ++rigidBodyIdx)
  {
    RigidBodyRef rigidBodyRef = p_RigidBodies[rigidBodyIdx];
    NodeRef nodeCompRef = NodeManager::getComponentForEntity(
        RigidBodyManager::_entity(rigidBodyRef));
    physx::PxRigidActor* actor = RigidBodyManager::_pxRigidActor(rigidBodyRef);

    if (actor)
    {
      physx::PxRigidDynamic* rigidDynamic = actor->is<physx::PxRigidDynamic>();
      physx::PxRigidStatic* rigidStatic = actor->is<physx::PxRigidStatic>();

      if (rigidDynamic &&
          rigidDynamic->getRigidBodyFlags().isSet(
              physx::PxRigidBodyFlag::eKINEMATIC))
      {
        // Update kinematic target
        const physx::PxTransform transform =
            PhysicsHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                                   NodeManager::_worldOrientation(nodeCompRef));
        rigidDynamic->setKinematicTarget(transform);
      }
      else if (rigidStatic)
      {
        const physx::PxTransform transform =
            PhysicsHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                                   NodeManager::_worldOrientation(nodeCompRef));
        rigidStatic->setGlobalPose(transform);
      }
    }
  }
}
}
}
}
