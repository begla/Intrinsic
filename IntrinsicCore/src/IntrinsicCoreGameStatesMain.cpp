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
        static const float camSpeedMouse = 0.5f;

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

      if (charCtrlRef.isValid())
      {
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

        moveVector = camOrient * moveVector;
        moveVector.y = 0.0f;
        moveVector *= actualMovedSpeed;

        if (Components::CharacterControllerManager::isGrounded(charCtrlRef) &&
            Input::System::getVirtualKeyState(Input::VirtualKey::kJump) > 0.0f)
        {
          moveVector.y += jumpSpeed;
        }

        Components::CharacterControllerManager::move(charCtrlRef, moveVector);
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
