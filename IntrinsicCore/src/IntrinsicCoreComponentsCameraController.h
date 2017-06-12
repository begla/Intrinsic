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

/** \file
 * Contains the Camera Controller Component Manager.
 */

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref CameraControllerRef;
typedef _INTR_ARRAY(CameraControllerRef) CameraControllerRefArray;

namespace CameraControllerType
{

///
/// Enum defining the different camera controller types.
///
enum Enum
{
  /** A third person camera controller. */
  kThirdPerson,
  /** A first person camera controller. */
  kFirstPerson
};
}

///
/// Stores all the relevant data for the Camera Controller Component in a data
/// oriented fashion.
///
struct CameraControllerData : Dod::Components::ComponentDataBase
{
  CameraControllerData()
      : Dod::Components::ComponentDataBase(
            _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT)
  {
    descTargetObjectName.resize(_INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT);
    descCameraControllerType.resize(
        _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT);
    descTargetEulerAngles.resize(_INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT);

    lastTargetEulerAngles.resize(_INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT);
    timeSinceLastOrientationChange.resize(
        _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT);
  }

  _INTR_ARRAY(Name) descTargetObjectName;
  _INTR_ARRAY(CameraControllerType::Enum) descCameraControllerType;
  _INTR_ARRAY(glm::vec3) descTargetEulerAngles;

  _INTR_ARRAY(glm::vec3) lastTargetEulerAngles;
  _INTR_ARRAY(float) timeSinceLastOrientationChange;
};

///
/// The manager for all Camera Controller Components.
///
struct CameraControllerManager
    : Dod::Components::ComponentManagerBase<
          CameraControllerData, _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT>
{
  ///
  /// Init. the Camera Controller Manager.
  ///
  static void init();

  // <-

  ///
  /// Requests a new reference for a Camera Controller Component.
  ///
  _INTR_INLINE static CameraControllerRef
  createCameraController(Entity::EntityRef p_ParentEntity)
  {
    CameraControllerRef ref = Dod::Components::ComponentManagerBase<
        CameraControllerData, _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT>::
        _createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  ///
  /// Resets the given Camera Controller Component to the default value.
  ///
  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
    _descCameraControllerType(p_Ref) = CameraControllerType::kThirdPerson;
    _descTargetEulerAngles(p_Ref) = glm::vec3(0.0f);
    _descTargetObjectName(p_Ref) = "";
  }

  // <-

  ///
  /// Destroys the given Camera Controller Component by putting the reference
  /// back in to to the pool.
  ///
  _INTR_INLINE static void
  destroyCameraController(CameraControllerRef p_CameraController)
  {
    Dod::Components::ComponentManagerBase<
        CameraControllerData, _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT>::
        _destroyComponent(p_CameraController);
  }

  // <-

  ///
  /// Compiles all exposed properties to a JSON descriptor.
  ///
  _INTR_INLINE static void compileDescriptor(CameraControllerRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "cameraControllerType",
        _INTR_CREATE_PROP_ENUM(p_Document, p_GenerateDesc, _N(CameraController),
                               _N(enum), _descCameraControllerType(p_Ref),
                               "ThirdPerson,FirstPerson", false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember("targetObjectName",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(CameraController), _N(string),
                                             _descTargetObjectName(p_Ref),
                                             false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember("targetEulerAngles",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(CameraController), _N(vec3),
                                             _descTargetEulerAngles(p_Ref),
                                             false, false),
                           p_Document.GetAllocator());
  }

  // <-

  ///
  /// Initializes all properties from a JSON descriptor.
  ///
  _INTR_INLINE static void initFromDescriptor(CameraControllerRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("cameraControllerType"))
      _descCameraControllerType(p_Ref) =
          (CameraControllerType::Enum)JsonHelper::readPropertyEnumUint(
              p_Properties["cameraControllerType"]);
    if (p_Properties.HasMember("targetObjectName"))
      _descTargetObjectName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["targetObjectName"]);
    if (p_Properties.HasMember("targetEulerAngles"))
      _descTargetEulerAngles(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["targetEulerAngles"]);
  }

  // <-

  ///
  /// Updates the given controllers.
  ///
  static void
  updateControllers(const CameraControllerRefArray& p_CamControllers,
                    float p_DeltaT);

  // <-

  // Members refs
  // ->

  // Description

  /// The target orientation of the Camera Controller Component described in
  /// Euler Angles. Can be controlled/overwritten internally.
  _INTR_INLINE static glm::vec3&
  _descTargetEulerAngles(CameraControllerRef p_Ref)
  {
    return _data.descTargetEulerAngles[p_Ref._id];
  }
  /// The target the Camera Controller Component should follow.
  _INTR_INLINE static Name& _descTargetObjectName(CameraControllerRef p_Ref)
  {
    return _data.descTargetObjectName[p_Ref._id];
  }
  /// The type of Camera Controller used.
  _INTR_INLINE static CameraControllerType::Enum&
  _descCameraControllerType(CameraControllerRef p_Ref)
  {
    return _data.descCameraControllerType[p_Ref._id];
  }

  // Resources
  /// The euler angles seen during the last update call.
  _INTR_INLINE static glm::vec3&
  _lastTargetEulerAngles(CameraControllerRef p_Ref)
  {
    return _data.lastTargetEulerAngles[p_Ref._id];
  }
  /// The time passed since the last change in orientation.
  _INTR_INLINE static float&
  _timeSinceLastOrientationChange(CameraControllerRef p_Ref)
  {
    return _data.timeSinceLastOrientationChange[p_Ref._id];
  }
};
}
}
}
