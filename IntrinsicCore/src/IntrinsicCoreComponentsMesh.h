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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref MeshRef;
typedef _INTR_ARRAY(MeshRef) MeshRefArray;

typedef _INTR_ARRAY(_INTR_ARRAY(Dod::Ref)) DrawCallArray;

struct MeshPerInstanceDataVertex
{
  glm::mat4 worldMatrix;
  glm::mat4 normalMatrix;
  glm::mat4 worldViewProjMatrix;

  glm::vec4 data0;
};

struct MeshPerInstanceDataFragment
{
  glm::vec4 data0;
};

struct MeshData : Dod::Components::ComponentDataBase
{
  MeshData();

  _INTR_ARRAY(Name) descMeshName;

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

  _INTR_INLINE static void compileDescriptor(MeshRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "meshName", _INTR_CREATE_PROP(p_Document, _N(Mesh), _N(string),
                                      _descMeshName(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(MeshRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("meshName"))
    {
      _descMeshName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["meshName"]);
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
  static void updatePerInstanceData(uint32_t p_FrustumIdx);

  static void collectDrawCallsAndMeshComponents();

  // Getter/Setter
  // Intrinsic

  _INTR_INLINE static const Name& getMeshName(MeshRef p_Ref)
  {
    return _data.descMeshName[p_Ref._id];
  }
  _INTR_INLINE static void setMeshName(MeshRef p_Ref, const Name& p_Name)
  {
    _data.descMeshName[p_Ref._id] = p_Name;
  }

  // <-

  // Member refs
  // Intrinsic

  // Description
  _INTR_INLINE static Name& _descMeshName(MeshRef p_Ref)
  {
    return _data.descMeshName[p_Ref._id];
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

  // Refs
  _INTR_INLINE static Components::NodeRef& _node(MeshRef p_Ref)
  {
    return _data.node[p_Ref._id];
  }

  // <-
};
}
}
}
