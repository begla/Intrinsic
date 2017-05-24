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
#include "stdafx_editor.h"
#include "stdafx.h"

IntrinsicEdViewport::IntrinsicEdViewport(QWidget* parent) : QWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  Resources::EventManager::connect(_N(MouseMoved),
                                   std::bind(&IntrinsicEdViewport::onMouseMoved,
                                             this, std::placeholders::_1));
  Resources::EventManager::connect(
      _N(AxisChanged), std::bind(&IntrinsicEdViewport::onAxisChanged, this,
                                 std::placeholders::_1));
  Resources::EventManager::connect(_N(KeyPressed),
                                   std::bind(&IntrinsicEdViewport::onKeyPressed,
                                             this, std::placeholders::_1));
  Resources::EventManager::connect(
      _N(KeyReleased), std::bind(&IntrinsicEdViewport::onKeyReleased, this,
                                 std::placeholders::_1));
}

IntrinsicEdViewport::~IntrinsicEdViewport() {}

void IntrinsicEdViewport::onKeyPressed(Resources::EventRef p_EventRef)
{
  const Resources::QueuedEventData& eventData =
      Resources::EventManager::_queuedEventData(p_EventRef);

  switch (eventData.keyEvent.key)
  {
  case Input::Key::kEscape:
    if (!IntrinsicEd::_mainWindow->_viewport->parent())
      IntrinsicEd::_mainWindow->onEndFullscreen();
    else
      IntrinsicEd::_mainWindow->onEditingGameState();
    break;
  case Input::Key::kF1:
    IntrinsicEd::_mainWindow->onEditingGameState();
    break;
  case Input::Key::kF2:
    IntrinsicEd::_mainWindow->onBenchmarkGameState();
    break;
  case Input::Key::kF3:
    IntrinsicEd::_mainWindow->onMainGameState();
    break;
  case Input::Key::k1:
    IntrinsicEd::_mainWindow->onEditingModeDefault();
    break;
  case Input::Key::k2:
    IntrinsicEd::_mainWindow->onEditingModeSelection();
    break;
  case Input::Key::k3:
    IntrinsicEd::_mainWindow->onEditingModeTranslation();
    break;
  case Input::Key::k4:
    IntrinsicEd::_mainWindow->onEditingModeRotation();
    break;
  case Input::Key::k5:
    IntrinsicEd::_mainWindow->onEditingModeScale();
    break;
  }
}

void IntrinsicEdViewport::onKeyReleased(Resources::EventRef p_EventRef) {}

void IntrinsicEdViewport::onAxisChanged(Resources::EventRef p_EventRef) {}

void IntrinsicEdViewport::onMouseMoved(Resources::EventRef p_EventRef) {}
