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

// PhysX includes
#include "extensions/PxDistanceJoint.h"
#include "PxRigidDynamic.h"
#include "extensions/PxRigidBodyExt.h"
#include "PxScene.h"
#include "extensions/PxJoint.h"

using namespace RResources;

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{
namespace
{
bool _rightMouseButtonPressed = false;
bool _leftMouseButtonPressed = false;

// Gizmos
bool _XAxisSelected = false;
bool _YAxisSelected = false;
bool _ZAxisSelected = false;

bool _XZPlaneSelected = false;
bool _XYPlaneSelected = false;
bool _YZPlaneSelected = false;

bool _anyPlaneSelected = false;
bool _anyAxisSelected = false;

glm::vec3 _initialPosOffset = glm::vec3(0.0f);
glm::vec3 _initialRotationDir = glm::vec3(0.0f);
glm::vec3 _initialScale = glm::vec3(1.0f);
glm::quat _initialOrientation = glm::quat();
glm::vec3 _selectedPlaneNormal = glm::vec3(0.0f);

float _lastGizmoScale = 1.0f;
const float _mouseSens = glm::half_pi<float>() * 0.025f;
const float _controllerSens = 10.0f;

// Grid
glm::vec3 _gridPosition = glm::vec3(0.0f);
float _gridFade = 0.0f;

// Editing cams
CameraMode::Enum _cameraMode = CameraMode::kFreeFlight;
glm::vec3 _orbitCamCenter = glm::vec3(0.0f);
float _orbitRadius = 200.0f;
glm::vec3 _camVel = glm::vec3();
glm::vec3 _camAngVel = glm::vec3();
glm::vec3 _eulerAngles = glm::vec3();

// Cloning
bool _cloningInProgress = false;

// Physics grabbing
physx::PxDistanceJoint* _pickingJoint = nullptr;
physx::PxRigidDynamic* _pickingDummyActor = nullptr;
float _pickingInitialDistance = 0.0f;

// <-

glm::vec3 _planeVectors[] = {
    glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)};

// <-

struct PerInstanceDataGizmoVertex
{
  glm::mat4 worldMatrix;
  glm::mat4 worldViewProjMatrix;
  glm::mat4 viewMatrix;

  glm::vec4 colorTintX;
  glm::vec4 colorTintY;
  glm::vec4 colorTintZ;
};

// <-

struct PerInstanceDataGizmoFragment
{
  float _dummy;
};

// <-

struct PerInstanceDataGridVertex
{
  glm::mat4 worldMatrix;
  glm::mat4 worldViewProjMatrix;
};

// <-

struct PerInstanceDataGridFragment
{
  glm::mat4 invWorldRotMatrix;
  glm::vec4 invWorldPos;
  glm::mat4 viewProjMatrix;
  glm::mat4 viewMatrix;

  glm::vec4 planeNormal;

  float gridSize;
  float fade;
};

// <-

DrawCallRef _drawCallRefGizmoTransl;
DrawCallRef _drawCallRefGizmoRotate;
DrawCallRef _drawCallRefGizmoScale;
DrawCallRef _drawCallRefGrid;

// <-

_INTR_INLINE bool isCurrentlySelectedEntityValid()
{
  return Editing::_currentlySelectedEntity.isValid() &&
         Components::NodeManager::getComponentForEntity(
             Editing::_currentlySelectedEntity)
             .isValid();
}

// <-

_INTR_INLINE void selectGizmoAxis(const glm::vec3& p_EntityWorldPos,
                                  const Math::Ray& p_WorldRay, bool& p_X,
                                  bool& p_Y, bool& p_Z)
{
  p_X = false;
  p_Y = false;
  p_Z = false;

  const float widthDepth = 0.025f * _lastGizmoScale;
  Math::AABB worldBoxX = {
      p_EntityWorldPos - glm::vec3(-0.5f, widthDepth, widthDepth),
      p_EntityWorldPos + glm::vec3(_lastGizmoScale, widthDepth, widthDepth)};
  Math::AABB worldBoxY = {
      p_EntityWorldPos - glm::vec3(widthDepth, -0.5f, widthDepth),
      p_EntityWorldPos + glm::vec3(widthDepth, _lastGizmoScale, widthDepth)};
  Math::AABB worldBoxZ = {
      p_EntityWorldPos - glm::vec3(widthDepth, widthDepth, -0.5f),
      p_EntityWorldPos + glm::vec3(widthDepth, widthDepth, _lastGizmoScale)};

  glm::vec3 xi0;
  glm::vec3 xi1;
  bool x = Math::calcIntersectRayAABB(p_WorldRay, worldBoxX, xi0, xi1);
  glm::vec3 yi0;
  glm::vec3 yi1;
  bool y = Math::calcIntersectRayAABB(p_WorldRay, worldBoxY, yi0, yi1);
  glm::vec3 zi0;
  glm::vec3 zi1;
  bool z = Math::calcIntersectRayAABB(p_WorldRay, worldBoxZ, zi0, zi1);

  float distX = FLT_MAX;
  if (x)
  {
    distX = glm::distance2(xi0, p_WorldRay.o);
  }

  float distY = FLT_MAX;
  if (y)
  {
    distY = glm::distance2(yi0, p_WorldRay.o);
  }

  float distZ = FLT_MAX;
  if (z)
  {
    distZ = glm::distance2(zi0, p_WorldRay.o);
  }

  if (distX < distY && distX < distZ)
  {
    p_X = true;
  }
  else if (distY < distX && distY < distZ)
  {
    p_Y = true;
  }
  else if (distZ < distX && distZ < distY)
  {
    p_Z = true;
  }
}

// <-

_INTR_INLINE void selectGizmoPlane(const glm::vec3& p_EntityWorldPos,
                                   const Math::Ray& p_WorldRay, bool& p_XZ,
                                   bool& p_XY, bool& p_YZ)
{
  p_XZ = false;
  p_XY = false;
  p_YZ = false;

  const float planeWidthHeight = _lastGizmoScale * 0.25f;

  glm::vec3 pXZ;
  bool iXZ = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(0.0f, 1.0f, 0.0f), p_EntityWorldPos, pXZ);
  glm::vec3 localXZ = pXZ - p_EntityWorldPos;
  iXZ = iXZ &&
        glm::all(glm::lessThanEqual(glm::vec2(localXZ.x, localXZ.z),
                                    glm::vec2(planeWidthHeight))) &&
        glm::all(
            greaterThanEqual(glm::vec2(localXZ.x, localXZ.z), glm::vec2(0.0f)));

  glm::vec3 pXY;
  bool iXY = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(0.0f, 0.0f, 1.0f), p_EntityWorldPos, pXY);
  glm::vec3 localXY = pXY - p_EntityWorldPos;
  iXY = iXY &&
        glm::all(glm::lessThanEqual(glm::vec2(localXY.x, localXY.y),
                                    glm::vec2(planeWidthHeight))) &&
        glm::all(
            greaterThanEqual(glm::vec2(localXY.x, localXY.y), glm::vec2(0.0f)));

  glm::vec3 pYZ;
  bool iYZ = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(1.0f, 0.0f, 0.0f), p_EntityWorldPos, pYZ);
  glm::vec3 localYZ = pYZ - p_EntityWorldPos;
  iYZ = iYZ &&
        glm::all(glm::lessThanEqual(glm::vec2(localYZ.y, localYZ.z),
                                    glm::vec2(planeWidthHeight))) &&
        glm::all(
            greaterThanEqual(glm::vec2(localYZ.y, localYZ.z), glm::vec2(0.0f)));

  float distXZ = FLT_MAX;
  if (iXZ)
  {
    distXZ = glm::distance2(pXZ, p_WorldRay.o);
  }

  float distXY = FLT_MAX;
  if (iXY)
  {
    distXY = glm::distance2(pXY, p_WorldRay.o);
  }

  float distYZ = FLT_MAX;
  if (iYZ)
  {
    distYZ = glm::distance2(pYZ, p_WorldRay.o);
  }

  if (distXZ < distXY && distXZ < distYZ)
  {
    p_XZ = true;
  }
  else if (distXY < distXZ && distXY < distYZ)
  {
    p_XY = true;
  }
  else if (distYZ < distXZ && distYZ < distXY)
  {
    p_YZ = true;
  }
}

