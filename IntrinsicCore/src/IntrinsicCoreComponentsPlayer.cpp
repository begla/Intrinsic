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
