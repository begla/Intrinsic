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
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
typedef Dod::Ref MaterialRef;
typedef Dod::RefArray MaterialRefArray;
typedef Name& (*MaterialResourceFunction)(Dod::Ref);

struct PerMaterialDataVertex
{
  float _dummy;
};

struct PerMaterialDataFragment
{
  glm::vec4 uvOffsetScale;
  glm::vec4 uvAnimation;
  glm::vec4 pbrBias;

  uint32_t data0[4];
};

namespace MaterialPass
{
namespace BoundResourceType
{
enum Enum
{
  kImage,
  kBuffer
};
}

struct MaterialPass
{
  Name name;
  uint8_t pipelineIdx;
  uint8_t pipelineLayoutIdx;
  uint8_t boundResoucesIdx;
};

struct BoundResourceEntry
{
  uint8_t type;
  uint8_t shaderStage;
  Name resourceName;
  Name slotName;
};

struct BoundResources
{
  Name name;
  _INTR_ARRAY(BoundResourceEntry) boundResourceEntries;
};
}

struct MaterialData : Dod::Resources::ResourceDataBase
{
  MaterialData() : Dod::Resources::ResourceDataBase(_INTR_MAX_MATERIAL_COUNT)
  {
    descAlbedoTextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descAlbedo1TextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descAlbedo2TextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descNormalTextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descNormal1TextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descNormal2TextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descPbrTextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descPbr1TextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descPbr2TextureName.resize(_INTR_MAX_MATERIAL_COUNT);

    descBlendMaskTextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descFoamTextureName.resize(_INTR_MAX_MATERIAL_COUNT);
    descFoamFadeDistance.resize(_INTR_MAX_MATERIAL_COUNT);
    descRefractionFactor.resize(_INTR_MAX_MATERIAL_COUNT);

    descTranslucencyThickness.resize(_INTR_MAX_MATERIAL_COUNT);

    descUvOffsetScale.resize(_INTR_MAX_MATERIAL_COUNT);
    descUvAnimation.resize(_INTR_MAX_MATERIAL_COUNT);
    descPbrBias.resize(_INTR_MAX_MATERIAL_COUNT);

    descMaterialPassMask.resize(_INTR_MAX_MATERIAL_COUNT);

    perMaterialDataVertexOffset.resize(_INTR_MAX_MATERIAL_COUNT);
    perMaterialDataFragmentOffset.resize(_INTR_MAX_MATERIAL_COUNT);
    materialBufferEntryIndex.resize(_INTR_MAX_MATERIAL_COUNT);
    materialPassMask.resize(_INTR_MAX_MATERIAL_COUNT);

    memset(perMaterialDataFragmentOffset.data(), 0xFF,
           perMaterialDataFragmentOffset.size() * sizeof(uint32_t));
    memset(perMaterialDataVertexOffset.data(), 0xFF,
           perMaterialDataVertexOffset.size() * sizeof(uint32_t));
    memset(materialBufferEntryIndex.data(), 0xFF,
           materialBufferEntryIndex.size() * sizeof(uint32_t));
  }

  // Description
  _INTR_ARRAY(Name) descAlbedoTextureName;
  _INTR_ARRAY(Name) descAlbedo1TextureName;
  _INTR_ARRAY(Name) descAlbedo2TextureName;
  _INTR_ARRAY(Name) descNormalTextureName;
  _INTR_ARRAY(Name) descNormal1TextureName;
  _INTR_ARRAY(Name) descNormal2TextureName;
  _INTR_ARRAY(Name) descPbrTextureName;
  _INTR_ARRAY(Name) descPbr1TextureName;
  _INTR_ARRAY(Name) descPbr2TextureName;

  _INTR_ARRAY(Name) descBlendMaskTextureName;
  _INTR_ARRAY(Name) descFoamTextureName;
  _INTR_ARRAY(float) descFoamFadeDistance;
  _INTR_ARRAY(float) descRefractionFactor;

  _INTR_ARRAY(float) descTranslucencyThickness;

  _INTR_ARRAY(glm::vec4) descUvOffsetScale;
  _INTR_ARRAY(glm::vec2) descUvAnimation;
  _INTR_ARRAY(glm::vec3) descPbrBias;

  _INTR_ARRAY(_INTR_ARRAY(Name)) descMaterialPassMask;

