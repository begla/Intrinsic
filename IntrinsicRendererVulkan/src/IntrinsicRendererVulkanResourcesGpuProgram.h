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
// Typedefs
typedef Dod::Ref GpuProgramRef;
typedef Dod::RefArray GpuProgramRefArray;

typedef std::vector<uint32_t> SpirvBuffer;

struct GpuProgramData : Dod::Resources::ResourceDataBase
{
  GpuProgramData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_GPU_PROGRAM_COUNT)
  {
    descGpuProgramName.resize(_INTR_MAX_GPU_PROGRAM_COUNT);
    descEntryPoint.resize(_INTR_MAX_GPU_PROGRAM_COUNT);
    descGpuProgramType.resize(_INTR_MAX_GPU_PROGRAM_COUNT);

    spirvBuffer.resize(_INTR_MAX_GPU_PROGRAM_COUNT);
    vkShaderModule.resize(_INTR_MAX_GPU_PROGRAM_COUNT);
    vkPipelineShaderStageCreateInfo.resize(_INTR_MAX_GPU_PROGRAM_COUNT);
  }

  _INTR_ARRAY(_INTR_STRING) descGpuProgramName;
  _INTR_ARRAY(_INTR_STRING) descEntryPoint;
  _INTR_ARRAY(uint8_t) descGpuProgramType;

  _INTR_ARRAY(SpirvBuffer) spirvBuffer;
  _INTR_ARRAY(VkShaderModule) vkShaderModule;
  _INTR_ARRAY(VkPipelineShaderStageCreateInfo) vkPipelineShaderStageCreateInfo;
};

struct GpuProgramManager
    : Dod::Resources::ResourceManagerBase<GpuProgramData,
                                          _INTR_MAX_GPU_PROGRAM_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static GpuProgramRef createGpuProgram(const Name& p_Name)
  {
    GpuProgramRef ref = Dod::Resources::ResourceManagerBase<
        GpuProgramData, _INTR_MAX_GPU_PROGRAM_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(GpuProgramRef p_Ref)
  {
    _descGpuProgramName(p_Ref) = "";
    _descEntryPoint(p_Ref) = "";
    _descGpuProgramType(p_Ref) = GpuProgramType::kVertex;
    _spirvBuffer(p_Ref).clear();
  }

  // <-

  _INTR_INLINE static void destroyGpuProgram(GpuProgramRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        GpuProgramData, _INTR_MAX_GPU_PROGRAM_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(GpuProgramRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        GpuProgramData,
        _INTR_MAX_GPU_PROGRAM_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                         p_Properties,
                                                         p_Document);

    p_Properties.AddMember(
        "gpuProgramName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(GpuProgram),
                          _N(string), _descGpuProgramName(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "entryPoint",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(GpuProgram),
                          _N(string), _descEntryPoint(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "gpuProgramType", _INTR_CREATE_PROP_ENUM(
                              p_Document, p_GenerateDesc, _N(GpuProgram),
                              _N(enum), _descGpuProgramType(p_Ref),
                              "Vertex,Fragment,Geometry,Compute", false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(GpuProgramRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        GpuProgramData,
        _INTR_MAX_GPU_PROGRAM_COUNT>::_initFromDescriptor(p_Ref, p_Properties);

    if (p_Properties.HasMember("gpuProgramName"))
      _descGpuProgramName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["gpuProgramName"]);
    if (p_Properties.HasMember("entryPoint"))
      _descEntryPoint(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["entryPoint"]);
    if (p_Properties.HasMember("gpuProgramType"))
      _descGpuProgramType(p_Ref) =
          (GpuProgramType::Enum)JsonHelper::readPropertyEnum(
              p_Properties["gpuProgramType"]);
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
    const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<GpuProgramData, _INTR_MAX_GPU_PROGRAM_COUNT>::
      _saveToMultipleFiles<rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
        p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
    const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<GpuProgramData, _INTR_MAX_GPU_PROGRAM_COUNT>::
      _loadFromMultipleFiles(p_Path, p_Extension, initFromDescriptor,
        resetToDefault);
  }

  // <-

  static void compileShaders(GpuProgramRefArray p_Refs);
  static void compileShader(GpuProgramRef p_Ref);
  static void compileAllShaders();

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  _INTR_INLINE static void destroyAllGpuResources()
  {
    destroyResources(_activeRefs);
  }

  // <-

  _INTR_INLINE static void destroyResources(const GpuProgramRefArray& p_Refs)
  {
    for (uint32_t i = 0u; i < p_Refs.size(); ++i)
    {
      GpuProgramRef ref = p_Refs[i];
      VkShaderModule shaderModule = _vkShaderModule(ref);

      if (shaderModule != VK_NULL_HANDLE)
      {
        vkDestroyShaderModule(RenderSystem::_vkDevice, shaderModule, nullptr);

        VkShaderModule nullModule = VK_NULL_HANDLE;
        _vkShaderModule(ref) = nullModule;
      }
    }
  }

  // <-

  static void createResources(const GpuProgramRefArray& p_Refs);

  // <-

  static void reflectPipelineLayout(uint32_t p_PoolCount,
                                    const Dod::RefArray& p_GpuPrograms,
                                    Dod::Ref p_PipelineLayoutToInit);

  // Member refs
  // ->

  // Description
  _INTR_INLINE static _INTR_STRING& _descGpuProgramName(GpuProgramRef p_Ref)
  {
    return _data.descGpuProgramName[p_Ref._id];
  }
  _INTR_INLINE static _INTR_STRING& _descEntryPoint(GpuProgramRef p_Ref)
  {
    return _data.descEntryPoint[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descGpuProgramType(GpuProgramRef p_Ref)
  {
    return _data.descGpuProgramType[p_Ref._id];
  }

  // Int. resources
  _INTR_INLINE static SpirvBuffer& _spirvBuffer(GpuProgramRef p_Ref)
  {
    return _data.spirvBuffer[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkShaderModule& _vkShaderModule(GpuProgramRef p_Ref)
  {
    return _data.vkShaderModule[p_Ref._id];
  }
  _INTR_INLINE static VkPipelineShaderStageCreateInfo&
  _vkPipelineShaderStageCreateInfo(GpuProgramRef p_Ref)
  {
    return _data.vkPipelineShaderStageCreateInfo[p_Ref._id];
  }

  // <-
};
}
}
}
}
