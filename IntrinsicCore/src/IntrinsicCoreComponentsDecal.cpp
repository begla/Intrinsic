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
void DecalManager::init()
{
  _INTR_LOG_INFO("Inititializing Decal Component Manager...");

  Dod::Components::ComponentManagerBase<
      DecalData, _INTR_MAX_DECAL_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry DecalEntry;
  {
    DecalEntry.createFunction = Components::DecalManager::createDecal;
    DecalEntry.destroyFunction = Components::DecalManager::destroyDecal;
    DecalEntry.getComponentForEntityFunction =
        Components::DecalManager::getComponentForEntity;
    DecalEntry.resetToDefaultFunction =
        Components::DecalManager::resetToDefault;

    Application::_componentManagerMapping[_N(Decal)] = DecalEntry;
    Application::_orderedComponentManagers.push_back(DecalEntry);
  }

  Dod::PropertyCompilerEntry propCompilerDecal;
  {
    propCompilerDecal.compileFunction =
        Components::DecalManager::compileDescriptor;
    propCompilerDecal.initFunction =
        Components::DecalManager::initFromDescriptor;
    propCompilerDecal.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(Decal)] =
        propCompilerDecal;
  }
}
}
}
}