  // Resources
  _INTR_ARRAY(uint32_t) perMaterialDataVertexOffset;
  _INTR_ARRAY(uint32_t) perMaterialDataFragmentOffset;
  _INTR_ARRAY(uint32_t) materialBufferEntryIndex;
  _INTR_ARRAY(uint32_t) materialPassMask;
};

struct MaterialManager
    : Dod::Resources::ResourceManagerBase<MaterialData,
                                          _INTR_MAX_MATERIAL_COUNT>
{
  static void init();

  _INTR_INLINE static MaterialRef createMaterial(const Name& p_Name)
  {
    MaterialRef ref = Dod::Resources::ResourceManagerBase<
        MaterialData, _INTR_MAX_MATERIAL_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(MaterialRef p_Ref)
  {
    _descAlbedoTextureName(p_Ref) = _N(checkerboard);
    _descAlbedo1TextureName(p_Ref) = _N(checkerboard);
    _descAlbedo2TextureName(p_Ref) = _N(checkerboard);
    _descNormalTextureName(p_Ref) = _N(default_NRM);
    _descNormal1TextureName(p_Ref) = _N(default_NRM);
    _descNormal2TextureName(p_Ref) = _N(default_NRM);
    _descPbrTextureName(p_Ref) = _N(default_PBR);
    _descPbr1TextureName(p_Ref) = _N(default_PBR);
    _descPbr2TextureName(p_Ref) = _N(default_PBR);

    _descBlendMaskTextureName(p_Ref) = _N(white);
    _descFoamTextureName(p_Ref) = _N(checkerboard);
    _descFoamFadeDistance(p_Ref) = 1.0f;
    _descRefractionFactor(p_Ref) = 0.0f;
    _descTranslucencyThickness(p_Ref) = 0.0f;

    _descUvOffsetScale(p_Ref) = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    _descPbrBias(p_Ref) = glm::vec3(0.0f);
    _descUvAnimation(p_Ref) = glm::vec2(0.0f);

    _descMaterialPassMask(p_Ref) = {"GBuffer", "Shadow", "PerPixelPicking"};
  }

  _INTR_INLINE static void destroyMaterial(MaterialRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        MaterialData, _INTR_MAX_MATERIAL_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void updateMaterialPassFlags(MaterialRef p_Ref) {}

  _INTR_INLINE static void compileDescriptor(MaterialRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        MaterialData,
        _INTR_MAX_MATERIAL_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                      p_Properties, p_Document);

    _INTR_STRING materialPassNames;
    for (auto& matPass : _materialPasses)
      materialPassNames += "," + matPass.name.getString();

    p_Properties.AddMember(
        "materialPassMask",
        _INTR_CREATE_PROP_FLAGS(p_Document, p_GenerateDesc, _N(Material), "",
                                _descMaterialPassMask(p_Ref), materialPassNames,
                                false, true),
        p_Document.GetAllocator());

    // General
    {
      p_Properties.AddMember(
          "albedoTextureName",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Textures), "string",
                            _descAlbedoTextureName(p_Ref), false, false),
          p_Document.GetAllocator());
      p_Properties.AddMember(
          "uvOffsetScale",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Properties_UV),
                            "vec4", _descUvOffsetScale(p_Ref), false, false),
          p_Document.GetAllocator());
      p_Properties.AddMember(
          "uvAnimation",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Properties_UV),
                            "vec2", _descUvAnimation(p_Ref), false, false),
          p_Document.GetAllocator());
      p_Properties.AddMember(
          "translucencyThickness",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                            _N(Properties_Translucency), "float",
                            _descTranslucencyThickness(p_Ref), false, false),
          p_Document.GetAllocator());
    }

    // Surface / Foliage properties
    if ((_materialPassMask(p_Ref) &
         (getMaterialPassFlag(_N(GBuffer)) |
          getMaterialPassFlag(_N(GBufferFoliage)) |
          getMaterialPassFlag(_N(GBufferWater)) |
          getMaterialPassFlag(_N(GBufferLayered)))) != 0u)
    {
      p_Properties.AddMember(
          "normalTextureName",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Textures), "string",
                            _descNormalTextureName(p_Ref), false, false),
          p_Document.GetAllocator());
      p_Properties.AddMember(
          "pbrTextureName",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Textures), "string",
                            _descPbrTextureName(p_Ref), false, false),
          p_Document.GetAllocator());

      p_Properties.AddMember(
          "pbrBias",
          _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Properties_Surface),
                            "vec3", _descPbrBias(p_Ref), false, false),
          p_Document.GetAllocator());
    }

    if ((_materialPassMask(p_Ref) & getMaterialPassFlag(_N(GBufferFoliage))) !=
        0u)
    {
      p_Properties.AddMember("blendMaskTextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descBlendMaskTextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
    }

    // Surface (Layered)
    if ((_materialPassMask(p_Ref) & getMaterialPassFlag(_N(GBufferLayered))) !=
        0u)
    {
      p_Properties.AddMember("albedo1TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descAlbedo1TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("normal1TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descNormal1TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("pbr1TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descPbr1TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("albedo2TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descAlbedo2TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("normal2TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descNormal2TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("pbr2TextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descPbr2TextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("blendMaskTextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Layered), "string",
                                               _descBlendMaskTextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
    }

    // Surface (Water)
    if ((_materialPassMask(p_Ref) & getMaterialPassFlag(_N(GBufferWater))) !=
        0u)
    {
      p_Properties.AddMember("foamTextureName",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Textures_Water), "string",
                                               _descFoamTextureName(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("foamFadeDistance",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Properties_Water), "float",
                                               _descFoamFadeDistance(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
      p_Properties.AddMember("refractionFactor",
                             _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                               _N(Properties_Water), "float",
                                               _descRefractionFactor(p_Ref),
                                               false, false),
                             p_Document.GetAllocator());
    }
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(MaterialRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        MaterialData,
        _INTR_MAX_MATERIAL_COUNT>::_initFromDescriptor(p_Ref, p_Properties);

    if (p_Properties.HasMember("albedoTextureName"))
      _descAlbedoTextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["albedoTextureName"]);
    if (p_Properties.HasMember("normalTextureName"))
      _descNormalTextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["normalTextureName"]);
    if (p_Properties.HasMember("pbrTextureName"))
      _descPbrTextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["pbrTextureName"]);
    if (p_Properties.HasMember("albedo1TextureName"))
      _descAlbedo1TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["albedo1TextureName"]);
    if (p_Properties.HasMember("normal1TextureName"))
      _descNormal1TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["normal1TextureName"]);
    if (p_Properties.HasMember("pbr1TextureName"))
      _descPbr1TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["pbr1TextureName"]);
    if (p_Properties.HasMember("albedo2TextureName"))
      _descAlbedo2TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["albedo2TextureName"]);
    if (p_Properties.HasMember("normal2TextureName"))
      _descNormal2TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["normal2TextureName"]);
    if (p_Properties.HasMember("pbr2TextureName"))
      _descPbr2TextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["pbr2TextureName"]);

    if (p_Properties.HasMember("blendMaskTextureName"))
      _descBlendMaskTextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["blendMaskTextureName"]);
    if (p_Properties.HasMember("foamTextureName"))
      _descFoamTextureName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["foamTextureName"]);
    if (p_Properties.HasMember("foamFadeDistance"))
      _descFoamFadeDistance(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["foamFadeDistance"]);
    if (p_Properties.HasMember("refractionFactor"))
      _descRefractionFactor(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["refractionFactor"]);

    if (p_Properties.HasMember("translucencyThickness"))
      _descTranslucencyThickness(p_Ref) =
          JsonHelper::readPropertyFloat(p_Properties["translucencyThickness"]);

    if (p_Properties.HasMember("uvOffsetScale"))
      _descUvOffsetScale(p_Ref) =
          JsonHelper::readPropertyVec4(p_Properties["uvOffsetScale"]);
    if (p_Properties.HasMember("uvAnimation"))
      _descUvAnimation(p_Ref) =
          JsonHelper::readPropertyVec2(p_Properties["uvAnimation"]);
    if (p_Properties.HasMember("pbrBias"))
      _descPbrBias(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["pbrBias"]);

    if (p_Properties.HasMember("materialPassMask"))
    {
      _descMaterialPassMask(p_Ref).clear();
      JsonHelper::readPropertyFlagsNameArray(p_Properties["materialPassMask"],
                                             _descMaterialPassMask(p_Ref));
    }
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<MaterialData,
                                        _INTR_MAX_MATERIAL_COUNT>::
        _saveToMultipleFiles<
            rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<
        MaterialData,
        _INTR_MAX_MATERIAL_COUNT>::_loadFromMultipleFiles(p_Path, p_Extension,
                                                          initFromDescriptor,
                                                          resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const MaterialRefArray& p_Materials);
  static void destroyResources(const MaterialRefArray& p_Materials);

  _INTR_INLINE static uint8_t getMaterialPassId(const Name& p_Name)
  {
    return _materialPassMapping[p_Name];
  }
  _INTR_INLINE static uint32_t getMaterialPassFlag(const Name& p_Name)
  {
    return (1u << getMaterialPassId(p_Name));
  }

  static void loadMaterialPassConfig();

  // Getter/Setter
  // ->

  // Description
  _INTR_INLINE static Name& _descAlbedoTextureName(MaterialRef p_Ref)
  {
    return _data.descAlbedoTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descNormalTextureName(MaterialRef p_Ref)
  {
    return _data.descNormalTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descPbrTextureName(MaterialRef p_Ref)
  {
    return _data.descPbrTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descAlbedo1TextureName(MaterialRef p_Ref)
  {
    return _data.descAlbedo1TextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descNormal1TextureName(MaterialRef p_Ref)
  {
    return _data.descNormal1TextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descPbr1TextureName(MaterialRef p_Ref)
  {
    return _data.descPbr1TextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descAlbedo2TextureName(MaterialRef p_Ref)
  {
    return _data.descAlbedo2TextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descNormal2TextureName(MaterialRef p_Ref)
  {
    return _data.descNormal2TextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descPbr2TextureName(MaterialRef p_Ref)
  {
    return _data.descPbr2TextureName[p_Ref._id];
  }

  _INTR_INLINE static Name& _descBlendMaskTextureName(MaterialRef p_Ref)
  {
    return _data.descBlendMaskTextureName[p_Ref._id];
  }
  _INTR_INLINE static Name& _descFoamTextureName(MaterialRef p_Ref)
  {
    return _data.descFoamTextureName[p_Ref._id];
  }
  _INTR_INLINE static float& _descFoamFadeDistance(MaterialRef p_Ref)
  {
    return _data.descFoamFadeDistance[p_Ref._id];
  }
  _INTR_INLINE static float& _descRefractionFactor(MaterialRef p_Ref)
  {
    return _data.descRefractionFactor[p_Ref._id];
  }

  _INTR_INLINE static glm::vec4& _descUvOffsetScale(MaterialRef p_Ref)
  {
    return _data.descUvOffsetScale[p_Ref._id];
  }
  _INTR_INLINE static glm::vec2& _descUvAnimation(MaterialRef p_Ref)
  {
    return _data.descUvAnimation[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3& _descPbrBias(MaterialRef p_Ref)
  {
    return _data.descPbrBias[p_Ref._id];
  }

  _INTR_INLINE static float& _descTranslucencyThickness(MaterialRef p_Ref)
  {
    return _data.descTranslucencyThickness[p_Ref._id];
  }

  _INTR_INLINE static _INTR_ARRAY(Name) &
      _descMaterialPassMask(MaterialRef p_Ref)
  {
    return _data.descMaterialPassMask[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static uint32_t& _perMaterialDataVertexOffset(MaterialRef p_Ref)
  {
    return _data.perMaterialDataVertexOffset[p_Ref._id];
  }
  _INTR_INLINE static uint32_t&
  _perMaterialDataFragmentOffset(MaterialRef p_Ref)
  {
    return _data.perMaterialDataFragmentOffset[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _materialBufferEntryIndex(MaterialRef p_Ref)
  {
    return _data.materialBufferEntryIndex[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _materialPassMask(MaterialRef p_Ref)
  {
    return _data.materialPassMask[p_Ref._id];
  }

  // <-

  static _INTR_ARRAY(MaterialPass::MaterialPass) _materialPasses;
  static _INTR_ARRAY(Dod::Ref) _materialPassPipelines;
  static _INTR_ARRAY(Dod::Ref) _materialPassPipelineLayouts;
  static _INTR_HASH_MAP(Name, uint8_t) _materialPassMapping;

  static _INTR_ARRAY(MaterialPass::BoundResources) _materialPassBoundResources;
  static _INTR_HASH_MAP(Name, MaterialResourceFunction)
      _materialResourceFunctionMapping;
};
}
}
}
}
