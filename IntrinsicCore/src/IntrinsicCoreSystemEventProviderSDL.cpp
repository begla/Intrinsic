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

namespace Intrinsic
{
namespace Core
{
namespace SystemEventProvider
{
namespace
{
const uint32_t _maxNumGameControllers = 4;
SDL_GameController* _gameControllers[_maxNumGameControllers] = {};
uint32_t _connectedControllerCount = 0u;
}

// <-

void SDL::init() {}

// <-

void SDL::pumpEvents()
{
  SDL_Event sdlEvent;
  while (SDL_PollEvent(&sdlEvent))
  {
    if (sdlEvent.type == SDL_KEYDOWN || sdlEvent.type == SDL_KEYUP)
    {
      Input::KeyEvent keyEvent;
      keyEvent.key = (Input::Key::Enum)((uint32_t)-1);

      // Keys
      switch (sdlEvent.key.keysym.sym)
      {
      case SDLK_UP:
        keyEvent.key = Input::Key::kUp;
        break;
      case SDLK_DOWN:
        keyEvent.key = Input::Key::kDown;
        break;
      case SDLK_RIGHT:
        keyEvent.key = Input::Key::kRight;
        break;
      case SDLK_LEFT:
        keyEvent.key = Input::Key::kLeft;
        break;
      case SDLK_w:
        keyEvent.key = Input::Key::kW;
        break;
      case SDLK_a:
        keyEvent.key = Input::Key::kA;
        break;
      case SDLK_s:
        keyEvent.key = Input::Key::kS;
        break;
      case SDLK_d:
        keyEvent.key = Input::Key::kD;
        break;
      case SDLK_f:
        keyEvent.key = Input::Key::kF;
        break;
      case SDLK_t:
        keyEvent.key = Input::Key::kT;
        break;
      case SDLK_F1:
        keyEvent.key = Input::Key::kF1;
        break;
      case SDLK_F2:
        keyEvent.key = Input::Key::kF2;
        break;
      case SDLK_F3:
        keyEvent.key = Input::Key::kF3;
        break;
      case SDLK_F10:
        keyEvent.key = Input::Key::kF10;
        break;
      case SDLK_0:
        keyEvent.key = Input::Key::k0;
        break;
      case SDLK_1:
        keyEvent.key = Input::Key::k1;
        break;
      case SDLK_2:
        keyEvent.key = Input::Key::k2;
        break;
      case SDLK_3:
        keyEvent.key = Input::Key::k3;
        break;
      case SDLK_4:
        keyEvent.key = Input::Key::k4;
        break;
      case SDLK_5:
        keyEvent.key = Input::Key::k5;
        break;
      case SDLK_6:
        keyEvent.key = Input::Key::k6;
        break;
      case SDLK_7:
        keyEvent.key = Input::Key::k7;
        break;
      case SDLK_8:
        keyEvent.key = Input::Key::k8;
        break;
      case SDLK_9:
        keyEvent.key = Input::Key::k9;
        break;
      case SDLK_LSHIFT:
        keyEvent.key = Input::Key::kShift;
        break;
      case SDLK_LALT:
        keyEvent.key = Input::Key::kAlt;
        break;
      case SDLK_LCTRL:
        keyEvent.key = Input::Key::kCtrl;
        break;
      case SDLK_SPACE:
        keyEvent.key = Input::Key::kSpace;
        break;
      case SDLK_ESCAPE:
        keyEvent.key = Input::Key::kEscape;
        break;
      case SDLK_DELETE:
        keyEvent.key = Input::Key::kDel;
        break;
      case SDLK_BACKSPACE:
        keyEvent.key = Input::Key::kBackspace;
        break;
      }

      if (keyEvent.key != (uint32_t)-1)
      {
        if (sdlEvent.type == SDL_KEYDOWN)
        {
          Input::System::processKeyPressEvent(keyEvent);
        }
        else
        {
          Input::System::processKeyReleaseEvent(keyEvent);
        }
      }
    }
    else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN ||
             sdlEvent.type == SDL_MOUSEBUTTONUP)
    {
      Input::KeyEvent keyEvent;
      keyEvent.key = (Input::Key::Enum)((uint32_t)-1);

      switch (sdlEvent.button.button)
      {
      case SDL_BUTTON_LEFT:
        keyEvent.key = Input::Key::kMouseLeft;
        break;
      case SDL_BUTTON_RIGHT:
        keyEvent.key = Input::Key::kMouseRight;
        break;
      }

      if (keyEvent.key != (uint32_t)-1)
      {
        if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
        {
          Input::System::processKeyPressEvent(keyEvent);
        }
        else
        {
          Input::System::processKeyReleaseEvent(keyEvent);
        }
      }
    }
    else if (sdlEvent.type == SDL_MOUSEMOTION)
    {
      Input::MouseMoveEvent mouseEvent = {
          glm::vec2(sdlEvent.motion.x, sdlEvent.motion.y),
          glm::vec2(sdlEvent.motion.x, sdlEvent.motion.y) /
              glm::vec2(RV::RenderSystem::_backbufferDimensions)};

      Input::System::processMouseMoveEvent(mouseEvent);
    }
    else if (sdlEvent.type == SDL_WINDOWEVENT)
    {
      switch (sdlEvent.window.event)
      {
      case SDL_WINDOWEVENT_RESIZED:
        if (sdlEvent.window.data1 !=
                RV::RenderSystem::_backbufferDimensions.x ||
            sdlEvent.window.data2 != RV::RenderSystem::_backbufferDimensions.y)
        {
          RV::RenderSystem::onViewportChanged();
        }
        break;
      case SDL_WINDOWEVENT_CLOSE:
        Application::shutdown();
        break;
      }
    }
    else if (sdlEvent.type == SDL_CONTROLLERDEVICEADDED)
    {
      if (sdlEvent.cdevice.which < _maxNumGameControllers)
      {
        _gameControllers[sdlEvent.cdevice.which] =
            SDL_GameControllerOpen(sdlEvent.cdevice.which);
        ++_connectedControllerCount;
      }
    }
    else if (sdlEvent.type == SDL_CONTROLLERDEVICEREMOVED)
    {
      if (sdlEvent.cdevice.which < _maxNumGameControllers)
      {
        SDL_GameControllerClose(_gameControllers[sdlEvent.cdevice.which]);
        _gameControllers[sdlEvent.cdevice.which] = nullptr;
      }
    }
    else if (sdlEvent.type == SDL_CONTROLLERBUTTONUP ||
             sdlEvent.type == SDL_CONTROLLERBUTTONDOWN)
    {
      Input::KeyEvent keyEvent;
      keyEvent.key = (Input::Key::Enum)(uint32_t)-1;

      switch (sdlEvent.cbutton.button)
      {
      case SDL_CONTROLLER_BUTTON_A:
        keyEvent.key = Input::Key::kControllerButtonA;
        break;
      case SDL_CONTROLLER_BUTTON_B:
        keyEvent.key = Input::Key::kControllerButtonB;
        break;
      case SDL_CONTROLLER_BUTTON_X:
        keyEvent.key = Input::Key::kControllerButtonX;
        break;
      case SDL_CONTROLLER_BUTTON_Y:
        keyEvent.key = Input::Key::kControllerButtonY;
        break;
      }

      if (keyEvent.key != (uint32_t)-1)
      {
        if (sdlEvent.type == SDL_CONTROLLERBUTTONDOWN)
        {
          Input::System::processKeyPressEvent(keyEvent);
        }
        else
        {
          Input::System::processKeyReleaseEvent(keyEvent);
        }
      }
    }
    else if (sdlEvent.type == SDL_CONTROLLERAXISMOTION)
    {
      Input::AxisEvent axisEvent;
      axisEvent.axis = (Input::Axis::Enum)(uint32_t)-1;
      axisEvent.value = (float)sdlEvent.caxis.value / INT16_MAX;

      switch (sdlEvent.caxis.axis)
      {
      case SDL_CONTROLLER_AXIS_LEFTX:
        axisEvent.axis = Input::Axis::kLeftX;
        break;
      case SDL_CONTROLLER_AXIS_LEFTY:
        axisEvent.axis = Input::Axis::kLeftY;
        break;
      case SDL_CONTROLLER_AXIS_RIGHTX:
        axisEvent.axis = Input::Axis::kRightX;
        break;
      case SDL_CONTROLLER_AXIS_RIGHTY:
        axisEvent.axis = Input::Axis::kRightY;
        break;
      case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        axisEvent.axis = Input::Axis::kTriggerLeft;
        break;
      case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        axisEvent.axis = Input::Axis::kTriggerRight;
        break;
      }

      if (axisEvent.axis != (uint32_t)-1)
      {
        Input::System::processAxisEvent(axisEvent);
      }
    }
  }
}
}
}
}
