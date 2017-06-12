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
    managerEntry.loadFromMultipleFilesFunction =
        Resources::PostEffectManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction =
        Resources::PostEffectManager::saveToMultipleFiles;
    managerEntry.resetToDefaultFunction =
        Resources::PostEffectManager::resetToDefault;
    managerEntry.getResourceFlagsFunction = PostEffectManager::_resourceFlags;

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
