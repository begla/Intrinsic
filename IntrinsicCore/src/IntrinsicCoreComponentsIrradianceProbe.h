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

struct IrradianceProbeData : Dod::Components::ComponentDataBase
{
  IrradianceProbeData()
      : Dod::Components::ComponentDataBase(
            _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT)
  {
    descRadius.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
    descPriority.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
    descFalloffRangePerc.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
    descFalloffExponent.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);

    descSHs.resize(_INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT);
  }

  _INTR_ARRAY(float) descRadius;
  _INTR_ARRAY(float) descFalloffRangePerc;
  _INTR_ARRAY(float) descFalloffExponent;
  _INTR_ARRAY(uint32_t) descPriority;

  _INTR_ARRAY(_INTR_ARRAY(Irradiance::SH9)) descSHs;
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
    _descFalloffRangePerc(p_Ref) = 0.2f;
    _descFalloffExp(p_Ref) = 1.0f;
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
    p_Properties.AddMember("falloffRangePerc",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(IrradianceProbe), _N(float),
                                             _descFalloffRangePerc(p_Ref),
                                             false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "falloffExp",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(float), _descFalloffExp(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "priority",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(IrradianceProbe),
                          _N(float), _descPriority(p_Ref), false, false),
        p_Document.GetAllocator());

    rapidjson::Value shs = rapidjson::Value(rapidjson::kArrayType);
    for (uint32_t i = 0u; i < _descSHs(p_Ref).size(); ++i)
    {
      shs.PushBack(_INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                     _N(IrradianceProbe), _N(sh),
                                     _descSHs(p_Ref)[i], false, false),
                   p_Document.GetAllocator());
    }
    p_Properties.AddMember("shs", shs, p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(IrradianceProbeRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("radius"))
      _descRadius(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["radius"]);
    if (p_Properties.HasMember("falloffRangePerc"))
      _descFalloffRangePerc(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["falloffRangePerc"]);
    if (p_Properties.HasMember("falloffExp"))
      _descFalloffExp(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["falloffExp"]);
    if (p_Properties.HasMember("priority"))
      _descPriority(p_Ref) =
          (uint32_t)JsonHelper::readPropertyFloat(p_Properties["priority"]);

    if (p_Properties.HasMember("shs"))
    {
      _descSHs(p_Ref).clear();
      const rapidjson::Value& shs = p_Properties["shs"];
      for (uint32_t i = 0u; i < shs.Size(); ++i)
      {
        _descSHs(p_Ref).push_back(JsonHelper::readPropertySH(shs[i]));
      }
    }
  }

  // <-

  _INTR_INLINE static void sortByPriority(IrradianceProbeRefArray& p_Probes)
  {
    _INTR_PROFILE_CPU("General", "Sort Irradiance Probes");

    struct Comparator
    {
      bool operator()(const Dod::Ref& a, const Dod::Ref& b) const
      {
        return _descPriority(a) < _descPriority(b);
      }
    } comp;

    Algorithm::parallelSort<Dod::Ref, Comparator>(p_Probes, comp);
  }

  // <-

  // Getter/Setter
  // ->

  // Description
  _INTR_INLINE static float& _descRadius(IrradianceProbeRef p_Ref)
  {
    return _data.descRadius[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descPriority(IrradianceProbeRef p_Ref)
  {
    return _data.descPriority[p_Ref._id];
  }
  _INTR_INLINE static float& _descFalloffRangePerc(IrradianceProbeRef p_Ref)
  {
    return _data.descFalloffRangePerc[p_Ref._id];
  }
  _INTR_INLINE static float& _descFalloffExp(IrradianceProbeRef p_Ref)
  {
    return _data.descFalloffExponent[p_Ref._id];
  }

  _INTR_INLINE static _INTR_ARRAY(Irradiance::SH9) &
      _descSHs(IrradianceProbeRef p_Ref)
  {
    return _data.descSHs[p_Ref._id];
  }
};
}
}
}
