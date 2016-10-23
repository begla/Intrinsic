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

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{
// Static members
GameState::Enum Manager::_activeGameState = GameState::kNone;

// <-

void Manager::activateGameState(GameState::Enum p_GameState)
{
  deactivateGameState();

  if (_activeGameState != p_GameState)
  {
    switch (p_GameState)
    {
    case GameState::kEditing:
      Editing::activate();
      break;
    case GameState::kMain:
      Main::activate();
      break;
    }

    _activeGameState = p_GameState;
  }
}

// <-

void Manager::deactivateGameState()
{
  switch (_activeGameState)
  {
  case GameState::kEditing:
    Editing::deativate();
    break;
  case GameState::kMain:
    Main::deativate();
    break;
  }

  _activeGameState = GameState::kNone;
}

// <-

void Manager::update(float p_DeltaT)
{
  switch (_activeGameState)
  {
  case GameState::kEditing:
    Editing::update(p_DeltaT);
    break;
  case GameState::kMain:
    Main::update(p_DeltaT);
    break;
  }
}

// <-
}
}
}
