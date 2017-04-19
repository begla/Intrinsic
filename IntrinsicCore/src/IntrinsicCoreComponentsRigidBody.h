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

namespace physx
{
class PxRigidActor;
class PxMaterial;
}

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref RigidBodyRef;
typedef _INTR_ARRAY(RigidBodyRef) RigidBodyRefArray;

namespace RigidBodyType
{
enum Enum
{
  kBoxKinematic,
  kBoxDynamic,
  kTriangleMeshStatic,
  kTriangleMeshKinematic,
  kSphereKinematic,
  kSphereDynamic,
};
}

struct RigidBodyData : Dod::Components::ComponentDataBase
{
  RigidBodyData();

  // Description
  _INTR_ARRAY(RigidBodyType::Enum) descRigidBodyType;
  _INTR_ARRAY(float) descDensity;

  // Resources
  _INTR_ARRAY(physx::PxRigidActor*) pxRigidActor;
};

struct RigidBodyManager
    : Dod::Components::ComponentManagerBase<
          RigidBodyData, _INTR_MAX_RIGID_BODY_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static RigidBodyRef
  createRigidBody(Entity::EntityRef p_ParentEntity)
  {
    RigidBodyRef ref = Dod::Components::ComponentManagerBase<
        RigidBodyData,
        _INTR_MAX_RIGID_BODY_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void destroyRigidBody(RigidBodyRef p_RigidBody)
  {
    Dod::Components::ComponentManagerBase<
        RigidBodyData,
        _INTR_MAX_RIGID_BODY_COMPONENT_COUNT>::_destroyComponent(p_RigidBody);
  }

  // <-

  static void resetToDefault(RigidBodyRef p_RigidBody);

  // <-

  _INTR_INLINE static void compileDescriptor(RigidBodyRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "rigidBodyType",
        _INTR_CREATE_PROP_ENUM(
            p_Document, p_GenerateDesc, _N(RigidBody), _N(enum),
            _descRigidBodyType(p_Ref),
            "BoxKinematic,BoxDynamic,TriangleMeshStatic,TriangleMeshKinematic,"
            "SphereKinematic,SphereDynamic,",
            false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember("density",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(RigidBody), _N(float),
                                             _descDensity(p_Ref), false, false),
                           p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(RigidBodyRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("rigidBodyType"))
    {
      _descRigidBodyType(p_Ref) =
          (RigidBodyType::Enum)JsonHelper::readPropertyEnum(
              p_Properties["rigidBodyType"]);
    }
    if (p_Properties.HasMember("density"))
    {
      _descDensity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["density"]);
    }
  }

  // <-

  _INTR_INLINE static void createResources(RigidBodyRef p_RigidBody)
  {
    RigidBodyRefArray RigidBodyes = {p_RigidBody};
    createResources(RigidBodyes);
  }
  _INTR_INLINE static void destroyResources(RigidBodyRef p_RigidBody)
  {
    RigidBodyRefArray RigidBodyes = {p_RigidBody};
    destroyResources(RigidBodyes);
  }

  // <-

  static void createResources(const RigidBodyRefArray& p_RigidBodies);
  static void destroyResources(const RigidBodyRefArray& p_RigidBodies);

  // <-

  static void updateNodesFromActors(const RigidBodyRefArray& p_RigidBodies);
  static void updateActorsFromNodes(const RigidBodyRefArray& p_RigidBodies);

  // Member refs
  // ->

  // Description
  _INTR_INLINE static RigidBodyType::Enum&
  _descRigidBodyType(RigidBodyRef p_Ref)
  {
    return _data.descRigidBodyType[p_Ref._id];
  }
  _INTR_INLINE static float& _descDensity(RigidBodyRef p_Ref)
  {
    return _data.descDensity[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static physx::PxRigidActor*& _pxRigidActor(RigidBodyRef p_Ref)
  {
    return _data.pxRigidActor[p_Ref._id];
  }

  // Static members
  static physx::PxMaterial* _defaultMaterial;

  // <-
};
}
}
}
