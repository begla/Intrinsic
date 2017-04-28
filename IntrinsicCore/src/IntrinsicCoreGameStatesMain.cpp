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

// PhysX includes
#include "extensions/PxDistanceJoint.h"
#include "PxRigidDynamic.h"
#include "extensions/PxRigidBodyExt.h"
#include "PxScene.h"
#include "extensions/PxJoint.h"

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{

void Main::init() {}

// <-

void Main::activate()
{
  Entity::EntityRef entityRef =
      Entity::EntityManager::getEntityByName(_N(GameCamera));
  _INTR_ASSERT(entityRef.isValid());
  Components::CameraRef cameraRef =
      Components::CameraManager::getComponentForEntity(entityRef);
  _INTR_ASSERT(cameraRef.isValid());

  World::setActiveCamera(cameraRef);
}

// <-

void Main::deativate() {}

// <-

void Main::update(float p_DeltaT)
{
  _INTR_PROFILE_CPU("Game States", "Main Game State Update");

  for (uint32_t i = 0u; i < Components::PlayerManager::_activeRefs.size(); ++i)
  {
    Components::PlayerRef playerRef = Components::PlayerManager::_activeRefs[i];
    Components::NodeRef playerNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::PlayerManager::_entity(playerRef));

    if (Components::PlayerManager::_descPlayerId(playerRef) == 0u)
    {
      Components::CameraRef camRef = World::getActiveCamera();
      const glm::quat& camOrient =
          Components::CameraManager::_actualCameraOrientation(camRef);

      Components::CharacterControllerRef charCtrlRef =
          Components::CharacterControllerManager::getComponentForEntity(
              Components::PlayerManager::_entity(playerRef));
      Components::CameraControllerRef camCtrlRef =
          Components::CameraControllerManager::getComponentForEntity(
              Components::CameraManager::_entity(camRef));

      if (camCtrlRef.isValid())
      {
        static const float camSpeed = 2.0f;
        static const float camSpeedMouse = 0.75f;

        glm::vec3& targetEulerAngles =
            Components::CameraControllerManager::_descTargetEulerAngles(
                camCtrlRef);

        targetEulerAngles.y += -camSpeed *
                               Input::System::getVirtualKeyState(
                                   Input::VirtualKey::kMoveCameraHorizontal) *
                               p_DeltaT;
        targetEulerAngles.x += camSpeed *
                               Input::System::getVirtualKeyState(
                                   Input::VirtualKey::kMoveCameraVertical) *
                               p_DeltaT;

        targetEulerAngles.y +=
            -camSpeedMouse * Input::System::getLastMousePosRel().x * p_DeltaT;
        targetEulerAngles.x +=
            camSpeedMouse * Input::System::getLastMousePosRel().y * p_DeltaT;
      }

      static const float moveSpeed = 8.0f;
      static const float runMultiplier = 0.5f;
      static const float jumpSpeed = 20.0f;

      const float actualMovedSpeed =
          moveSpeed * (1.0f +
                       runMultiplier * Input::System::getVirtualKeyState(
                                           Input::VirtualKey::kRun));

      glm::vec3 moveVector = glm::vec3(0.0f);
      {
        moveVector +=
            Input::System::getVirtualKeyState(Input::VirtualKey::kMoveUp) *
            glm::vec3(0.0f, 0.0f, 1.0f);
        moveVector +=
            Input::System::getVirtualKeyState(Input::VirtualKey::kMoveDown) *
            glm::vec3(0.0f, 0.0f, -1.0f);
        moveVector +=
            Input::System::getVirtualKeyState(Input::VirtualKey::kMoveRight) *
            glm::vec3(-1.0f, 0.0f, 0.0f);
        moveVector +=
            Input::System::getVirtualKeyState(Input::VirtualKey::kMoveLeft) *
            glm::vec3(1.0f, 0.0f, 0.0f);

        moveVector += glm::vec3(-Input::System::getVirtualKeyState(
                                    Input::VirtualKey::kMoveHorizontal),
                                0.0f, 0.0f);
        moveVector +=
            glm::vec3(0.0f, 0.0f, -Input::System::getVirtualKeyState(
                                      Input::VirtualKey::kMoveVertical));
      }

      moveVector = camOrient * glm::normalize(moveVector);
      moveVector.y = 0.0f;
      moveVector *= actualMovedSpeed;

      if (charCtrlRef.isValid())
      {
        if (Components::CharacterControllerManager::isGrounded(charCtrlRef) &&
            Input::System::getVirtualKeyState(Input::VirtualKey::kJump) > 0.0f)
        {
          moveVector.y += jumpSpeed;
        }

        Components::CharacterControllerManager::move(charCtrlRef, moveVector);

        // Rotate the player into the direction of the move vector
        glm::vec3 actualMoveVector =
            Components::CharacterControllerManager::_internalMoveVector(
                charCtrlRef);
        actualMoveVector.y = 0.0f;
        const float movVecLen = glm::length(actualMoveVector);

        if (movVecLen > _INTR_EPSILON)
        {
          const glm::vec3 playerOrientation = actualMoveVector / movVecLen;
          Components::NodeManager::_orientation(playerNodeRef) =
              glm::rotation(glm::vec3(0.0, 0.0, 1.0), playerOrientation);
          Components::NodeManager::updateTransforms(playerNodeRef);
        }
      }
    }

    Components::CameraControllerManager::updateControllers(
        Components::CameraControllerManager::_activeRefs, p_DeltaT);
    Components::CharacterControllerManager::updateControllers(
        Components::CharacterControllerManager::_activeRefs, p_DeltaT);
    Components::ScriptManager::tickScripts(
        Components::ScriptManager::_activeRefs, p_DeltaT);
  }
}
}
}
}
