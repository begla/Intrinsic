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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Components
{
// Typedefs
typedef Dod::Ref MeshRef;
typedef _INTR_ARRAY(MeshRef) MeshRefArray;

typedef _INTR_ARRAY(_INTR_ARRAY(Dod::Ref)) DrawCallArray;

struct MeshPerInstanceDataVertex
{
  glm::mat4 worldMatrix;
  glm::mat4 worldViewProjMatrix;
  glm::mat4 worldViewMatrix;
  glm::mat4 viewProjMatrix;
  glm::mat4 viewMatrix;

  glm::vec4 data0;
};

struct MeshPerInstanceDataFragment
{
  glm::vec4 colorTint;
  glm::vec4 camParams;
  glm::vec4 data0;
};

struct MeshData : Dod::Components::ComponentDataBase
{
  MeshData();

  // Description
  _INTR_ARRAY(Name) descMeshName;
  _INTR_ARRAY(glm::vec4) descColorTint;

  // Resources
  _INTR_ARRAY(MeshPerInstanceDataVertex) perInstanceDataVertex;
  _INTR_ARRAY(MeshPerInstanceDataFragment) perInstanceDataFragment;
  _INTR_ARRAY(DrawCallArray) drawCalls;
  _INTR_ARRAY(Components::NodeRef) node;
};

struct MeshManager
    : Dod::Components::ComponentManagerBase<MeshData,
                                            _INTR_MAX_MESH_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static MeshRef createMesh(Entity::EntityRef p_ParentEntity)
  {
    MeshRef ref = Dod::Components::ComponentManagerBase<
        MeshData,
        _INTR_MAX_MESH_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void destroyMesh(MeshRef p_Mesh)
  {
    Dod::Components::ComponentManagerBase<
        MeshData, _INTR_MAX_MESH_COMPONENT_COUNT>::_destroyComponent(p_Mesh);
  }

  // <-

  static void resetToDefault(MeshRef p_Mesh);

  // <-

  _INTR_INLINE static void compileDescriptor(MeshRef p_Ref, bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "meshName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Mesh),
                          _N(meshSelector), _descMeshName(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "colorTint",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Mesh), _N(color),
                          _descColorTint(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(MeshRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("meshName"))
    {
      _descMeshName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["meshName"]);
    }
    if (p_Properties.HasMember("colorTint"))
    {
      _descColorTint(p_Ref) =
          JsonHelper::readPropertyVec4(p_Properties["colorTint"]);
    }
  }

  // <-

  _INTR_INLINE static void createResources(MeshRef p_Mesh)
  {
    MeshRefArray meshes = {p_Mesh};
    createResources(meshes);
  }

  // <-

  _INTR_INLINE static void destroyResources(MeshRef p_Mesh)
  {
    MeshRefArray meshes = {p_Mesh};
    destroyResources(meshes);
  }

  // <-

  static void createResources(const MeshRefArray& p_Meshes);
  static void destroyResources(const MeshRefArray& p_Meshes);

  // <-

  static void updateUniformData(Dod::RefArray& p_Meshes);
  static void updatePerInstanceData(Dod::Ref p_CameraRef,
                                    uint32_t p_FrustumIdx);

  static void collectDrawCallsAndMeshComponents();

  // Scripting interface
  _INTR_INLINE static const Name& getMeshName(MeshRef p_Ref)
  {
    return _data.descMeshName[p_Ref._id];
  }
  _INTR_INLINE static void setMeshName(MeshRef p_Ref, const Name& p_Name)
  {
    _data.descMeshName[p_Ref._id] = p_Name;
  }

  // Description
  _INTR_INLINE static Name& _descMeshName(MeshRef p_Ref)
  {
    return _data.descMeshName[p_Ref._id];
  }
  _INTR_INLINE static glm::vec4& _descColorTint(MeshRef p_Ref)
  {
    return _data.descColorTint[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static MeshPerInstanceDataVertex&
  _perInstanceDataVertex(MeshRef p_Ref)
  {
    return _data.perInstanceDataVertex[p_Ref._id];
  }
  _INTR_INLINE static MeshPerInstanceDataFragment&
  _perInstanceDataFragment(MeshRef p_Ref)
  {
    return _data.perInstanceDataFragment[p_Ref._id];
  }
  _INTR_INLINE static DrawCallArray& _drawCalls(MeshRef p_Ref)
  {
    return _data.drawCalls[p_Ref._id];
  }
  _INTR_INLINE static Components::NodeRef& _node(MeshRef p_Ref)
  {
    return _data.node[p_Ref._id];
  }

  // <-
};
}
}
}
