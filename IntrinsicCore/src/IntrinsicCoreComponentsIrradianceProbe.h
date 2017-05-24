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

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref IrradianceProbeRef;
typedef _INTR_ARRAY(IrradianceProbeRef) IrradianceProbeRefArray;

struct SHCoeffs
{
  glm::vec3 L0;

  glm::vec3 L10;
  glm::vec3 L11;
  glm::vec3 L12;

  glm::vec3 L20;
  glm::vec3 L21;
  glm::vec3 L22;
  glm::vec3 L23;
  glm::vec3 L24;
};

struct IrradianceProbeData : Dod::Components::ComponentDataBase
{
  IrradianceProbeData()
      : Dod::Components::ComponentDataBase(
            _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT)
  {
    descRadius.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
    descSHCoeffs.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
  }

  _INTR_ARRAY(float) descRadius;
  _INTR_ARRAY(SHCoeffs) descSHCoeffs;
};

struct IrradianceProbeManager
    : Dod::Components::ComponentManagerBase<
          IrradianceProbeData, _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static IrradianceProbeRef
  createIrradianceProbe(Entity::EntityRef p_ParentEntity)
  {
    IrradianceProbeRef ref = Dod::Components::ComponentManagerBase<
        IrradianceProbeData, _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT>::
        _createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
    _descRadius(p_Ref) = 20.0f;
    memset(&_descSHCoeffs(p_Ref), 0x00, sizeof(SHCoeffs));
  }

  // <-

  _INTR_INLINE static void
  destroyIrradianceProbe(IrradianceProbeRef p_IrradianceProbe)
  {
    Dod::Components::ComponentManagerBase<
        IrradianceProbeData, _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT>::
        _destroyComponent(p_IrradianceProbe);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(IrradianceProbeRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember("radius",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(IrradianceProbe), _N(float),
                                             _descRadius(p_Ref), false, false),
                           p_Document.GetAllocator());

    p_Properties.AddMember(
        "shL0",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L0, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL10",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L10, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL11",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L11, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL12",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L12, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL20",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L20, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL21",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L21, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL22",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L22, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL23",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L23, false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "shL24",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(vec3), _descSHCoeffs(p_Ref).L24, false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(IrradianceProbeRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("radius"))
      _descRadius(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["radius"]);

    if (p_Properties.HasMember("shL0"))
      _descSHCoeffs(p_Ref).L0 =
          JsonHelper::readPropertyVec3(p_Properties["shL0"]);
    if (p_Properties.HasMember("shL10"))
      _descSHCoeffs(p_Ref).L10 =
          JsonHelper::readPropertyVec3(p_Properties["shL10"]);
    if (p_Properties.HasMember("shL11"))
      _descSHCoeffs(p_Ref).L11 =
          JsonHelper::readPropertyVec3(p_Properties["shL11"]);
    if (p_Properties.HasMember("shL12"))
      _descSHCoeffs(p_Ref).L12 =
          JsonHelper::readPropertyVec3(p_Properties["shL12"]);
    if (p_Properties.HasMember("shL20"))
      _descSHCoeffs(p_Ref).L20 =
          JsonHelper::readPropertyVec3(p_Properties["shL20"]);
    if (p_Properties.HasMember("shL21"))
      _descSHCoeffs(p_Ref).L21 =
          JsonHelper::readPropertyVec3(p_Properties["shL21"]);
    if (p_Properties.HasMember("shL22"))
      _descSHCoeffs(p_Ref).L22 =
          JsonHelper::readPropertyVec3(p_Properties["shL22"]);
    if (p_Properties.HasMember("shL23"))
      _descSHCoeffs(p_Ref).L23 =
          JsonHelper::readPropertyVec3(p_Properties["shL23"]);
    if (p_Properties.HasMember("shL24"))
      _descSHCoeffs(p_Ref).L24 =
          JsonHelper::readPropertyVec3(p_Properties["shL24"]);
  }

  // <-

  // Getter/Setter
  // ->

  // Description
  _INTR_INLINE static float& _descRadius(IrradianceProbeRef p_Ref)
  {
    return _data.descRadius[p_Ref._id];
  }

  _INTR_INLINE static SHCoeffs& _descSHCoeffs(IrradianceProbeRef p_Ref)
  {
    return _data.descSHCoeffs[p_Ref._id];
  }
};
}
}
}
