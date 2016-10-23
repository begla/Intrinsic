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
namespace Resources
{
// Static members
PostEffectRef PostEffectManager::_blendTargetRef;

void PostEffectManager::init()
{
  _INTR_LOG_INFO("Inititializing Post Effect Manager...");

  Dod::Resources::ResourceManagerBase<
      PostEffectData, _INTR_MAX_POST_EFFECT_COUNT>::_initResourceManager();

  Dod::Resources::ResourceManagerEntry managerEntry;
  {
    managerEntry.createFunction =
        Resources::PostEffectManager::createPostEffect;
    managerEntry.destroyFunction =
        Resources::PostEffectManager::destroyPostEffect;
    managerEntry.getActiveResourceAtIndexFunction =
        Resources::PostEffectManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        Resources::PostEffectManager::getActiveResourceCount;
    managerEntry.loadFromSingleFileFunction =
        Resources::PostEffectManager::loadFromSingleFile;
    managerEntry.saveToSingleFileFunction =
        Resources::PostEffectManager::saveToSingleFile;
    managerEntry.resetToDefaultFunction =
        Resources::PostEffectManager::resetToDefault;

    Application::_resourceManagerMapping[_N(PostEffect)] = managerEntry;
  }

  Dod::PropertyCompilerEntry propertyEntry;
  {
    propertyEntry.compileFunction =
        Resources::PostEffectManager::compileDescriptor;
    propertyEntry.initFunction =
        Resources::PostEffectManager::initFromDescriptor;
    propertyEntry.ref = Dod::Ref();

    Application::_resourcePropertyCompilerMapping[_N(PostEffect)] =
        propertyEntry;
  }

  // Create the "system" post effects
  _blendTargetRef = createPostEffect(_N(BlendTarget));
  addResourceFlags(_blendTargetRef,
                   Dod::Resources::ResourceFlags::kResourceVolatile);
  resetToDefault(_blendTargetRef);
}
}
}
}
