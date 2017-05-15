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
namespace Resources
{
typedef Dod::Ref PostEffectRef;
typedef _INTR_ARRAY(PostEffectRef) PostEffectRefArray;

struct PostEffectData : Dod::Resources::ResourceDataBase
{
  PostEffectData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_POST_EFFECT_COUNT)
  {
    descVolumetricLightingScattering.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descVolumetricLightingLocalLightIntensity.resize(
        _INTR_MAX_POST_EFFECT_COUNT);

    descMainLightTemp.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descMainLightColor.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descMainLightOrientation.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descMainLightIntens.resize(_INTR_MAX_POST_EFFECT_COUNT);
  }

  // <-

  _INTR_ARRAY(float) descVolumetricLightingScattering;
  _INTR_ARRAY(float) descVolumetricLightingLocalLightIntensity;

  _INTR_ARRAY(float) descMainLightTemp;
  _INTR_ARRAY(glm::vec3) descMainLightColor;
  _INTR_ARRAY(glm::quat) descMainLightOrientation;
  _INTR_ARRAY(float) descMainLightIntens;
};

struct PostEffectManager
    : Dod::Resources::ResourceManagerBase<PostEffectData,
                                          _INTR_MAX_POST_EFFECT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static PostEffectRef createPostEffect(const Name& p_Name)
  {
    PostEffectRef ref = Dod::Resources::ResourceManagerBase<
        PostEffectData, _INTR_MAX_POST_EFFECT_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(PostEffectRef p_Ref)
  {
    _descVolumetricLightingScattering(p_Ref) = 0.0f;
    _descVolumetricLightingLocalLightIntensity(p_Ref) = 0.0f;
    _descMainLightColor(p_Ref) = glm::vec3(1.0f);
    _descMainLightIntens(p_Ref) = 10.0f;
    _descMainLightOrientation(p_Ref) = glm::quat();
    _descMainLightTemp(p_Ref) = 3200.0f;
  }

  // <-

  _INTR_INLINE static void destroyPostEffect(PostEffectRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        PostEffectData, _INTR_MAX_POST_EFFECT_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(PostEffectRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        PostEffectData,
        _INTR_MAX_POST_EFFECT_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                         p_Properties,
                                                         p_Document);

    p_Properties.AddMember(
        "Scattering",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(VolumetricLighting),
                          _N(float), _descVolumetricLightingScattering(p_Ref),
                          false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "LocalLightIntensity",
        _INTR_CREATE_PROP(
            p_Document, p_GenerateDesc, _N(VolumetricLighting), _N(float),
            _descVolumetricLightingLocalLightIntensity(p_Ref), false, false),
        p_Document.GetAllocator());

    p_Properties.AddMember(
        "MainLightIntensity",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descMainLightIntens(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "MainLightTemperature",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descMainLightTemp(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember("MainLightOrientation",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Lighting), _N(rotation),
                                             _descMainLightOrientation(p_Ref),
                                             false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "MainLightColor",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(vec3),
                          _descMainLightColor(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(PostEffectRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        PostEffectData,
        _INTR_MAX_POST_EFFECT_COUNT>::_initFromDescriptor(p_Ref, p_Properties);

    if (p_Properties.HasMember("Scattering"))
      _descVolumetricLightingScattering(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["Scattering"]);
    if (p_Properties.HasMember("LocalLightIntensity"))
      _descVolumetricLightingLocalLightIntensity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["LocalLightIntensity"]);

    if (p_Properties.HasMember("MainLightIntensity"))
      _descMainLightIntens(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["MainLightIntensity"]);
    if (p_Properties.HasMember("MainLightTemperature"))
      _descMainLightTemp(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["MainLightTemperature"]);
    if (p_Properties.HasMember("MainLightOrientation"))
      _descMainLightOrientation(p_Ref) =
          JsonHelper::readPropertyQuat(p_Properties["MainLightOrientation"]);
    if (p_Properties.HasMember("MainLightColor"))
      _descMainLightColor(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["MainLightColor"]);
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<PostEffectData,
                                        _INTR_MAX_POST_EFFECT_COUNT>::
        _saveToMultipleFiles<
            rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<
        PostEffectData,
        _INTR_MAX_POST_EFFECT_COUNT>::_loadFromMultipleFiles(p_Path,
                                                             p_Extension,
                                                             initFromDescriptor,
                                                             resetToDefault);
  }

  // <-

  _INTR_INLINE static void blendPostEffect(Dod::Ref p_Target, Dod::Ref p_Left,
                                           Dod::Ref p_Right,
                                           float p_BlendFactor)
  {
    _descVolumetricLightingScattering(p_Target) =
        glm::mix(_descVolumetricLightingScattering(p_Left),
                 _descVolumetricLightingScattering(p_Right), p_BlendFactor);
    _descVolumetricLightingLocalLightIntensity(p_Target) = glm::mix(
        _descVolumetricLightingLocalLightIntensity(p_Left),
        _descVolumetricLightingLocalLightIntensity(p_Right), p_BlendFactor);

    _descMainLightColor(p_Target) =
        glm::mix(_descMainLightColor(p_Left), _descMainLightColor(p_Right),
                 p_BlendFactor);
    _descMainLightIntens(p_Target) =
        glm::mix(_descMainLightIntens(p_Left), _descMainLightIntens(p_Right),
                 p_BlendFactor);
    _descMainLightTemp(p_Target) = glm::mix(
        _descMainLightTemp(p_Left), _descMainLightTemp(p_Right), p_BlendFactor);
    _descMainLightOrientation(p_Target) =
        glm::slerp(_descMainLightOrientation(p_Left),
                   _descMainLightOrientation(p_Right), p_BlendFactor);
  }

  // <-

  // Members refs.
  // ->

  // Description
  _INTR_INLINE static float&
  _descVolumetricLightingScattering(PostEffectRef p_Ref)
  {
    return _data.descVolumetricLightingScattering[p_Ref._id];
  }
  _INTR_INLINE static float&
  _descVolumetricLightingLocalLightIntensity(PostEffectRef p_Ref)
  {
    return _data.descVolumetricLightingLocalLightIntensity[p_Ref._id];
  }

  _INTR_INLINE static float& _descMainLightIntens(PostEffectRef p_Ref)
  {
    return _data.descMainLightIntens[p_Ref._id];
  }
  _INTR_INLINE static float& _descMainLightTemp(PostEffectRef p_Ref)
  {
    return _data.descMainLightTemp[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3& _descMainLightColor(PostEffectRef p_Ref)
  {
    return _data.descMainLightColor[p_Ref._id];
  }
  _INTR_INLINE static glm::quat& _descMainLightOrientation(PostEffectRef p_Ref)
  {
    return _data.descMainLightOrientation[p_Ref._id];
  }

  // Static members
  static PostEffectRef _blendTargetRef;
};
}
}
}
