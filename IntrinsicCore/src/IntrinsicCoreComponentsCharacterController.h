// Copyright 2016 Benjamin Glatzel
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

#pragma once

// Forward decls.
namespace physx
{
class PxController;
}

namespace Intrinsic
{
namespace Core
{
namespace Components
{
namespace CharacterControllerCollisionFlags
{
enum Flags
{
  kSides = 0x01,
  kUp = 0x02,
  kDown = 0x04
};
}

typedef Dod::Ref CharacterControllerRef;
typedef _INTR_ARRAY(CharacterControllerRef) CharacterControllerRefArray;

struct CharacterControllerData : Dod::Components::ComponentDataBase
{
  CharacterControllerData()
      : Dod::Components::ComponentDataBase(
            _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT)
  {
    pxController.resize(_INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT);
    internalMoveVector.resize(_INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT);
    currentMoveVector.resize(_INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT);
    lastCollisionFlags.resize(_INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT);
  }

  // Resources
  _INTR_ARRAY(glm::vec3) internalMoveVector;
  _INTR_ARRAY(glm::vec3) currentMoveVector;
  _INTR_ARRAY(physx::PxController*) pxController;
  _INTR_ARRAY(uint32_t) lastCollisionFlags;
};

struct CharacterControllerManager
    : Dod::Components::ComponentManagerBase<
          CharacterControllerData,
          _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static CharacterControllerRef
  createCharacterController(Entity::EntityRef p_ParentEntity)
  {
    CharacterControllerRef ref = Dod::Components::ComponentManagerBase<
        CharacterControllerData,
        _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT>::
        _createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(CharacterControllerRef p_Ref) {}

  // <-

  _INTR_INLINE static void
  destroyCharacterController(CharacterControllerRef p_CharacterController)
  {
    Dod::Components::ComponentManagerBase<
        CharacterControllerData,
        _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT>::
        _destroyComponent(p_CharacterController);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(CharacterControllerRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(CharacterControllerRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
  }

  // <-

  _INTR_INLINE static void createResources(CharacterControllerRef p_Mesh)
  {
    CharacterControllerRefArray meshes = {p_Mesh};
    createResources(meshes);
  }

  // <-

  _INTR_INLINE static void destroyResources(CharacterControllerRef p_Mesh)
  {
    CharacterControllerRefArray meshes = {p_Mesh};
    destroyResources(meshes);
  }

  // <-

  static void createResources(const CharacterControllerRefArray& p_Meshes);
  static void destroyResources(const CharacterControllerRefArray& p_Meshes);

  // <-

  _INTR_INLINE static void move(CharacterControllerRef p_Ref,
                                const glm::vec3& p_MoveVector)
  {
    _currentMoveVector(p_Ref) += p_MoveVector;
  }

  // <-

  _INTR_INLINE static bool isGrounded(CharacterControllerRef p_Ref)
  {
    return (_lastCollisionFlags(p_Ref) &
            CharacterControllerCollisionFlags::kDown) > 0u;
  }

  // <-

  static void
  updateControllers(const CharacterControllerRefArray& p_CharacterControllers,
                    float p_DeltaT);

  // <-

  // Members refs
  // ->

  // Resources
  _INTR_INLINE static glm::vec3&
  _currentMoveVector(CharacterControllerRef p_Ref)
  {
    return _data.currentMoveVector[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3&
  _internalMoveVector(CharacterControllerRef p_Ref)
  {
    return _data.internalMoveVector[p_Ref._id];
  }

  _INTR_INLINE static physx::PxController*&
  _pxController(CharacterControllerRef p_Ref)
  {
    return _data.pxController[p_Ref._id];
  }
  _INTR_INLINE static uint32_t&
  _lastCollisionFlags(CharacterControllerRef p_Ref)
  {
    return _data.lastCollisionFlags[p_Ref._id];
  }
};
}
}
}
