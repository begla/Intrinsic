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

    descSunOrientation.resize(_INTR_MAX_POST_EFFECT_COUNT);

    descDayNightFactor.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descSunIntensity.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descSkyAlbedo.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descSkyLightIntensity.resize(_INTR_MAX_POST_EFFECT_COUNT);
    descSkyTurbidity.resize(_INTR_MAX_POST_EFFECT_COUNT);
  }

  // <-

  _INTR_ARRAY(float) descVolumetricLightingScattering;

  _INTR_ARRAY(glm::quat) descSunOrientation;

  _INTR_ARRAY(float) descDayNightFactor;
  _INTR_ARRAY(float) descSunIntensity;
  _INTR_ARRAY(float) descSkyTurbidity;
  _INTR_ARRAY(float) descSkyAlbedo;
  _INTR_ARRAY(float) descSkyLightIntensity;
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
    _descSunOrientation(p_Ref) = glm::quat();
    _descSunIntensity(p_Ref) = 10.0f;
    _descDayNightFactor(p_Ref) = 1.0f;
    _descSkyTurbidity(p_Ref) = 2.0f;
    _descSkyAlbedo(p_Ref) = 0.0f;
    _descSkyLightIntensity(p_Ref) = 0.05f;
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
        "scattering",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(VolumetricLighting),
                          _N(float), _descVolumetricLightingScattering(p_Ref),
                          false, false),
        p_Document.GetAllocator());

    p_Properties.AddMember("sunOrientation",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Lighting), _N(rotation),
                                             _descSunOrientation(p_Ref), false,
                                             false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "dayNightFactor",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descDayNightFactor(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "skyLightIntensity",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descSkyLightIntensity(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "skyTurbidity",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descSkyTurbidity(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "skyAlbedo",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descSkyAlbedo(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "sunIntensity",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Lighting), _N(float),
                          _descSunIntensity(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(PostEffectRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        PostEffectData,
        _INTR_MAX_POST_EFFECT_COUNT>::_initFromDescriptor(p_Ref, p_Properties);

    if (p_Properties.HasMember("scattering"))
      _descVolumetricLightingScattering(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["scattering"]);

    if (p_Properties.HasMember("sunOrientation"))
      _descSunOrientation(p_Ref) =
          JsonHelper::readPropertyQuat(p_Properties["sunOrientation"]);
    if (p_Properties.HasMember("sunIntensity"))
      _descSunIntensity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["sunIntensity"]);
    if (p_Properties.HasMember("skyTurbidity"))
      _descSkyTurbidity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["skyTurbidity"]);
    if (p_Properties.HasMember("skyAlbedo"))
      _descSkyAlbedo(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["skyAlbedo"]);
    if (p_Properties.HasMember("skyLightIntensity"))
      _descSkyLightIntensity(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["skyLightIntensity"]);
    if (p_Properties.HasMember("dayNightFactor"))
      _descDayNightFactor(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["dayNightFactor"]);
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

  _INTR_INLINE static glm::quat calcActualSunOrientation(PostEffectRef p_Ref)
  {
    return glm::slerp(_descSunOrientation(p_Ref),
                      World::_currentSunLightOrientation,
                      _descDayNightFactor(p_Ref));
  }

  // <-

  _INTR_INLINE static void blendPostEffect(Dod::Ref p_Target, Dod::Ref p_Left,
                                           Dod::Ref p_Right,
                                           float p_BlendFactor)
  {
    _descVolumetricLightingScattering(p_Target) =
        glm::mix(_descVolumetricLightingScattering(p_Left),
                 _descVolumetricLightingScattering(p_Right), p_BlendFactor);

    _descSunOrientation(p_Target) =
        glm::slerp(_descSunOrientation(p_Left), _descSunOrientation(p_Right),
                   p_BlendFactor);
    _descDayNightFactor(p_Target) =
        glm::mix(_descDayNightFactor(p_Left), _descDayNightFactor(p_Right),
                 p_BlendFactor);
    _descSkyTurbidity(p_Target) = glm::mix(
        _descSkyTurbidity(p_Left), _descSkyTurbidity(p_Right), p_BlendFactor);
    _descSkyAlbedo(p_Target) = glm::mix(_descSkyAlbedo(p_Left),
                                        _descSkyAlbedo(p_Right), p_BlendFactor);
    _descSkyLightIntensity(p_Target) =
        glm::mix(_descSkyLightIntensity(p_Left),
                 _descSkyLightIntensity(p_Right), p_BlendFactor);
    _descSunIntensity(p_Target) = glm::mix(
        _descSunIntensity(p_Left), _descSunIntensity(p_Right), p_BlendFactor);
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

  _INTR_INLINE static glm::quat& _descSunOrientation(PostEffectRef p_Ref)
  {
    return _data.descSunOrientation[p_Ref._id];
  }
  _INTR_INLINE static float& _descDayNightFactor(PostEffectRef p_Ref)
  {
    return _data.descDayNightFactor[p_Ref._id];
  }
  _INTR_INLINE static float& _descSkyLightIntensity(PostEffectRef p_Ref)
  {
    return _data.descSkyLightIntensity[p_Ref._id];
  }
  _INTR_INLINE static float& _descSkyAlbedo(PostEffectRef p_Ref)
  {
    return _data.descSkyAlbedo[p_Ref._id];
  }
  _INTR_INLINE static float& _descSkyTurbidity(PostEffectRef p_Ref)
  {
    return _data.descSkyTurbidity[p_Ref._id];
  }
  _INTR_INLINE static float& _descSunIntensity(PostEffectRef p_Ref)
  {
    return _data.descSunIntensity[p_Ref._id];
  }

  // Static members
  static PostEffectRef _blendTargetRef;
};
}
}
}
