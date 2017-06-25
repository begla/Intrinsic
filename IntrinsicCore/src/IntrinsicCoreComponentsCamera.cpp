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
void CameraManager::init()
{
  _INTR_LOG_INFO("Inititializing Camera Component Manager...");

  Dod::Components::ComponentManagerBase<
      CameraData, _INTR_MAX_CAMERA_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry cameraEntry;
  {
    cameraEntry.createFunction = Components::CameraManager::createCamera;
    cameraEntry.destroyFunction = Components::CameraManager::destroyCamera;
    cameraEntry.resetToDefaultFunction =
        Components::CameraManager::resetToDefault;
    cameraEntry.getComponentForEntityFunction =
        Components::CameraManager::getComponentForEntity;

    Application::_componentManagerMapping[_N(Camera)] = cameraEntry;
    Application::_orderedComponentManagers.push_back(cameraEntry);
  }

  Dod::PropertyCompilerEntry propCompilerCamera;
  {
    propCompilerCamera.compileFunction =
        Components::CameraManager::compileDescriptor;
    propCompilerCamera.initFunction =
        Components::CameraManager::initFromDescriptor;
    propCompilerCamera.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(Camera)] =
        propCompilerCamera;
  }
}

// <-

void CameraManager::updateFrustumsAndMatrices(const CameraRefArray& p_Cameras)
{
  _INTR_PROFILE_CPU("General", "Cam. Matrix Updt.");

  for (uint32_t i = 0u; i < static_cast<uint32_t>(p_Cameras.size()); ++i)
  {
    CameraRef campCompRef = p_Cameras[i];
    Entity::EntityRef entityRef = _entity(campCompRef);
    Components::NodeRef nodeCompRef =
        Components::NodeManager::getComponentForEntity(entityRef);

    glm::vec3& forward = _forward(campCompRef);
    glm::vec3& up = _up(campCompRef);

    glm::vec3& camWorldPosition =
        Components::NodeManager::_worldPosition(nodeCompRef);

    forward = Components::NodeManager::_worldOrientation(nodeCompRef) *
              glm::vec3(0.0f, 0.0f, 1.0f);
    up = Components::NodeManager::_worldOrientation(nodeCompRef) *
         glm::vec3(0.0f, 1.0f, 0.0f);

    _prevViewMatrix(campCompRef) = _viewMatrix(campCompRef);
    _viewMatrix(campCompRef) =
        glm::lookAt(camWorldPosition, camWorldPosition + forward, up);

    _projectionMatrix(campCompRef) = computeCustomProjMatrix(
        campCompRef, _descNearPlane(campCompRef), _descFarPlane(campCompRef));

    Resources::FrustumManager::_descNearFarPlaneDistances(
        _frustum(campCompRef)) =
        glm::vec2(_descNearPlane(campCompRef), _descFarPlane(campCompRef));
    Resources::FrustumManager::_descProjectionType(_frustum(campCompRef)) =
        Resources::ProjectionType::kPerspective;
  }
}

// <-

glm::mat4 CameraManager::computeCustomProjMatrix(CameraRef p_Ref, float p_Near,
                                                 float p_Far)
{
  const float aspectRatio = R::RenderSystem::_backbufferDimensions.x /
                            (float)R::RenderSystem::_backbufferDimensions.y;
  return glm::scale(glm::vec3(1.0f, -1.0f, 1.0f)) *
         glm::perspective(_descFov(p_Ref), aspectRatio, p_Near, p_Far);
}
}
}
}
