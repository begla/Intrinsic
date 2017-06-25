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
#include "characterkinematic/PxControllerManager.h"
#include "characterkinematic/PxController.h"
#include "characterkinematic/PxCapsuleController.h"

namespace Intrinsic
{
namespace Core
{
namespace Components
{
namespace
{
physx::PxControllerManager* _pxControllerManager = nullptr;
// TODO: Expose as properties
const float _controllerRadius = 1.5f;
const float _controllerHeight = 2.0f;
}

// <-

void CharacterControllerManager::init()
{
  _INTR_LOG_INFO("Inititializing Character Controller Component Manager...");

  Dod::Components::ComponentManagerBase<
      CharacterControllerData,
      _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry characterControllerEntry;
  {
    characterControllerEntry.createFunction =
        Components::CharacterControllerManager::createCharacterController;
    characterControllerEntry.destroyFunction =
        Components::CharacterControllerManager::destroyCharacterController;
    characterControllerEntry.createResourcesFunction =
        Components::CharacterControllerManager::createResources;
    characterControllerEntry.destroyResourcesFunction =
        Components::CharacterControllerManager::destroyResources;
    characterControllerEntry.getComponentForEntityFunction =
        Components::CharacterControllerManager::getComponentForEntity;
    characterControllerEntry.resetToDefaultFunction =
        Components::CharacterControllerManager::resetToDefault;

    Application::_componentManagerMapping[_N(CharacterController)] =
        characterControllerEntry;
    Application::_orderedComponentManagers.push_back(characterControllerEntry);
  }

  Dod::PropertyCompilerEntry propCompilerCharacterController;
  {
    propCompilerCharacterController.compileFunction =
        Components::CharacterControllerManager::compileDescriptor;
    propCompilerCharacterController.initFunction =
        Components::CharacterControllerManager::initFromDescriptor;
    propCompilerCharacterController.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(CharacterController)] =
        propCompilerCharacterController;
  }