// <-

_INTR_INLINE void selectGizmoRotationAxis(const glm::vec3& p_EntityWorldPos,
                                          const Math::Ray& p_WorldRay,
                                          bool& p_X, bool& p_Y, bool& p_Z)
{
  p_X = false;
  p_Y = false;
  p_Z = false;

  const float innerRadius = _lastGizmoScale * 0.95f;
  const float outerRadius = _lastGizmoScale * 1.05f;

  glm::vec3 pXZ;
  bool iXZ = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(0.0f, 1.0f, 0.0f), p_EntityWorldPos, pXZ);
  glm::vec3 localXZ = pXZ - p_EntityWorldPos;
  const float distXZ = glm::length(localXZ);
  iXZ = iXZ && distXZ > innerRadius && distXZ < outerRadius;

  glm::vec3 pXY;
  bool iXY = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(0.0f, 0.0f, 1.0f), p_EntityWorldPos, pXY);
  glm::vec3 localXY = pXY - p_EntityWorldPos;
  const float distXY = glm::length(localXY);
  iXY = iXY && distXY > innerRadius && distXY < outerRadius;

  glm::vec3 pYZ;
  bool iYZ = Math::calcIntersectRayPlane(
      p_WorldRay, glm::vec3(1.0f, 0.0f, 0.0f), p_EntityWorldPos, pYZ);
  glm::vec3 localYZ = pYZ - p_EntityWorldPos;
  const float distYZ = glm::length(localYZ);
  iYZ = iYZ && distYZ > innerRadius && distYZ < outerRadius;

  float oDistXZ = FLT_MAX;
  if (iXZ)
  {
    oDistXZ = glm::distance2(pXZ, p_WorldRay.o);
  }

  float oDistXY = FLT_MAX;
  if (iXY)
  {
    oDistXY = glm::distance2(pXY, p_WorldRay.o);
  }

  float oDistYZ = FLT_MAX;
  if (iYZ)
  {
    oDistYZ = glm::distance2(pYZ, p_WorldRay.o);
  }

  if (oDistXZ < oDistXY && oDistXZ < oDistYZ)
  {
    p_Y = true;
  }
  else if (oDistXY < oDistXZ && oDistXY < oDistYZ)
  {
    p_Z = true;
  }
  else if (oDistYZ < oDistXZ && oDistYZ < oDistXY)
  {
    p_X = true;
  }
}

// <-

