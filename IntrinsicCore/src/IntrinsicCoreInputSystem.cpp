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
namespace Input
{
uint8_t System::_keyStates[] = {};
float System::_axisStates[Axis::kCount] = {};
float System::_virtualKeyStates[_INTR_MAX_PLAYER_COUNT][VirtualKey::kCount] =
    {};

glm::vec2 System::_lastMousePos = glm::vec2(0.0f);
glm::vec2 System::_lastMousePosViewport = glm::vec2(0.0f);
glm::vec2 System::_lastMousePosRel = glm::vec2(0.0f);

_INTR_HASH_MAP(uint32_t, uint32_t) System::_keyToVirtualKeyMapping;
_INTR_HASH_MAP(uint32_t, uint32_t) System::_axisToVirtualKeyMapping;

// <-

void System::init()
{
  _keyToVirtualKeyMapping[Key::kW] = VirtualKey::kMoveUp;
  _keyToVirtualKeyMapping[Key::kS] = VirtualKey::kMoveDown;
  _keyToVirtualKeyMapping[Key::kD] = VirtualKey::kMoveRight;
  _keyToVirtualKeyMapping[Key::kA] = VirtualKey::kMoveLeft;
  _keyToVirtualKeyMapping[Key::kSpace] = VirtualKey::kJump;
  _keyToVirtualKeyMapping[Key::kShift] = VirtualKey::kRun;

  _keyToVirtualKeyMapping[Key::kControllerButtonX] = VirtualKey::kJump;
  _axisToVirtualKeyMapping[Axis::kLeftX] = VirtualKey::kMoveHorizontal;
  _axisToVirtualKeyMapping[Axis::kLeftY] = VirtualKey::kMoveVertical;
  _axisToVirtualKeyMapping[Axis::kRightX] = VirtualKey::kMoveCameraHorizontal;
  _axisToVirtualKeyMapping[Axis::kRightY] = VirtualKey::kMoveCameraVertical;
  _axisToVirtualKeyMapping[Axis::kTriggerRight] = VirtualKey::kRun;
}

void System::reset() { _lastMousePosRel = glm::vec2(0.0f); }

// <-

void System::processAxisEvent(const AxisEvent& p_Event)
{
  const float axisValue = p_Event.value;
  if (_axisStates[p_Event.axis] != axisValue)
  {
    _axisStates[p_Event.axis] = axisValue;

    Resources::QueuedEventData eventData;
    eventData.axisEvent.axis = p_Event.axis;
    eventData.axisEvent.value = axisValue;
    Resources::EventManager::queueEvent(_N(AxisChanged), eventData);

    auto virtualKey = _axisToVirtualKeyMapping.find(p_Event.axis);
    if (virtualKey != _axisToVirtualKeyMapping.end())
    {
      _virtualKeyStates[p_Event.playerId][virtualKey->second] = axisValue;
    }
  }
}

// <-

void System::processKeyPressEvent(const KeyEvent& p_Event)
{
  if (_keyStates[p_Event.key] != KeyState::kPressed)
  {
    _keyStates[p_Event.key] = KeyState::kPressed;

    Resources::QueuedEventData eventData;
    eventData.keyEvent.key = p_Event.key;
    Resources::EventManager::queueEvent(_N(KeyPressed), eventData);

    auto virtualKey = _keyToVirtualKeyMapping.find(p_Event.key);
    if (virtualKey != _keyToVirtualKeyMapping.end())
    {
      _virtualKeyStates[p_Event.playerId][virtualKey->second] = 1.0f;
    }
  }
}

// <-

void System::processKeyReleaseEvent(const KeyEvent& p_Event)
{
  if (_keyStates[p_Event.key] != KeyState::kReleased)
  {
    _keyStates[p_Event.key] = KeyState::kReleased;

    Resources::QueuedEventData eventData;
    eventData.keyEvent.key = p_Event.key;
    Resources::EventManager::queueEvent(_N(KeyReleased), eventData);

    auto virtualKey = _keyToVirtualKeyMapping.find(p_Event.key);
    if (virtualKey != _keyToVirtualKeyMapping.end())
    {
      _virtualKeyStates[p_Event.playerId][virtualKey->second] = 0.0f;
    }
  }
}

// <-

void System::processMouseMoveEvent(const MouseMoveEvent& p_Event)
{
  if (_lastMousePos != p_Event.pos ||
      _lastMousePosViewport != p_Event.posViewport ||
      _lastMousePosRel != p_Event.posRel)
  {
    Resources::QueuedEventData eventData;
    eventData.mouseEvent.pos[0] = p_Event.pos.x;
    eventData.mouseEvent.pos[1] = p_Event.pos.y;
    eventData.mouseEvent.posViewport[0] = p_Event.posViewport.x;
    eventData.mouseEvent.posViewport[1] = p_Event.posViewport.y;
    eventData.mouseEvent.posRel[0] = p_Event.posRel.x;
    eventData.mouseEvent.posRel[1] = p_Event.posRel.y;
    Resources::EventManager::queueEvent(_N(MouseMoved), eventData);

    _lastMousePosRel += p_Event.posRel;
    _lastMousePos = p_Event.pos;
    _lastMousePosViewport = p_Event.posViewport;
  }
}
}
}
}