  _INTR_ASSERT(_pxControllerManager == nullptr);
  _pxControllerManager =
      PxCreateControllerManager(*Physics::System::_pxScene, false);
  _INTR_ASSERT(_pxControllerManager);
}

// <-

void CharacterControllerManager::updateControllers(
    const CharacterControllerRefArray& p_CharacterControllers, float p_DeltaT)
{
  for (uint32_t i = 0u; i < p_CharacterControllers.size(); ++i)
  {
    CharacterControllerRef charCtrlRef = p_CharacterControllers[i];
    physx::PxController*& pxController = _pxController(charCtrlRef);
    _INTR_ASSERT(pxController);
    glm::vec3& internalMoveVector = _internalMoveVector(charCtrlRef);

    NodeRef nodeRef = NodeManager::getComponentForEntity(_entity(charCtrlRef));
    const glm::vec3 currentControllerPosition =
        PhysicsHelper::convertExt(pxController->getFootPosition());
    const glm::vec3& currentWorldPos = NodeManager::_worldPosition(nodeRef);

    if (currentControllerPosition != currentWorldPos)
    {
      pxController->setPosition(PhysicsHelper::convertExt(currentWorldPos));
    }

    // Apply gravity
    static const float gravity = -30.0f;
    internalMoveVector.y += gravity * p_DeltaT;

    if (internalMoveVector.y < gravity)
    {
      internalMoveVector.y = gravity;
    }

    // Apply and reset movement
    const float xzVelocity =
        glm::length(glm::vec2(internalMoveVector.x, internalMoveVector.z));
    const float xzMoveVelocity = glm::length(glm::vec2(
        _currentMoveVector(charCtrlRef).x, _currentMoveVector(charCtrlRef).z));

    if (xzVelocity < xzMoveVelocity)
    {
      internalMoveVector.x = p_DeltaT * _currentMoveVector(charCtrlRef).x;
      internalMoveVector.z = p_DeltaT * _currentMoveVector(charCtrlRef).z;
    }
    else
    {
      glm::vec2 dampedXzMovement =
          glm::vec2(internalMoveVector.x, internalMoveVector.z);
      Math::dampSimple(dampedXzMovement, 0.005f, p_DeltaT);
      internalMoveVector.x = dampedXzMovement.x;
      internalMoveVector.z = dampedXzMovement.y;
    }
    internalMoveVector.y += _currentMoveVector(charCtrlRef).y;

    _currentMoveVector(charCtrlRef) = glm::vec3(0.0f);

    const physx::PxVec3 finalMoveVector =
        PhysicsHelper::convert(p_DeltaT * internalMoveVector);
    physx::PxControllerFilters filters;

    const physx::PxControllerCollisionFlags currentFlags =
        pxController->move(finalMoveVector, 0.001f, p_DeltaT, filters);

    // Update collision flags
    uint32_t& collisionFlags = _lastCollisionFlags(charCtrlRef);
    {
      collisionFlags = 0x0;

      if (currentFlags.isSet(
              physx::PxControllerCollisionFlag::eCOLLISION_SIDES))
      {
        collisionFlags |= CharacterControllerCollisionFlags::kSides;
      }
      if (currentFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_UP))
      {
        collisionFlags |= CharacterControllerCollisionFlags::kUp;
      }
      if (currentFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
      {
        collisionFlags |= CharacterControllerCollisionFlags::kDown;
      }
    }

    // Reset vertical acceleration/gravity
    if ((collisionFlags & CharacterControllerCollisionFlags::kDown) > 0u)
    {
      // Keep some downward acceleration to keep the character grounded
      internalMoveVector.y = -8.0f;
    }

    const glm::vec3 nodeWorldPosAfterSim =
        PhysicsHelper::convertExt(pxController->getFootPosition());

    // Update node position (and remove parent transform beforehand)
    NodeRef parentNodeRef = NodeManager::_parent(nodeRef);
    if (parentNodeRef.isValid())
    {
      glm::quat inverseParentOrient =
          glm::inverse(NodeManager::_worldOrientation(parentNodeRef));
      NodeManager::_position(nodeRef) =
          inverseParentOrient *
          (nodeWorldPosAfterSim - NodeManager::_worldPosition(parentNodeRef));
    }

    // Rotate the CCT into the direction of the move vector
    glm::vec3 rotationDirection = internalMoveVector;
    rotationDirection.y = 0.0f;
    const float movVecLen = glm::length(rotationDirection);

    if (movVecLen > _INTR_EPSILON)
    {
      const glm::vec3 playerOrientation = rotationDirection / movVecLen;
      Components::NodeManager::_orientation(nodeRef) =
          glm::slerp(Components::NodeManager::_orientation(nodeRef),
                     glm::rotation(glm::vec3(0.0, 0.0, 1.0), playerOrientation),
                     glm::clamp(p_DeltaT / 0.1f, 0.0f, 1.0f));
    }

    NodeManager::updateTransforms(nodeRef);
  }
}

// <-

void CharacterControllerManager::createResources(
    const CharacterControllerRefArray& p_CharacterControllers)
{
  for (uint32_t cctIdx = 0u; cctIdx < p_CharacterControllers.size(); ++cctIdx)
  {
    CharacterControllerRef charCtrlRef = p_CharacterControllers[cctIdx];

    physx::PxController*& pxController = _pxController(charCtrlRef);
    _INTR_ASSERT(pxController == nullptr);

    // Create capsule controller
    {
      physx::PxCapsuleControllerDesc controllerDesc;
      {
        controllerDesc.height = _controllerHeight;
        controllerDesc.radius = _controllerRadius;
        controllerDesc.nonWalkableMode = physx::PxControllerNonWalkableMode::
            ePREVENT_CLIMBING_AND_FORCE_SLIDING;
        controllerDesc.invisibleWallHeight = 2.0f;
        controllerDesc.slopeLimit = cos(glm::radians(45.0f));
        controllerDesc.material = RigidBodyManager::_defaultMaterial;
      }
      _INTR_ASSERT(controllerDesc.isValid());

      NodeRef nodeRef =
          NodeManager::getComponentForEntity(_entity(charCtrlRef));
      controllerDesc.position =
          PhysicsHelper::convertExt(NodeManager::_worldPosition(nodeRef));

      pxController = _pxControllerManager->createController(controllerDesc);
      _INTR_ASSERT(pxController);
    }
  }
}

// <-

void CharacterControllerManager::destroyResources(
    const CharacterControllerRefArray& p_CharacterControllers)
{
  for (uint32_t cctIdx = 0u; cctIdx < p_CharacterControllers.size(); ++cctIdx)
  {
    CharacterControllerRef charCtrlRef = p_CharacterControllers[cctIdx];
    physx::PxController*& pxController = _pxController(charCtrlRef);
    _INTR_ASSERT(pxController);

    pxController->release();
    pxController = nullptr;

    _lastCollisionFlags(charCtrlRef) = 0x0;
    _currentMoveVector(charCtrlRef) = glm::vec3(0.0f);
    _internalMoveVector(charCtrlRef) = glm::vec3(0.0f);
  }
}
}
}
}
