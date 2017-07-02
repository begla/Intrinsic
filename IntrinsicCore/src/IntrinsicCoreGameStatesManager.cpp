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
namespace GameStates
{
namespace
{
_INTR_ARRAY(GameStateEntry) _gameStates;
}
// Static members
GameState::Enum Manager::_activeGameState = GameState::kNone;

// <-

void Manager::init()
{
  // Register all available Game States
  {
    GameStateEntry entry = {};
    entry._type = GameState::kEditing;
    entry._activateFunction = Editing::activate;
    entry._deactivateFunction = Editing::deativate;
    entry._updateFunction = Editing::update;
    _gameStates.push_back(entry);
  }

  {
    GameStateEntry entry = {};
    entry._type = GameState::kBenchmark;
    entry._activateFunction = Benchmark::activate;
    entry._deactivateFunction = Benchmark::deativate;
    entry._updateFunction = Benchmark::update;
    _gameStates.push_back(entry);
  }

  {
    GameStateEntry entry = {};
    entry._type = GameState::kMain;
    entry._activateFunction = Main::activate;
    entry._deactivateFunction = Main::deativate;
    entry._updateFunction = Main::update;
    _gameStates.push_back(entry);
  }
}

void Manager::activate(GameState::Enum p_GameState)
{
  deactivate();

  if (_activeGameState != p_GameState)
  {
    const GameStateEntry& entry = getGameStateEntry(p_GameState);
    if (entry._activateFunction)
      entry._activateFunction();

    _activeGameState = p_GameState;
  }
}

// <-

void Manager::deactivate()
{
  const GameStateEntry& entry = getGameStateEntry(_activeGameState);
  if (entry._deactivateFunction)
    entry._deactivateFunction();

  _activeGameState = GameState::kNone;
}

// <-

void Manager::update(float p_DeltaT)
{
  _INTR_PROFILE_CPU("General", "Game State Manager");

  const GameStateEntry& entry = getGameStateEntry(_activeGameState);
  if (entry._updateFunction)
    entry._updateFunction(p_DeltaT);
}

const GameStateEntry& Manager::getGameStateEntry(GameState::Enum p_GameState)
{
  for (const GameStateEntry& entry : _gameStates)
  {
    if (entry._type == p_GameState)
    {
      return entry;
    }
  }

  static GameStateEntry defaultEntry = {};
  return defaultEntry;
}

// <-
}
}
}
