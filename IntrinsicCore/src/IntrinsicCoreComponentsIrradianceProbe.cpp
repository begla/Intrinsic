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
void IrradianceProbeManager::init()
{
  _INTR_LOG_INFO("Inititializing IrradianceProbe Component Manager...");

  Dod::Components::ComponentManagerBase<
      IrradianceProbeData,
      _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry IrradianceProbeEntry;
  {
    IrradianceProbeEntry.createFunction =
        Components::IrradianceProbeManager::createIrradianceProbe;
    IrradianceProbeEntry.destroyFunction =
        Components::IrradianceProbeManager::destroyIrradianceProbe;
    IrradianceProbeEntry.getComponentForEntityFunction =
        Components::IrradianceProbeManager::getComponentForEntity;
    IrradianceProbeEntry.resetToDefaultFunction =
        Components::IrradianceProbeManager::resetToDefault;

    Application::_componentManagerMapping[_N(IrradianceProbe)] =
        IrradianceProbeEntry;
    Application::_orderedComponentManagers.push_back(IrradianceProbeEntry);
  }

  Dod::PropertyCompilerEntry propCompilerIrradianceProbe;
  {
    propCompilerIrradianceProbe.compileFunction =
        Components::IrradianceProbeManager::compileDescriptor;
    propCompilerIrradianceProbe.initFunction =
        Components::IrradianceProbeManager::initFromDescriptor;
    propCompilerIrradianceProbe.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(IrradianceProbe)] =
        propCompilerIrradianceProbe;
  }
}
}
}
}
