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
    IntrinsicEd::_mainWindow->onEndFullscreen();
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
