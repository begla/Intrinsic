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
typedef Dod::Ref LightRef;
typedef _INTR_ARRAY(LightRef) LightRefArray;

struct LightData : Dod::Components::ComponentDataBase
{
  LightData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_LIGHT_COMPONENT_COUNT)
  {
    descRadius.resize(_INTR_MAX_LIGHT_COMPONENT_COUNT);
    descColor.resize(_INTR_MAX_LIGHT_COMPONENT_COUNT);
    descTemperature.resize(_INTR_MAX_LIGHT_COMPONENT_COUNT);
    descIntensity.resize(_INTR_MAX_LIGHT_COMPONENT_COUNT);
  }

  // Description
  _INTR_ARRAY(float) descRadius;
  _INTR_ARRAY(glm::vec3) descColor;
  _INTR_ARRAY(float) descIntensity;
  _INTR_ARRAY(float) descTemperature;
};

struct LightManager
    : Dod::Components::ComponentManagerBase<LightData,
                                            _INTR_MAX_LIGHT_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static LightRef createLight(Entity::EntityRef p_ParentEntity)
  {
    LightRef ref = Dod::Components::ComponentManagerBase<
        LightData,
        _INTR_MAX_LIGHT_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
    _descRadius(p_Ref) = 5.0f;
    _descColor(p_Ref) = glm::vec3(1.0f, 1.0f, 1.0f);
    _descTemperature(p_Ref) = 6500.0f;
    _descIntensity(p_Ref) = 5.0f;
  }

  // <-

  _INTR_INLINE static void destroyLight(LightRef p_Light)
  {
    Dod::Components::ComponentManagerBase<
        LightData, _INTR_MAX_LIGHT_COMPONENT_COUNT>::_destroyComponent(p_Light);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(LightRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember("radius",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Light), _N(float),
                                             _descRadius(p_Ref), false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember("color",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Light), _N(color),
                                             _descColor(p_Ref), false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "intensity",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Light), _N(float),
                          _descIntensity(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "temperature",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Light), _N(float),
                          _descTemperature(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(LightRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("radius"))
      _descRadius(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["radius"]);
    if (p_Properties.HasMember("color"))
      _descColor(p_Ref) = JsonHelper::readPropertyVec3(p_Properties["color"]);
    if (p_Properties.HasMember("intensity"))
      _descIntensity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["intensity"]);
    if (p_Properties.HasMember("temperature"))
      _descTemperature(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["temperature"]);
  }

  // <-

  // Description
  _INTR_INLINE static float& _descRadius(LightRef p_Ref)
  {
    return _data.descRadius[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3& _descColor(LightRef p_Ref)
  {
    return _data.descColor[p_Ref._id];
  }
  _INTR_INLINE static float& _descIntensity(LightRef p_Ref)
  {
    return _data.descIntensity[p_Ref._id];
  }
  _INTR_INLINE static float& _descTemperature(LightRef p_Ref)
  {
    return _data.descTemperature[p_Ref._id];
  }
};
}
}
}
