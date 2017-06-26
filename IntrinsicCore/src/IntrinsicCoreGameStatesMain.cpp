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
namespace
{
uint32_t _currentlyActivePlayerId = 0u;
}

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
  _INTR_PROFILE_CPU("Game States", "Main");

  // Avoid time mod.
  const float deltaT = TaskManager::_lastDeltaT;

  for (uint32_t i = 0u; i < Components::PlayerManager::_activeRefs.size(); ++i)
  {
    Components::PlayerRef playerRef = Components::PlayerManager::_activeRefs[i];
    Components::NodeRef playerNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::PlayerManager::_entity(playerRef));

    Components::CameraRef camRef = World::_activeCamera;
    Components::NodeRef camNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(camRef));

    const glm::quat& camOrient =
        Components::NodeManager::_worldOrientation(camNodeRef);

    Components::CharacterControllerRef charCtrlRef =
        Components::CharacterControllerManager::getComponentForEntity(
            Components::PlayerManager::_entity(playerRef));
    Components::CameraControllerRef camCtrlRef =
        Components::CameraControllerManager::getComponentForEntity(
            Components::CameraManager::_entity(camRef));

    const uint32_t playerId =
        Components::PlayerManager::_descPlayerId(playerRef);
    const glm::vec4 movement = Input::System::getMovementFiltered(playerId);

    if (camCtrlRef.isValid() && playerId == _currentlyActivePlayerId)
    {
      static const float camSpeed = 3.0f;
      static const float camSpeedMouse = 0.075f;

      glm::vec3& targetEulerAngles =
          Components::CameraControllerManager::_descTargetEulerAngles(
              camCtrlRef);

      targetEulerAngles.y += -camSpeed * movement.w * deltaT;
      targetEulerAngles.x += camSpeed * movement.z * deltaT;

      targetEulerAngles.y +=
          -camSpeedMouse * Input::System::getLastMousePosRel().x * deltaT;
      targetEulerAngles.x +=
          camSpeedMouse * Input::System::getLastMousePosRel().y * deltaT;
    }

    static const float moveSpeed = 500.0f;
    static const float runMultiplier = 0.5f;
    static const float jumpSpeed = 20.0f;

    const float actualMovedSpeed =
        moveSpeed * (1.0f +
                     runMultiplier * Input::System::getVirtualKeyState(
                                         Input::VirtualKey::kRun, playerId));

    glm::vec3 moveVector = glm::vec3(0.0f);
    {
      moveVector += Input::System::getVirtualKeyState(
                        Input::VirtualKey::kMoveUp, playerId) *
                    glm::vec3(0.0f, 0.0f, 1.0f);
      moveVector += Input::System::getVirtualKeyState(
                        Input::VirtualKey::kMoveDown, playerId) *
                    glm::vec3(0.0f, 0.0f, -1.0f);
      moveVector += Input::System::getVirtualKeyState(
                        Input::VirtualKey::kMoveRight, playerId) *
                    glm::vec3(-1.0f, 0.0f, 0.0f);
      moveVector += Input::System::getVirtualKeyState(
                        Input::VirtualKey::kMoveLeft, playerId) *
                    glm::vec3(1.0f, 0.0f, 0.0f);

      moveVector += glm::vec3(-movement.y, 0.0f, 0.0f);
      moveVector += glm::vec3(0.0f, 0.0f, -movement.x);
    }

    const float moveVecLen = glm::length(moveVector);
    if (moveVecLen < Settings::Manager::_controllerDeadZone)
      moveVector = glm::vec3(0.0f);
    else if (moveVecLen > 1.0f)
      moveVector /= moveVecLen;

    moveVector = camOrient * moveVector;
    moveVector.y = 0.0f;
    moveVector *= actualMovedSpeed;

    if (charCtrlRef.isValid())
    {
      if (Components::CharacterControllerManager::isGrounded(charCtrlRef) &&
          Input::System::getVirtualKeyState(Input::VirtualKey::kJump,
                                            playerId) > 0.0f)
      {
        moveVector.y += jumpSpeed;
      }

      Components::CharacterControllerManager::move(charCtrlRef, moveVector);
    }
  }

  Components::CameraControllerManager::updateControllers(
      Components::CameraControllerManager::_activeRefs, deltaT);
  Components::CharacterControllerManager::updateControllers(
      Components::CharacterControllerManager::_activeRefs, p_DeltaT);
}
}
}
}
