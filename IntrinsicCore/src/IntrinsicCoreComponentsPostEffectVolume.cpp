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
void PostEffectVolumeManager::init()
{
  _INTR_LOG_INFO("Inititializing PostEffectVolume Component Manager...");

  Dod::Components::ComponentManagerBase<
      PostEffectVolumeData, _INTR_MAX_POST_EFFECT_CONTROLLER_COMPONENT_COUNT>::
      _initComponentManager();

  Dod::Components::ComponentManagerEntry postEffectVolumeEntry;
  {
    postEffectVolumeEntry.createFunction =
        Components::PostEffectVolumeManager::createPostEffectVolume;
    postEffectVolumeEntry.destroyFunction =
        Components::PostEffectVolumeManager::destroyPostEffectVolume;
    postEffectVolumeEntry.getComponentForEntityFunction =
        Components::PostEffectVolumeManager::getComponentForEntity;
    postEffectVolumeEntry.resetToDefaultFunction =
        Components::PostEffectVolumeManager::resetToDefault;

    Application::_componentManagerMapping[_N(PostEffectVolume)] =
        postEffectVolumeEntry;
    Application::_orderedComponentManagers.push_back(postEffectVolumeEntry);
  }

  Dod::PropertyCompilerEntry propCompilerPostEffectVolume;
  {
    propCompilerPostEffectVolume.compileFunction =
        Components::PostEffectVolumeManager::compileDescriptor;
    propCompilerPostEffectVolume.initFunction =
        Components::PostEffectVolumeManager::initFromDescriptor;
    propCompilerPostEffectVolume.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(PostEffectVolume)] =
        propCompilerPostEffectVolume;
  }
}

// <-

void PostEffectVolumeManager::blendPostEffects(
    const PostEffectVolumeRefArray& p_PostEffectVolumes)
{
  CameraRef camRef = World::_activeCamera;
  NodeRef camNodeRef =
      NodeManager::getComponentForEntity(CameraManager::_entity(camRef));
  const glm::vec3& camWorldPosition = NodeManager::_worldPosition(camNodeRef);

  const Resources::PostEffectRef defaultEffectRef =
      Resources::PostEffectManager::getResourceByName(_N(Default));

  // Always start with the default effect
  Resources::PostEffectManager::blendPostEffect(
      Resources::PostEffectManager::_blendTargetRef, defaultEffectRef,
      defaultEffectRef, 0.0f);

  for (uint32_t i = 0u; i < p_PostEffectVolumes.size(); ++i)
  {
    PostEffectVolumeRef postVolRef = p_PostEffectVolumes[i];
    NodeRef postVolNodeRef =
        NodeManager::getComponentForEntity(_entity(postVolRef));
    Resources::PostEffectRef postEffect =
        Resources::PostEffectManager::_getResourceByName(
            _descPostEffectName(postVolRef));

    if (postEffect.isValid())
    {
      const float radius = _descRadius(postVolRef);
      const float blendRange = _descBlendRange(postVolRef);
      const glm::vec3& volumeWorldPostion =
          NodeManager::_worldPosition(postVolNodeRef);

      const float distanceToCamera =
          glm::distance(volumeWorldPostion, camWorldPosition);

      if (distanceToCamera < radius)
      {
        const float blendFactor =
            1.0f -
            glm::clamp((distanceToCamera - (radius - blendRange)) / blendRange,
                       0.0f, 1.0f);

        Resources::PostEffectManager::blendPostEffect(
            Resources::PostEffectManager::_blendTargetRef,
            Resources::PostEffectManager::_blendTargetRef, postEffect,
            blendFactor);
      }
    }
  }
}
}
}
}