_INTR_INLINE glm::vec3 getAxisVector(bool p_X, bool p_Y, bool p_Z)
{
  if (p_X)
  {
    _INTR_ASSERT(!p_Y && !p_Z);
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  else if (p_Y)
  {
    _INTR_ASSERT(!p_X && !p_Z);
    return glm::vec3(0.0f, 1.0f, 0.0f);
  }

  _INTR_ASSERT(!p_X && !p_Y);
  return glm::vec3(0.0f, 0.0f, 1.0f);
}

// <-

_INTR_INLINE glm::vec3 findBestPlaneNormal(const glm::vec3& p_Axis,
                                           const glm::vec3& p_DirectionToCam)
{
  float maxDot = -FLT_MAX;
  uint32_t planeIdx = (uint32_t)-1;

  for (uint32_t i = 0u; i < 6u; ++i)
  {
    if (glm::abs(glm::dot(_planeVectors[i], p_Axis)) > 0.0f)
    {
      continue;
    }

    const float dot = glm::dot(_planeVectors[i], p_DirectionToCam);
    if (dot > maxDot)
    {
      maxDot = dot;
      planeIdx = i;
    }
  }

  return _planeVectors[planeIdx];
}

// <-

_INTR_INLINE glm::vec3 snapToGrid(const glm::vec3& p_Position, float p_GridSize)
{
  return glm::trunc(p_Position / p_GridSize) * p_GridSize;
}

// <-

_INTR_INLINE void updateCameraOrbit(float p_DeltaT)
{
  Components::CameraRef camRef = World::_activeCamera;
  Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(camRef));

  static const float damping = 0.005f;

  glm::quat camRot = glm::quat(_eulerAngles);
  glm::vec3 forward = camRot * glm::vec3(0.0f, 0.0f, 1.0f);

  // Initializes orbit camera mode
  if (_cameraMode != CameraMode::kOrbit)
  {
    _camAngVel = glm::vec3(0.0f);
    _camVel = glm::vec3(0.0f);
    _orbitCamCenter = Components::NodeManager::_worldPosition(camNodeRef) +
                      _orbitRadius * forward;

    _cameraMode = CameraMode::kOrbit;
  }

  if (Input::System::getKeyStates()[Input::Key::kMouseRight] ==
      Input::KeyState::kPressed)
  {
    if (!_rightMouseButtonPressed)
    {
      _rightMouseButtonPressed = true;
    }
    else
    {
      const glm::vec2& mouseVelocity =
          _mouseSens * Input::System::getLastMousePosRel();
      _camAngVel += glm::vec3(mouseVelocity.x, mouseVelocity.y, 0.0f);
    }
  }
  else
  {
    _rightMouseButtonPressed = false;
  }

  if (Input::System::getKeyStates()[Input::Key::kAlt] ==
          Input::KeyState::kPressed &&
      Input::System::getKeyStates()[Input::Key::kCtrl] ==
          Input::KeyState::kPressed)
  {
    glm::vec3 up = camRot * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(forward, up);

    _orbitCamCenter += Editing::_cameraSpeed * _camAngVel.x * p_DeltaT * right;
    _orbitCamCenter += Editing::_cameraSpeed * -_camAngVel.y * p_DeltaT * up;
  }
  else if (Input::System::getKeyStates()[Input::Key::kAlt] ==
           Input::KeyState::kPressed)
  {
    _eulerAngles.x += _camAngVel.y * p_DeltaT;
    _eulerAngles.y += -_camAngVel.x * p_DeltaT;
  }
  else if (Input::System::getKeyStates()[Input::Key::kCtrl] ==
           Input::KeyState::kPressed)
  {
    _orbitRadius =
        glm::max(_orbitRadius - Editing::_cameraSpeed *
                                    (_camAngVel.x - _camAngVel.y) * p_DeltaT,
                 0.1f);
  }

  camRot = glm::quat(_eulerAngles);
  Components::NodeManager::_orientation(camNodeRef) = _eulerAngles;
  Components::NodeManager::_position(camNodeRef) =
      _orbitCamCenter + camRot * glm::vec3(0.0f, 0.0f, -_orbitRadius);
  Components::NodeManager::updateTransforms(camNodeRef);

  Math::dampSimple(_camAngVel, damping, p_DeltaT);
}

// <-

_INTR_INLINE void updateCameraFreeFlight(float p_DeltaT)
{
  // Initializes free flight mode
  if (_cameraMode != CameraMode::kFreeFlight)
  {
    _camAngVel = glm::vec3(0.0f);
    _camVel = glm::vec3(0.0f);

    _cameraMode = CameraMode::kFreeFlight;
  }

  Components::CameraRef camRef = World::_activeCamera;
  Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(camRef));

  static const float damping = 0.005f;

  if (Input::System::getKeyStates()[Input::Key::kMouseRight] ==
      Input::KeyState::kPressed)
  {
    if (!_rightMouseButtonPressed)
    {
      _rightMouseButtonPressed = true;
    }
    else
    {
      const glm::vec2& mouseVelocity =
          _mouseSens * Input::System::getLastMousePosRel();
      _camAngVel += glm::vec3(mouseVelocity.y, -mouseVelocity.x, 0.0f);
    }
  }
  else
  {
    _rightMouseButtonPressed = false;
  }

  // Controller handling
  const glm::vec4 movement = Input::System::getMovementFiltered();
  _camAngVel.y += -_controllerSens * movement.w * p_DeltaT;
  _camAngVel.x += _controllerSens * movement.z * p_DeltaT;

  _eulerAngles += _camAngVel * p_DeltaT;
  glm::quat camRot = glm::quat(_eulerAngles);

  glm::vec3 forward = camRot * glm::vec3(0.0f, 0.0f, 1.0f);
  glm::vec3 up = camRot * glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 right = glm::cross(forward, up);

  float actualMoveSpeed = Editing::_cameraSpeed;
  if (Input::System::getKeyStates()[Input::Key::kShift] ==
      Input::KeyState::kPressed)
  {
    actualMoveSpeed *= 4.0f;
  }
  if (Input::System::getKeyStates()[Input::Key::kW] ==
      Input::KeyState::kPressed)
  {
    _camVel += forward * p_DeltaT * actualMoveSpeed;
  }
  if (Input::System::getKeyStates()[Input::Key::kA] ==
      Input::KeyState::kPressed)
  {
    _camVel += -right * p_DeltaT * actualMoveSpeed;
  }
  if (Input::System::getKeyStates()[Input::Key::kS] ==
      Input::KeyState::kPressed)
  {
    _camVel += -forward * p_DeltaT * actualMoveSpeed;
  }
  if (Input::System::getKeyStates()[Input::Key::kD] ==
      Input::KeyState::kPressed)
  {
    _camVel += right * p_DeltaT * actualMoveSpeed;
  }

  // Controller handling
  _camVel += right * actualMoveSpeed * movement.y * p_DeltaT +
             forward * -actualMoveSpeed * movement.x * p_DeltaT;

  Components::NodeManager::_orientation(camNodeRef) = _eulerAngles;
  Components::NodeManager::_position(camNodeRef) += _camVel * p_DeltaT;
  Components::NodeManager::updateTransforms(camNodeRef);

  Math::dampSimple(_camVel, damping, p_DeltaT);
  Math::dampSimple(_camAngVel, damping, p_DeltaT);
}

