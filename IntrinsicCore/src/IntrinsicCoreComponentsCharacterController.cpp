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
}

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

  _pxControllerManager =
      PxCreateControllerManager(*Physics::System::_pxScene, false);
  _INTR_ASSERT(_pxControllerManager);
}

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
        PhysxHelper::convertExt(pxController->getPosition());
    const glm::vec3& currentWorldPos = NodeManager::_worldPosition(nodeRef);

    if (currentControllerPosition != currentWorldPos)
    {
      pxController->setPosition(PhysxHelper::convertExt(currentWorldPos));
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
      internalMoveVector.x += p_DeltaT * _currentMoveVector(charCtrlRef).x;
      internalMoveVector.z += p_DeltaT * _currentMoveVector(charCtrlRef).z;
    }
    else
    {
      glm::vec2 dampedXzMovement =
          glm::vec2(internalMoveVector.x, internalMoveVector.z);
      Math::dampSimple(dampedXzMovement, 0.01f, p_DeltaT);
      internalMoveVector.x = dampedXzMovement.x;
      internalMoveVector.z = dampedXzMovement.y;
    }
    internalMoveVector.y += _currentMoveVector(charCtrlRef).y;

    _currentMoveVector(charCtrlRef) = glm::vec3(0.0f);

    const physx::PxVec3 finalMoveVector =
        PhysxHelper::convert(p_DeltaT * internalMoveVector);
    physx::PxControllerFilters filters;

    const physx::PxControllerFlags currentFlags =
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
        PhysxHelper::convertExt(pxController->getPosition());

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

    NodeManager::updateTransforms(nodeRef);
  }
}

void CharacterControllerManager::createResources(
    const CharacterControllerRefArray& p_CharacterControllers)
{
  for (uint32_t meshIdx = 0u; meshIdx < p_CharacterControllers.size();
       ++meshIdx)
  {
    CharacterControllerRef charCtrlRef = p_CharacterControllers[meshIdx];

    physx::PxController*& pxController = _pxController(charCtrlRef);
    _INTR_ASSERT(pxController == nullptr);

    // Create capsule controller
    {
      physx::PxCapsuleControllerDesc controllerDesc;
      {
        controllerDesc.height = 2.0f;
        controllerDesc.radius = 0.5f;
        controllerDesc.nonWalkableMode = physx::PxControllerNonWalkableMode::
            ePREVENT_CLIMBING_AND_FORCE_SLIDING;
        controllerDesc.material = RigidBodyManager::_defaultMaterial;
      }
      _INTR_ASSERT(controllerDesc.isValid());

      NodeRef nodeRef =
          NodeManager::getComponentForEntity(_entity(charCtrlRef));
      controllerDesc.position =
          PhysxHelper::convertExt(NodeManager::_worldPosition(nodeRef));

      pxController = _pxControllerManager->createController(controllerDesc);
      _INTR_ASSERT(pxController);
    }
  }
}

void CharacterControllerManager::destroyResources(
    const CharacterControllerRefArray& p_CharacterControllers)
{
  for (uint32_t meshIdx = 0u; meshIdx < p_CharacterControllers.size();
       ++meshIdx)
  {
    CharacterControllerRef charCtrlRef = p_CharacterControllers[meshIdx];
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
