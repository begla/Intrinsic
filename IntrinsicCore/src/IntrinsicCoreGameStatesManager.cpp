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
    case GameState::kBenchmark:
      Benchmark::activate();
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
  case GameState::kBenchmark:
    Benchmark::deativate();
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
  case GameState::kBenchmark:
    Benchmark::update(p_DeltaT);
    break;
    ;
  }
}

// <-
}
}
}