// <-

_INTR_INLINE void handleGizmo(float p_DeltaT)
{
  if (Input::System::getKeyStates()[Input::Key::kMouseLeft] ==
      Input::KeyState::kPressed)
  {
    Components::CameraRef camRef = World::_activeCamera;
    Components::NodeRef camNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(camRef));
    const glm::vec3& camPosition =
        Components::NodeManager::_worldPosition(camNodeRef);

    Components::NodeRef currentEntityNodeRef =
        Components::NodeManager::getComponentForEntity(
            Editing::_currentlySelectedEntity);
    const glm::vec3& entityWorldPos =
        Components::NodeManager::_worldPosition(currentEntityNodeRef);
    const glm::vec3 dirToCam = glm::normalize(camPosition - entityWorldPos);

    const Math::Ray worldRay = Math::calcMouseRay(
        camPosition, Input::System::getLastMousePosViewport(),
        Components::CameraManager::_inverseViewProjectionMatrix(camRef));

    if (Editing::_editingMode == EditingMode::kTranslation ||
        Editing::_editingMode == EditingMode::kScale ||
        Editing::_editingMode == EditingMode::kRotation)
    {
      // Initial click handling - select gizmo axis and calc. initial offsets
      if (!_leftMouseButtonPressed)
      {
        if (Editing::_editingMode == EditingMode::kTranslation ||
            Editing::_editingMode == EditingMode::kScale)
        {
          selectGizmoAxis(entityWorldPos, worldRay, _XAxisSelected,
                          _YAxisSelected, _ZAxisSelected);
          _anyAxisSelected = _XAxisSelected || _YAxisSelected || _ZAxisSelected;

          if (!_anyAxisSelected)
          {
            if (Editing::_editingMode == EditingMode::kTranslation)
            {
              selectGizmoPlane(entityWorldPos, worldRay, _XZPlaneSelected,
                               _XYPlaneSelected, _YZPlaneSelected);
              _anyPlaneSelected =
                  _XZPlaneSelected || _XYPlaneSelected || _YZPlaneSelected;
            }

            if (!_anyPlaneSelected)
              return;
          }

          if (_anyAxisSelected)
          {
            const glm::vec3 axisVector =
                getAxisVector(_XAxisSelected, _YAxisSelected, _ZAxisSelected);
            _selectedPlaneNormal = findBestPlaneNormal(axisVector, dirToCam);
          }
          else if (_anyPlaneSelected)
          {
            if (_XYPlaneSelected)
            {
              _selectedPlaneNormal = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            else if (_XZPlaneSelected)
            {
              _selectedPlaneNormal = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            else if (_YZPlaneSelected)
            {
              _selectedPlaneNormal = glm::vec3(1.0f, 0.0f, 0.0f);
            }

            _selectedPlaneNormal *=
                glm::dot(_selectedPlaneNormal, dirToCam) > 0.0f ? 1.0f : -1.0f;
          }
        }
        else
        {
          selectGizmoRotationAxis(entityWorldPos, worldRay, _XAxisSelected,
                                  _YAxisSelected, _ZAxisSelected);
          _anyAxisSelected = _XAxisSelected || _YAxisSelected || _ZAxisSelected;

          if (!_anyAxisSelected)
            return;

          _selectedPlaneNormal = glm::vec3(_XAxisSelected ? 1.0f : 0.0f,
                                           _YAxisSelected ? 1.0f : 0.0f,
                                           _ZAxisSelected ? 1.0f : 0.0f);

          _selectedPlaneNormal *=
              glm::dot(_selectedPlaneNormal, dirToCam) > 0.0f ? 1.0f : -1.0f;
        }

        glm::vec3 planeIntersectionPoint;
        bool intersects =
            Math::calcIntersectRayPlane(worldRay, _selectedPlaneNormal,
                                        entityWorldPos, planeIntersectionPoint);
        _INTR_ASSERT(intersects);

        glm::vec3 planeIntersectionPointRot;
        intersects = Math::calcIntersectRayPlane(
            worldRay, -Components::CameraManager::_forward(camRef),
            entityWorldPos, planeIntersectionPointRot);
        _INTR_ASSERT(intersects);

        _initialRotationDir = planeIntersectionPointRot - entityWorldPos;

        if (glm::length2(_initialRotationDir) > _INTR_EPSILON)
        {
          _initialRotationDir = glm::normalize(_initialRotationDir);
        }
        else
        {
          _initialRotationDir = glm::vec3(0.0f);
        }

        _initialPosOffset = planeIntersectionPoint - entityWorldPos;
        _initialOrientation =
            Components::NodeManager::_orientation(currentEntityNodeRef);
        _initialScale = Components::NodeManager::_size(currentEntityNodeRef);
        _leftMouseButtonPressed = true;
      }

      if (Editing::_editingMode == EditingMode::kRotation)
      {
        float planeRotation = 0.0f;

        const glm::vec3 rayDir = glm::normalize(entityWorldPos - camPosition);
        const glm::vec3 pointOnHelperPlane = camPosition + rayDir * 2.0f;

        glm::vec3 planeIntersectionPoint;
        if (Math::calcIntersectRayPlane(
                worldRay, -Components::CameraManager::_forward(camRef),
                pointOnHelperPlane, planeIntersectionPoint))
        {
          glm::vec3 dir = planeIntersectionPoint - pointOnHelperPlane;

          if (glm::length2(dir) > _INTR_EPSILON &&
              glm::length2(_initialRotationDir) > _INTR_EPSILON)
          {
            dir = glm::normalize(dir);

            planeRotation = std::acos(
                glm::clamp(glm::dot(dir, _initialRotationDir), -1.0f, 1.0f));

            const glm::vec3 v0 = glm::cross(dir, _initialRotationDir);
            const float sign =
                glm::dot(v0, Components::CameraManager::_forward(camRef));
            planeRotation *= sign >= 0.0f ? 1.0f : -1.0f;

            // Visualize helper line
            R::RenderPass::Debug::renderLineDotted(pointOnHelperPlane,
                                                   planeIntersectionPoint,
                                                   glm::vec3(0.5f, 0.5f, 0.5f));
          }
        }

        // Quant. rotation
        const float rotationQuantStep =
            1.0f / glm::radians(Editing::_rotationStepSize);

        planeRotation =
            glm::trunc(planeRotation * rotationQuantStep) / rotationQuantStep;
        const glm::vec3 newRotation = planeRotation * _selectedPlaneNormal;
        Components::NodeManager::_orientation(currentEntityNodeRef) =
            glm::quat(newRotation) * _initialOrientation;
      }
      else
      {
        // Calc. intersection between ray and selected axis plane
        glm::vec3 planeIntersectionPoint;
        if (Math::calcIntersectRayPlane(worldRay, _selectedPlaneNormal,
                                        entityWorldPos, planeIntersectionPoint))
        {
          Components::NodeRef parentNode =
              Components::NodeManager::_parent(currentEntityNodeRef);
          glm::vec3 newWorldPos =
              Components::NodeManager::_worldPosition(currentEntityNodeRef);
          glm::vec3 newSize = _initialScale;

          // Screen space scaling factor
          const glm::vec3 posOffset =
              planeIntersectionPoint - _initialPosOffset;
          const glm::vec3 posOffsetLocal = posOffset - newWorldPos;

          if (_XAxisSelected)
          {
            newSize.x += posOffsetLocal.x;
            newWorldPos.x = posOffset.x;
          }
          else if (_YAxisSelected)
          {
            newSize.y += posOffsetLocal.y;
            newWorldPos.y = posOffset.y;
          }
          else if (_ZAxisSelected)
          {
            newSize.z += posOffsetLocal.z;
            newWorldPos.z = posOffset.z;
          }
          else if (_anyPlaneSelected)
          {
            newWorldPos = planeIntersectionPoint - _initialPosOffset;
          }

          if (Editing::_editingMode == EditingMode::kTranslation)
          {
            newWorldPos = snapToGrid(newWorldPos, Editing::_gridSize);
            Components::NodeManager::updateFromWorldPosition(
                currentEntityNodeRef, newWorldPos);
          }
          else if (Editing::_editingMode == EditingMode::kScale)
          {
            newSize = snapToGrid(newSize, Editing::_gridSize);
            Components::NodeManager::_size(currentEntityNodeRef) = newSize;
          }
        }
      }
    }

    Components::NodeManager::updateTransforms(currentEntityNodeRef);
    _gridPosition =
        Components::NodeManager::_worldPosition(currentEntityNodeRef);
  }
  else
  {
    _XAxisSelected = false;
    _XZPlaneSelected = false;

    _YAxisSelected = false;
    _XYPlaneSelected = false;

    _ZAxisSelected = false;
    _YZPlaneSelected = false;

    _anyAxisSelected = false;
    _anyPlaneSelected = false;

    _leftMouseButtonPressed = false;
  }
}
}

