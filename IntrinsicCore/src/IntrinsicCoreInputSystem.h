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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Input
{
namespace KeyState
{
enum Enum
{
  kReleased,
  kPressed
};
}

namespace VirtualKey
{
enum Enum
{
  kMoveUp,
  kMoveDown,
  kMoveLeft,
  kMoveRight,

  kMoveHorizontal,
  kMoveVertical,

  kMoveCameraHorizontal,
  kMoveCameraVertical,

  kJump,
  kRun,

  kCount
};
}

namespace Key
{
enum Enum
{
  kUp,
  kDown,
  kLeft,
  kRight,

  kW,
  kA,
  kS,
  kD,

  k0,
  k1,
  k2,
  k3,
  k4,
  k5,
  k6,
  k7,
  k8,
  k9,

  kF,

  kMouseLeft,
  kMouseRight,

  kShift,
  kAlt,
  kCtrl,

  kSpace,
  kEscape,

  kControllerButtonA,
  kControllerButtonY,
  kControllerButtonB,
  kControllerButtonX,

  kCount
};
}

namespace Axis
{
enum Enum
{
  kLeftX,
  kLeftY,
  kRightX,
  kRightY,

  kTriggerLeft,
  kTriggerRight,

  kCount
};
}

struct KeyEvent
{
  Key::Enum key;
};

struct AxisEvent
{
  Axis::Enum axis;
  float value;
};

struct MouseMoveEvent
{
  glm::vec2 pos;
  glm::vec2 posViewport;
  glm::vec2 posRel;
};

struct System
{
  static void init();
  static void reset();

  static void processAxisEvent(const AxisEvent& p_Event);
  static void processKeyPressEvent(const KeyEvent& p_Event);
  static void processKeyReleaseEvent(const KeyEvent& p_Event);
  static void processMouseMoveEvent(const MouseMoveEvent& p_Event);

  _INTR_INLINE static void resetKeyStates()
  {
    for (uint32_t i = 0u; i < Key::kCount; ++i)
    {
      _keyStates[i] = KeyState::kReleased;
    }
  }

  _INTR_INLINE static uint8_t& getKeyState(Key::Enum p_Key)
  {
    return _keyStates[p_Key];
  }
  _INTR_INLINE static uint8_t* getKeyStates() { return _keyStates; }
  _INTR_INLINE static float& getAxisState(Key::Enum p_Key)
  {
    return _axisStates[p_Key];
  }
  _INTR_INLINE static float* getAxisStates() { return _axisStates; }
  _INTR_INLINE static float& getVirtualKeyState(VirtualKey::Enum p_Key)
  {
    return _virtualKeyStates[p_Key];
  }
  _INTR_INLINE static float* getVirtualKeyStates() { return _virtualKeyStates; }
  _INTR_INLINE static glm::vec2& getLastMousePos() { return _lastMousePos; }
  _INTR_INLINE static glm::vec2& getLastMousePosViewport()
  {
    return _lastMousePosViewport;
  }
  _INTR_INLINE static glm::vec2& getLastMousePosRel()
  {
    return _lastMousePosRel;
  }

private:
  static uint8_t _keyStates[Key::kCount];
  static float _axisStates[Axis::kCount];

  static glm::vec2 _lastMousePos;
  static glm::vec2 _lastMousePosViewport;
  static glm::vec2 _lastMousePosRel;

  static _INTR_HASH_MAP(uint32_t, uint32_t) _keyToVirtualKeyMapping;
  static _INTR_HASH_MAP(uint32_t, uint32_t) _axisToVirtualKeyMapping;

  static float _virtualKeyStates[VirtualKey::kCount];
};
}
}
}
