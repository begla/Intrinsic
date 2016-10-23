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

void CameraManager::updateFrustums(const CameraRefArray& p_Cameras)
{
  _INTR_PROFILE_CPU("Components", "Cam. Matrix Updt.");

  for (uint32_t i = 0u; i < static_cast<uint32_t>(p_Cameras.size()); ++i)
  {
    CameraRef campCompRef = p_Cameras[i];
    Entity::EntityRef entityRef = _entity(campCompRef);
    Components::NodeRef nodeCompRef =
        Components::NodeManager::getComponentForEntity(entityRef);

    glm::vec3& forward = _forward(campCompRef);
    glm::vec3& up = _up(campCompRef);

    glm::quat& cameraOrientation = _actualCameraOrientation(campCompRef) =
        calcActualCameraOrientation(campCompRef);
    glm::vec3& camWorldPosition =
        Components::NodeManager::_worldPosition(nodeCompRef);

    forward = cameraOrientation * glm::vec3(0.0f, 0.0f, 1.0f);
    up = cameraOrientation * glm::vec3(0.0f, 1.0f, 0.0f);

    _viewMatrix(campCompRef) =
        glm::lookAt(camWorldPosition, camWorldPosition + forward, up);

    const float aspectRatio =
        Renderer::Vulkan::RenderSystem::_backbufferDimensions.x /
        (float)Renderer::Vulkan::RenderSystem::_backbufferDimensions.y;
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
  const float aspectRatio =
      Renderer::Vulkan::RenderSystem::_backbufferDimensions.x /
      (float)Renderer::Vulkan::RenderSystem::_backbufferDimensions.y;
  return glm::scale(glm::vec3(1.0f, -1.0f, 1.0f)) *
         glm::perspective(_descFov(p_Ref), aspectRatio, p_Near, p_Far);
}
}
}
}