// Static members
Entity::EntityRef Editing::_currentlySelectedEntity;
EditingMode::Enum Editing::_editingMode = EditingMode::kDefault;
float Editing::_gridSize = 1.0f;
float Editing::_gizmoSize = 0.1f;
float Editing::_cameraSpeed = 150.0f;
float Editing::_rotationStepSize = 5.0f;

// <-

void Editing::init() { onReinitRendering(); }

void Editing::onReinitRendering()
{
  Resources::MeshRef meshRefGizmoTransl =
      Resources::MeshManager::getResourceByName(_N(gizmo_translate));
  Resources::MeshRef meshRefGizmoRotate =
      Resources::MeshManager::getResourceByName(_N(gizmo_rotate));
  Resources::MeshRef meshRefGizmoScale =
      Resources::MeshManager::getResourceByName(_N(gizmo_scale));
  Resources::MeshRef meshRefGrid =
      Resources::MeshManager::getResourceByName(_N(plane));

  DrawCallRefArray drawCallsToDestroy;
  if (_drawCallRefGizmoTransl.isValid())
    drawCallsToDestroy.push_back(_drawCallRefGizmoTransl);
  if (_drawCallRefGizmoRotate.isValid())
    drawCallsToDestroy.push_back(_drawCallRefGizmoRotate);
  if (_drawCallRefGizmoScale.isValid())
    drawCallsToDestroy.push_back(_drawCallRefGizmoScale);
  if (_drawCallRefGrid.isValid())
    drawCallsToDestroy.push_back(_drawCallRefGrid);

  DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);

  // Draw calls
  DrawCallRefArray drawCallsToCreate;
  {
    _drawCallRefGizmoTransl = DrawCallManager::createDrawCallForMesh(
        _N(gizmo_translate), meshRefGizmoTransl,
        MaterialManager::getResourceByName(_N(gizmo)),
        MaterialManager::getMaterialPassId(_N(DebugGizmo)),
        sizeof(PerInstanceDataGizmoVertex),
        sizeof(PerInstanceDataGizmoFragment));
    drawCallsToCreate.push_back(_drawCallRefGizmoTransl);

    _drawCallRefGizmoRotate = DrawCallManager::createDrawCallForMesh(
        _N(gizmo_rotate), meshRefGizmoRotate,
        MaterialManager::getResourceByName(_N(gizmo)),
        MaterialManager::getMaterialPassId(_N(DebugGizmo)),
        sizeof(PerInstanceDataGizmoVertex),
        sizeof(PerInstanceDataGizmoFragment));
    drawCallsToCreate.push_back(_drawCallRefGizmoRotate);

    _drawCallRefGizmoScale = DrawCallManager::createDrawCallForMesh(
        _N(gizmo_scale), meshRefGizmoScale,
        MaterialManager::getResourceByName(_N(gizmo)),
        MaterialManager::getMaterialPassId(_N(DebugGizmo)),
        sizeof(PerInstanceDataGizmoVertex),
        sizeof(PerInstanceDataGizmoFragment));
    drawCallsToCreate.push_back(_drawCallRefGizmoScale);

    _drawCallRefGrid = DrawCallManager::createDrawCallForMesh(
        _N(Grid), meshRefGrid, MaterialManager::getResourceByName(_N(grid)),
        MaterialManager::getMaterialPassId(_N(DebugGrid)),
        sizeof(PerInstanceDataGridVertex), sizeof(PerInstanceDataGridFragment));
    drawCallsToCreate.push_back(_drawCallRefGrid);
  }

  DrawCallManager::createResources(drawCallsToCreate);
}

