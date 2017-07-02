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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{

namespace GameState
{
enum Enum
{
  kNone,
  kEditing,
  kMain,
  kBenchmark
};
}

typedef void (*GameStateActivateFunction)();
typedef void (*GameStateDeactivateFunction)();
typedef void (*GameStateUpdateFunction)(float);

/**
 * Struct describing a single Game State and its available functions.
 */
struct GameStateEntry
{
  uint8_t _type;

  GameStateActivateFunction _activateFunction;
  GameStateDeactivateFunction _deactivateFunction;
  GameStateUpdateFunction _updateFunction;
};

struct Manager
{
  static void init();

  static void activate(GameState::Enum p_GameState);
  static void deactivate();
  static void update(float p_DeltaT);

  static const GameStateEntry& getGameStateEntry(GameState::Enum p_GameState);
  static GameState::Enum getActiveGameState() { return _activeGameState; }

private:
  static GameState::Enum _activeGameState;
};
}
}
}
