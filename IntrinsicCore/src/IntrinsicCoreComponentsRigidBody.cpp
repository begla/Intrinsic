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

  // Init. default material
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

  const physx::PxTransform transform = PhysxHelper::convert(
      worldPos, NodeManager::_worldOrientation(nodeCompRef));
  const glm::vec3 halfExtent = Math::calcAABBHalfExtent(scaledLocalAABB);

  physx::PxRigidDynamic* sphereActor =
      Physics::System::_pxPhysics->createRigidDynamic(transform);
  _INTR_ASSERT(sphereActor);

  sphereActor->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC,
                                   p_Kinematic);

  physx::PxSphereGeometry sphereGeometry;
  sphereGeometry.radius = halfExtent.x;

  physx::PxShape* shape = sphereActor->createShape(
      sphereGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  physx::PxTransform localTransform = physx::PxTransform(
      PhysxHelper::convert(Math::calcAABBCenter(scaledLocalAABB)),
      physx::PxQuat::createIdentity());
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

  const physx::PxTransform transform = PhysxHelper::convert(
      worldPos, NodeManager::_worldOrientation(nodeCompRef));
  const glm::vec3 halfExtent = Math::calcAABBHalfExtent(scaledLocalAABB);

  physx::PxRigidDynamic* boxActor =
      Physics::System::_pxPhysics->createRigidDynamic(transform);
  _INTR_ASSERT(boxActor);

  boxActor->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC,
                                p_Kinematic);

  physx::PxBoxGeometry boxGeometry;
  boxGeometry.halfExtents = PhysxHelper::convert(halfExtent);

  physx::PxShape* shape =
      boxActor->createShape(boxGeometry, *RigidBodyManager::_defaultMaterial);
  _INTR_ASSERT(shape);

  physx::PxTransform localTransform = physx::PxTransform(
      PhysxHelper::convert(Math::calcAABBCenter(scaledLocalAABB)),
      physx::PxQuat::createIdentity());
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

  const physx::PxTransform transform =
      PhysxHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                           NodeManager::_worldOrientation(nodeCompRef));

  physx::PxTriangleMeshGeometry triangleMeshGeometry;
  triangleMeshGeometry.scale = physx::PxMeshScale(
      PhysxHelper::convert(NodeManager::_worldSize(nodeCompRef)),
      physx::PxQuat::createIdentity());
  triangleMeshGeometry.triangleMesh =
      Resources::MeshManager::_pxTriangleMesh(meshRef);

  physx::PxRigidActor* actor = nullptr;

  if (p_Kinematic)
  {
    physx::PxRigidDynamic* kinematicTriMeshActor =
        Physics::System::_pxPhysics->createRigidDynamic(transform);
    _INTR_ASSERT(kinematicTriMeshActor);
    actor = kinematicTriMeshActor;

    kinematicTriMeshActor->setRigidDynamicFlag(
        physx::PxRigidDynamicFlag::eKINEMATIC, true);

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

// <-

void RigidBodyManager::createResources(const RigidBodyRefArray& p_RigidBodies)
{
  Renderer::Vulkan::Resources::DrawCallRefArray drawCallsToCreate;

  for (uint32_t rigidBodyIdx = 0u; rigidBodyIdx < p_RigidBodies.size();
       ++rigidBodyIdx)
  {
    RigidBodyRef rigidBodyRef = p_RigidBodies[rigidBodyIdx];

    if (_descRigidBodyType(rigidBodyRef) == RigidBodyType::kBoxDynamic ||
        _descRigidBodyType(rigidBodyRef) == RigidBodyType::kBoxKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createBoxDynamicKinematic(
          rigidBodyRef,
          _descRigidBodyType(rigidBodyRef) == RigidBodyType::kBoxKinematic);
    }
    else if (_descRigidBodyType(rigidBodyRef) ==
                 RigidBodyType::kSphereDynamic ||
             _descRigidBodyType(rigidBodyRef) ==
                 RigidBodyType::kSphereKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createSphereDynamicKinematic(
          rigidBodyRef,
          _descRigidBodyType(rigidBodyRef) == RigidBodyType::kSphereKinematic);
    }
    else if (_descRigidBodyType(rigidBodyRef) ==
                 RigidBodyType::kTriangleMeshStatic ||
             _descRigidBodyType(rigidBodyRef) ==
                 RigidBodyType::kTriangleMeshKinematic)
    {
      _pxRigidActor(rigidBodyRef) = createTriangleMeshStaticKinematic(
          rigidBodyRef, _descRigidBodyType(rigidBodyRef) ==
                            RigidBodyType::kTriangleMeshKinematic);
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

    if (actor->isRigidDynamic() &&
        !actor->isRigidDynamic()->getRigidBodyFlags().isSet(
            physx::PxRigidBodyFlag::eKINEMATIC))
    {
      physx::PxTransform globalPose = actor->getGlobalPose();

      glm::vec3 worldPosition;
      glm::quat worldOrientation;
      PhysxHelper::convert(globalPose, worldPosition, worldOrientation);

      NodeRef parentNodeRef = NodeManager::_parent(nodeCompRef);
      if (parentNodeRef.isValid())
      {
        glm::quat inverseParentOrient =
            glm::inverse(NodeManager::_worldOrientation(parentNodeRef));

        NodeManager::_position(nodeCompRef) =
            inverseParentOrient *
            (worldPosition - NodeManager::_worldPosition(parentNodeRef));
        NodeManager::_orientation(nodeCompRef) =
            worldOrientation * inverseParentOrient;
      }

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

    physx::PxRigidDynamic* rigidDynamic = actor->isRigidDynamic();
    physx::PxRigidStatic* rigidStatic = actor->isRigidStatic();

    if (rigidDynamic &&
        rigidDynamic->getRigidBodyFlags().isSet(
            physx::PxRigidBodyFlag::eKINEMATIC))
    {
      // Update kinematic target
      const physx::PxTransform transform =
          PhysxHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                               NodeManager::_worldOrientation(nodeCompRef));
      rigidDynamic->setKinematicTarget(transform);
    }
    else if (rigidStatic)
    {
      const physx::PxTransform transform =
          PhysxHelper::convert(NodeManager::_worldPosition(nodeCompRef),
                               NodeManager::_worldOrientation(nodeCompRef));
      rigidStatic->setGlobalPose(transform);
    }
  }
}
}
}
}