// <-

void Editing::activate()
{
  Entity::EntityRef entityRef =
      Entity::EntityManager::getEntityByName(_N(MainCamera));
  _INTR_ASSERT(entityRef.isValid());
  Components::CameraRef cameraRef =
      Components::CameraManager::getComponentForEntity(entityRef);
  _INTR_ASSERT(cameraRef.isValid());

  World::setActiveCamera(cameraRef);

  const Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(cameraRef));
  _eulerAngles =
      glm::eulerAngles(Components::NodeManager::_orientation(camNodeRef));

  // Remove roll if it ever gets into play - possible when switching from euler
  // to quat. and the other way around
  if (glm::abs(_eulerAngles.z) > glm::pi<float>() - _INTR_EPSILON)
  {
    _eulerAngles.x -= glm::pi<float>();
    _eulerAngles.y =
        glm::half_pi<float>() - (_eulerAngles.y - glm::half_pi<float>());
    _eulerAngles.z = 0.0f;
  }
}

// <-

void Editing::deativate()
{
  if (_pickingJoint)
  {
    _pickingJoint->release();
    _pickingJoint = nullptr;
  }
  if (_pickingDummyActor)
  {
    _pickingDummyActor->release();
    _pickingDummyActor = nullptr;
  }
}

// <-

void Editing::updatePerInstanceData()
{
  Components::CameraRef camRef = World::_activeCamera;
  _INTR_ASSERT(camRef.isValid());

  // Gizmo
  if (isCurrentlySelectedEntityValid())
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getComponentForEntity(
            _currentlySelectedEntity);
    glm::mat4 trans = glm::translate(
        glm::mat4(1.0f), Components::NodeManager::_worldPosition(nodeRef));

    _lastGizmoScale = Math::calcScreenSpaceScale(
        Components::NodeManager::_worldPosition(nodeRef),
        Components::CameraManager::_viewProjectionMatrix(camRef), _gizmoSize);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(_lastGizmoScale));

    PerInstanceDataGizmoVertex perInstanceDataVertex;
    {
      perInstanceDataVertex.worldMatrix = trans * scale;
      perInstanceDataVertex.worldViewProjMatrix =
          Components::CameraManager::_viewProjectionMatrix(camRef) *
          perInstanceDataVertex.worldMatrix;
      perInstanceDataVertex.viewMatrix =
          Components::CameraManager::_viewMatrix(camRef);

      const Components::NodeRef camNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::CameraManager::_entity(camRef));
      const Math::Ray worldRay = Math::calcMouseRay(
          Components::NodeManager::_worldPosition(camNodeRef),
          Input::System::getLastMousePosViewport(),
          Components::CameraManager::_inverseViewProjectionMatrix(camRef));

      // Highlight gizmo axis
      bool x = _XAxisSelected, y = _YAxisSelected, z = _ZAxisSelected;

      if (!_anyAxisSelected)
      {
        if (_editingMode == EditingMode::kTranslation ||
            _editingMode == EditingMode::kScale)
        {
          selectGizmoAxis(Components::NodeManager::_worldPosition(nodeRef),
                          worldRay, x, y, z);
        }
        else if (_editingMode == EditingMode::kRotation)
        {
          selectGizmoRotationAxis(
              Components::NodeManager::_worldPosition(nodeRef), worldRay, x, y,
              z);
        }
      }

      perInstanceDataVertex.colorTintX = x ? glm::vec4(1.0f) : glm::vec4(0.5f);
      perInstanceDataVertex.colorTintY = y ? glm::vec4(1.0f) : glm::vec4(0.5f);
      perInstanceDataVertex.colorTintZ = z ? glm::vec4(1.0f) : glm::vec4(0.5f);
    }

    PerInstanceDataGizmoFragment perInstanceDataFragment;
    {
      perInstanceDataFragment._dummy = 42.0f;
    }

    // Write to GPU memory
    DrawCallRefArray dcsToUpdate = {_drawCallRefGizmoTransl,
                                    _drawCallRefGizmoRotate,
                                    _drawCallRefGizmoScale};
    DrawCallManager::allocateAndUpdateUniformMemory(
        dcsToUpdate, &perInstanceDataVertex, sizeof(PerInstanceDataGizmoVertex),
        &perInstanceDataFragment, sizeof(PerInstanceDataGizmoFragment));
  }

  // Axis plane
  if (_gridFade > _INTR_EPSILON)
  {
    PerInstanceDataGridVertex perInstanceDataVertex;
    PerInstanceDataGridFragment perInstanceDataFragment;

    glm::mat4 rot = glm::mat4_cast(
        glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), _selectedPlaneNormal));
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), _gridPosition);
    glm::mat4 scale =
        glm::scale(glm::mat4(1.0f), glm::vec3(_gridSize, 1.0f, _gridSize));

    {
      perInstanceDataVertex.worldMatrix = trans * rot * scale;
      perInstanceDataVertex.worldViewProjMatrix =
          Components::CameraManager::_viewProjectionMatrix(camRef) *
          perInstanceDataVertex.worldMatrix;
    }

    {
      perInstanceDataFragment.invWorldRotMatrix = glm::inverse(rot);
      perInstanceDataFragment.invWorldPos = glm::vec4(-_gridPosition, 1.0f);
      perInstanceDataFragment.planeNormal =
          glm::vec4(_selectedPlaneNormal, 0.0f);
      perInstanceDataFragment.viewProjMatrix =
          Components::CameraManager::_viewProjectionMatrix(camRef);
      perInstanceDataFragment.viewMatrix =
          Components::CameraManager::_viewMatrix(camRef);
      perInstanceDataFragment.gridSize = _gridSize;
      perInstanceDataFragment.fade = _gridFade;
    }

    // Write to GPU memory
    DrawCallRefArray dcsToUpdate = {_drawCallRefGrid};
    DrawCallManager::allocateAndUpdateUniformMemory(
        dcsToUpdate, &perInstanceDataVertex, sizeof(PerInstanceDataGridVertex),
        &perInstanceDataFragment, sizeof(PerInstanceDataGridFragment));
  }
}

