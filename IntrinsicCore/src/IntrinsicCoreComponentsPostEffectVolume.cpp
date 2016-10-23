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
  CameraRef camRef = World::getActiveCamera();
  NodeRef camNodeRef =
      NodeManager::getComponentForEntity(CameraManager::_entity(camRef));
  const glm::vec3& camWorldPosition = NodeManager::_worldPosition(camNodeRef);

  bool volumeInRange = false;
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
        volumeInRange = true;
        const float blendFactor =
            1.0f -
            glm::clamp((distanceToCamera - (radius - blendRange)) / blendRange,
                       0.0f, 1.0f);

        Resources::PostEffectManager::blendPostEffect(
            Resources::PostEffectManager::_blendTargetRef,
            Resources::PostEffectManager::getResourceByName(_N(Default)),
            postEffect, blendFactor);

        // Only possible to blend to one scene setting at once
        break;
      }
    }
  }

  if (!volumeInRange)
  {
    // No volume in range? Use the default post effect
    Resources::PostEffectManager::blendPostEffect(
        Resources::PostEffectManager::_blendTargetRef,
        Resources::PostEffectManager::getResourceByName(_N(Default)),
        Resources::PostEffectManager::_blendTargetRef, 0.0f);
  }
}
}
}
}
