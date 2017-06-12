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
namespace Components
{
void PlayerManager::init()
{
  _INTR_LOG_INFO("Inititializing Player Component Manager...");

  Dod::Components::ComponentManagerBase<
      PlayerData, _INTR_MAX_PLAYER_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry playerEntry;
  {
    playerEntry.createFunction = Components::PlayerManager::createPlayer;
    playerEntry.destroyFunction = Components::PlayerManager::destroyPlayer;
    playerEntry.getComponentForEntityFunction =
        Components::PlayerManager::getComponentForEntity;
    playerEntry.resetToDefaultFunction =
        Components::PlayerManager::resetToDefault;

    Application::_componentManagerMapping[_N(Player)] = playerEntry;
    Application::_orderedComponentManagers.push_back(playerEntry);
  }

  Dod::PropertyCompilerEntry propCompilerPlayer;
  {
    propCompilerPlayer.compileFunction =
        Components::PlayerManager::compileDescriptor;
    propCompilerPlayer.initFunction =
        Components::PlayerManager::initFromDescriptor;
    propCompilerPlayer.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(Player)] =
        propCompilerPlayer;
  }
}
}
}
}