// <-

void Editing::findVisibleEditingDrawCalls(Dod::RefArray& p_DrawCalls)
{
  if (isCurrentlySelectedEntityValid())
  {
    if (_editingMode == EditingMode::kTranslation)
    {
      p_DrawCalls.push_back(_drawCallRefGizmoTransl);
    }
    else if (_editingMode == EditingMode::kRotation)
    {
      p_DrawCalls.push_back(_drawCallRefGizmoRotate);
    }
    else if (_editingMode == EditingMode::kScale)
    {
      p_DrawCalls.push_back(_drawCallRefGizmoScale);
    }
  }

  if (_gridFade > _INTR_EPSILON)
  {
    p_DrawCalls.push_back(_drawCallRefGrid);
  }
}

// <-

void Editing::update(float p_DeltaT)
{
  _INTR_PROFILE_CPU("Game States", "Editing");

  // Avoid time mod.
  const float deltaT = TaskManager::_lastDeltaT;

  // Fade grid in/out
  static const float fadeDurationInSeconds = 1.0f;
  if (_anyAxisSelected || _anyPlaneSelected)
  {
    if (_gridFade < 1.0f)
    {
      _gridFade += deltaT * fadeDurationInSeconds;
      _gridFade = glm::min(_gridFade, 1.0f);
    }
  }
  else
  {
    if (_gridFade > 0.0f)
    {
      _gridFade -= deltaT * fadeDurationInSeconds;
      _gridFade = glm::max(_gridFade, 0.0f);
    }
  }

  if (isCurrentlySelectedEntityValid())
  {
    if (_editingMode == EditingMode::kTranslation)
    {
      handleGizmo(deltaT);

      // Clone node
      if (Input::System::getKeyStates()[Input::Key::kAlt] ==
              Input::KeyState::kPressed &&
          Input::System::getKeyStates()[Input::Key::kMouseLeft] ==
              Input::KeyState::kPressed &&
          !_cloningInProgress)
      {
        _currentlySelectedEntity =
            World::cloneNodeFull(Components::NodeManager::getComponentForEntity(
                _currentlySelectedEntity));
        Resources::EventManager::queueEventIfNotExisting(
            _N(CurrentlySelectedEntityChanged));

        _cloningInProgress = true;
      }
      else if (Input::System::getKeyStates()[Input::Key::kMouseLeft] ==
               Input::KeyState::kReleased)
      {
        _cloningInProgress = false;
      }
    }
    else if (_editingMode == EditingMode::kRotation)
    {
      handleGizmo(deltaT);
    }
    else if (_editingMode == EditingMode::kScale)
    {
      handleGizmo(deltaT);
    }
  }

  if (_editingMode == EditingMode::kDefault)
  {
    // Physics picking
    if (Input::System::getKeyStates()[Input::Key::kMouseLeft] ==
        Input::KeyState::kPressed)
    {
      Components::CameraRef camRef = World::_activeCamera;
      Components::NodeRef camNodeRef =
          Components::NodeManager::getComponentForEntity(
              Components::CameraManager::_entity(camRef));
      const glm::vec3& camPosition =
          Components::NodeManager::_worldPosition(camNodeRef);

      const Math::Ray worldRay = Math::calcMouseRay(
          camPosition, Input::System::getLastMousePosViewport(),
          Components::CameraManager::_inverseViewProjectionMatrix(camRef));

      if (_pickingJoint == nullptr && _pickingDummyActor == nullptr)
      {
        // Init physics dummy picking actor
        _pickingDummyActor = Physics::System::_pxPhysics->createRigidDynamic(
            physx::PxTransform(physx::PxIdentity));
        _INTR_ASSERT(_pickingDummyActor);

        _pickingDummyActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC,
                                             true);

        physx::PxSphereGeometry sphereGeometry;
        sphereGeometry.radius = 0.1f;

        physx::PxShape* shape = _pickingDummyActor->createShape(
            sphereGeometry, *Components::RigidBodyManager::_defaultMaterial);
        _INTR_ASSERT(shape);

        physx::PxRigidBodyExt::updateMassAndInertia(*_pickingDummyActor, 1.0f);
        Physics::System::_pxScene->addActor(*_pickingDummyActor);

        physx::PxRaycastHit hit;
        if (PhysicsHelper::raycast(worldRay, hit, 1000.0f,
                                   physx::PxQueryFlag::eDYNAMIC))
        {
          _pickingInitialDistance = hit.distance;

          _pickingDummyActor->setGlobalPose(
              physx::PxTransform(PhysicsHelper::convert(
                  worldRay.o + worldRay.d * _pickingInitialDistance)));

          _pickingJoint = physx::PxDistanceJointCreate(
              *Physics::System::_pxPhysics, hit.actor,
              physx::PxTransform(
                  hit.actor->getGlobalPose().transformInv(hit.position)),
              _pickingDummyActor,
              physx::PxTransform(
                  _pickingDummyActor->getGlobalPose().transformInv(
                      hit.position)));
        }
      }
      else
      {
        _pickingDummyActor->setKinematicTarget(
            physx::PxTransform(PhysicsHelper::convert(
                worldRay.o + worldRay.d * _pickingInitialDistance)));
      }
    }
    else
    {
      // Release joint and helper actor
      if (_pickingJoint)
      {
        _pickingJoint->release();
        _pickingJoint = nullptr;
      }
      if (_pickingDummyActor)
      {
        _pickingDummyActor->release();
        _pickingDummyActor = nullptr;
      }
    }
  }
  else if (_editingMode == EditingMode::kSelection)
  {
    // Pick nodes
    if (Input::System::getKeyStates()[Input::Key::kMouseLeft] ==
        Input::KeyState::kPressed)
    {
      Components::NodeRef pickedNode = R::RenderPass::PerPixelPicking::pickNode(
          Input::System::getLastMousePosViewport());

      if (pickedNode.isValid())
      {
        Entity::EntityRef entityToSelect =
            Components::NodeManager::_entity(pickedNode);

        if ((Components::NodeManager::_flags(pickedNode) &
             Components::NodeFlags::Flags::kSpawned) == 0u &&
            entityToSelect != _currentlySelectedEntity)
        {
          _currentlySelectedEntity = entityToSelect;
          Resources::EventManager::queueEventIfNotExisting(
              _N(CurrentlySelectedEntityChanged));
        }
      }
    }
  }

  // Focus on selected object
  if (Input::System::getKeyStates()[Input::Key::kF] ==
          Input::KeyState::kPressed &&
      _currentlySelectedEntity.isValid())
  {
    Components::NodeRef camNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(World::_activeCamera));
    Components::NodeRef entityNodeRef =
        Components::NodeManager::getComponentForEntity(
            _currentlySelectedEntity);

    const glm::vec3 halfExtents = Math::calcAABBHalfExtent(
        Components::NodeManager::_worldAABB(entityNodeRef));
    const float camOffset = glm::length(halfExtents) * 2.0f;

    const glm::vec3 newPosition =
        Math::calcAABBCenter(
            Components::NodeManager::_worldAABB(entityNodeRef)) -
        camOffset * Components::CameraManager::_forward(World::_activeCamera);

    const float blendFactor = deltaT * 2.0f;
    Components::NodeManager::_position(camNodeRef) =
        (1.0f - blendFactor) * Components::NodeManager::_position(camNodeRef) +
        blendFactor * newPosition;
  }

  // Snap/align currently selected object to the ground
  if (Input::System::getKeyStates()[Input::Key::kT] ==
          Input::KeyState::kPressed &&
      _currentlySelectedEntity.isValid())
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getComponentForEntity(
            _currentlySelectedEntity);
    World::alignNodeWithGround(nodeRef);
  }
  else if ((Input::System::getKeyStates()[Input::Key::kDel] ==
                Input::KeyState::kPressed ||
            Input::System::getKeyStates()[Input::Key::kBackspace] ==
                Input::KeyState::kPressed) &&
           _currentlySelectedEntity.isValid())
  {
    Components::NodeRef currSelNodeRef =
        Components::NodeManager::getComponentForEntity(
            _currentlySelectedEntity);

    if (currSelNodeRef != World::_rootNode)
    {
      World::destroyNodeFull(currSelNodeRef);
      _currentlySelectedEntity =
          Components::NodeManager::_entity(World::_rootNode);
      Resources::EventManager::queueEventIfNotExisting(
          _N(CurrentlySelectedEntityChanged));
    }
  }

  if (Input::System::getKeyStates()[Input::Key::kAlt] ==
          Input::KeyState::kPressed ||
      Input::System::getKeyStates()[Input::Key::kCtrl] ==
          Input::KeyState::kPressed)
  {
    updateCameraOrbit(deltaT);
  }
  else
  {
    updateCameraFreeFlight(deltaT);
  }
}
}
}
}
