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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
// Global variables
// ->

_INTR_STRING _shaderPath = "assets/shaders/";
_INTR_STRING _shaderCachePath = "media/shaders/";
_INTR_STRING _shaderCacheFilePath = _shaderCachePath + "ShaderCache.json";

class GlslangIncluder : public glslang::TShader::Includer
{
public:
  IncludeResult* includeLocal(const char* p_RequestedSource,
                              const char* p_RequestingSource,
                              size_t p_InclusionDepth) override
  {
    const _INTR_STRING filePath = _shaderPath + p_RequestedSource;

    _INTR_FSTREAM inFileStream =
        _INTR_FSTREAM(filePath.c_str(), std::ios::in | std::ios::binary);
    _INTR_ASSERT(inFileStream);

    _INTR_OSTRINGSTREAM contents;
    contents << inFileStream.rdbuf();
    inFileStream.close();

    const _INTR_STRING sourceStr = contents.str();

    const char* sourceBuffer =
        (const char*)Memory::Tlsf::MainAllocator::allocate(
            (uint32_t)sourceStr.size());
    memcpy((void*)sourceBuffer, sourceStr.c_str(), sourceStr.size());

    IncludeResult result = {p_RequestedSource, sourceBuffer, sourceStr.size(),
                            nullptr};
    return new IncludeResult(result);
  }

  virtual void releaseInclude(IncludeResult* result) override
  {
    Memory::Tlsf::MainAllocator::free((void*)result->headerData);
    delete result;
  }
} _includer;

TBuiltInResource _defaultResource;
rapidjson::Document _shaderCache = rapidjson::Document(rapidjson::kObjectType);
// <-

bool isShaderUpToDate(uint32_t p_ShaderHash, const char* p_GpuProgramName)
{
  if (_shaderCache.HasMember(p_GpuProgramName))
  {
    return _shaderCache[p_GpuProgramName].GetUint() == p_ShaderHash;
  }

  return false;
}

void loadShaderCache()
{
  FILE* fp = fopen(_shaderCacheFilePath.c_str(), "rb");

  if (fp == nullptr)
  {
    _INTR_LOG_WARNING("Shader cache not available...");
    return;
  }

  char* readBuffer = (char*)Memory::Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileReadStream is(fp, readBuffer, 65536u);
    _shaderCache.ParseStream(is);
    fclose(fp);
  }
  Memory::Tlsf::MainAllocator::free(readBuffer);
}

void saveShaderCache()
{
  FILE* fp = fopen(_shaderCacheFilePath.c_str(), "wb");

  if (fp == nullptr)
  {
    _INTR_LOG_ERROR("Failed to save shader cache...");
    return;
  }

  char* writeBuffer = (char*)Memory::Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileWriteStream os(fp, writeBuffer, 65536u);
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    _shaderCache.Accept(writer);
    fclose(fp);
  }
  Memory::Tlsf::MainAllocator::free(writeBuffer);
}

void addShaderToCache(uint32_t p_ShaderHash, const char* p_GpuProgramName,
                      const SpirvBuffer& p_SpirvBuffer)
{
  if (_shaderCache.HasMember(p_GpuProgramName))
  {
    _shaderCache[p_GpuProgramName].SetUint(p_ShaderHash);
  }
  else
  {
    rapidjson::Value fileName =
        rapidjson::Value(p_GpuProgramName, _shaderCache.GetAllocator());
    _shaderCache.AddMember(fileName, p_ShaderHash, _shaderCache.GetAllocator());
  }

  _INTR_STRING cacheFileName = _shaderCachePath + p_GpuProgramName + ".cached";
  _INTR_OFSTREAM of(cacheFileName.c_str(), std::ofstream::binary);
  of.write((const char*)p_SpirvBuffer.data(),
           p_SpirvBuffer.size() * sizeof(uint32_t));
  of.close();

  saveShaderCache();
}

void loadShaderFromCache(const char* p_GpuProgranName,
                         SpirvBuffer& p_SpirvBuffer)
{
  _INTR_STRING cacheFileName = _shaderCachePath + p_GpuProgranName + ".cached";
  _INTR_IFSTREAM ifs(cacheFileName.c_str(), std::ifstream::binary);

  ifs.seekg(0, ifs.end);
  std::streamoff size = ifs.tellg();
  ifs.seekg(0);

  if (size != -1)
  {
    p_SpirvBuffer.resize((size_t)size / sizeof(uint32_t));
    ifs.read((char*)p_SpirvBuffer.data(), size);
  }
  else
  {
    _INTR_LOG_WARNING("Shader cache file '%s' not found!",
                      cacheFileName.c_str());
  }

  ifs.close();
}

void GpuProgramManager::init()
{
  _INTR_LOG_INFO("Inititializing GPU Program Manager...");

  Dod::Resources::ResourceManagerBase<
      GpuProgramData, _INTR_MAX_GPU_PROGRAM_COUNT>::_initResourceManager();

  {
    Dod::Resources::ResourceManagerEntry managerEntry;
    managerEntry.createFunction =
        Resources::GpuProgramManager::createGpuProgram;
    managerEntry.destroyFunction =
        Resources::GpuProgramManager::destroyGpuProgram;
    managerEntry.createResourcesFunction =
        Resources::GpuProgramManager::createResources;
    managerEntry.destroyResourcesFunction =
        Resources::GpuProgramManager::destroyResources;
    managerEntry.getActiveResourceAtIndexFunction =
        Resources::GpuProgramManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        Resources::GpuProgramManager::getActiveResourceCount;
    managerEntry.loadFromMultipleFilesFunction =
        Resources::GpuProgramManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction =
        Resources::GpuProgramManager::saveToMultipleFiles;
    managerEntry.getResourceFlagsFunction = GpuProgramManager::_resourceFlags;
    Application::_resourceManagerMapping[_N(GpuProgram)] = managerEntry;
  }

  {
    Dod::PropertyCompilerEntry compilerEntry;
    compilerEntry.compileFunction =
        Resources::GpuProgramManager::compileDescriptor;
    compilerEntry.initFunction =
        Resources::GpuProgramManager::initFromDescriptor;
    compilerEntry.ref = Dod::Ref();
    Application::_resourcePropertyCompilerMapping[_N(GpuProgram)] =
        compilerEntry;
  }

  glslang::InitializeProcess();
  Helper::initResource(_defaultResource);
  loadShaderCache();
}

void GpuProgramManager::reflectPipelineLayout(
    uint32_t p_PoolCount, const Dod::RefArray& p_GpuPrograms,
    Dod::Ref p_PipelineLayoutToInit)
{

  _INTR_ARRAY(BindingDescription) bindingDecs;

  for (uint32_t i = 0u; i < p_GpuPrograms.size(); ++i)
  {
    GpuProgramRef gpuProgramRef = p_GpuPrograms[i];

    spirv_cross::CompilerGLSL glsl(
        GpuProgramManager::_spirvBuffer(gpuProgramRef));
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    // Uniform buffers
    for (uint32_t i = 0u; i < resources.uniform_buffers.size(); ++i)
    {
      spirv_cross::Resource& res = resources.uniform_buffers[i];

      if (glsl.get_decoration(res.id, spv::DecorationDescriptorSet) != 0u)
        continue;

      const bool isDynamic = res.name == "PerInstance" ||
                             res.name == "PerMaterial" ||
                             res.name == "PerFrame";

      BindingDescription bd;
      {
        bd.name = res.name.c_str();
        bd.bindingType = isDynamic ? BindingType::kUniformBufferDynamic
                                   : BindingType::kUniformBuffer;
        bd.binding = glsl.get_decoration(res.id, spv::DecorationBinding);
        bd.poolCount = p_PoolCount;
        bd.shaderStage = GpuProgramManager::_descGpuProgramType(gpuProgramRef);
      }

      bindingDecs.push_back(bd);
    }

    // Images
    for (uint32_t i = 0u; i < resources.sampled_images.size(); ++i)
    {
      spirv_cross::Resource& res = resources.sampled_images[i];

      if (glsl.get_decoration(res.id, spv::DecorationDescriptorSet) != 0u)
        continue;

      BindingDescription bd;
      {
        bd.name = res.name.c_str();
        bd.bindingType = BindingType::kImageAndSamplerCombined;
        bd.binding = glsl.get_decoration(res.id, spv::DecorationBinding);
        bd.poolCount = p_PoolCount;
        bd.shaderStage = GpuProgramManager::_descGpuProgramType(gpuProgramRef);
      }

      bindingDecs.push_back(bd);
    }

    // Storage Images
    for (uint32_t i = 0u; i < resources.storage_images.size(); ++i)
    {
      spirv_cross::Resource& res = resources.storage_images[i];

      if (glsl.get_decoration(res.id, spv::DecorationDescriptorSet) != 0u)
        continue;

      BindingDescription bd;
      {
        bd.name = res.name.c_str();
        bd.bindingType = BindingType::kStorageImage;
        bd.binding = glsl.get_decoration(res.id, spv::DecorationBinding);
        bd.poolCount = p_PoolCount;
        bd.shaderStage = GpuProgramManager::_descGpuProgramType(gpuProgramRef);
      }

      bindingDecs.push_back(bd);
    }

    // Storage Buffers
    for (uint32_t i = 0u; i < resources.storage_buffers.size(); ++i)
    {
      spirv_cross::Resource& res = resources.storage_buffers[i];

      if (glsl.get_decoration(res.id, spv::DecorationDescriptorSet) != 0u)
        continue;

      BindingDescription bd;
      {
        bd.name = res.name.c_str();
        bd.bindingType = BindingType::kStorageBuffer;
        bd.binding = glsl.get_decoration(res.id, spv::DecorationBinding);
        bd.poolCount = p_PoolCount;
        bd.shaderStage = GpuProgramManager::_descGpuProgramType(gpuProgramRef);
      }

      bindingDecs.push_back(bd);
    }
  }

  std::sort(bindingDecs.begin(), bindingDecs.end(),
            [](const BindingDescription& p_Left,
               const BindingDescription& p_Right) -> bool {
              return p_Left.binding < p_Right.binding;
            });

  PipelineLayoutManager::_descBindingDescs(p_PipelineLayoutToInit) =
      std::move(bindingDecs);
}

void GpuProgramManager::compileShaders(GpuProgramRefArray p_Refs,
                                       bool p_ForceRecompile,
                                       bool p_UpdateResources)
{
  _INTR_LOG_INFO("Loading/Compiling GPU Programs...");

  GpuProgramRefArray changedGpuPrograms;

  for (uint32_t gpIdx = 0u; gpIdx < p_Refs.size(); ++gpIdx)
  {
    GpuProgramRef ref = p_Refs[gpIdx];
    const _INTR_STRING gpuProgramName =
        "gpu_program_" + StringUtil::toString(_name(ref)._hash);

    SpirvBuffer& spirvBuffer = _spirvBuffer(ref);
    spirvBuffer.clear();

    EShLanguage stage = Helper::mapGpuProgramTypeToEshLang(
        (GpuProgramType::Enum)_descGpuProgramType(ref));
    glslang::TShader shader(stage);
    glslang::TProgram program;

    const EShMessages messages =
        (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    const _INTR_STRING& fileName = _descGpuProgramName(ref);
    _INTR_STRING filePath = _shaderPath + fileName;

    _INTR_FSTREAM inFileStream =
        _INTR_FSTREAM(filePath.c_str(), std::ios::in | std::ios::binary);
    _INTR_STRING glslString;
    uint32_t shaderHash = 0u;

    if (inFileStream)
    {
      _INTR_OSTRINGSTREAM contents;
      contents << inFileStream.rdbuf();
      inFileStream.close();
      glslString = contents.str();

      // Add pre processor defines
      {
        _INTR_ARRAY(_INTR_STRING) preProcessorDefines;
        StringUtil::split(_descPreprocessorDefines(ref), ";",
                          preProcessorDefines);

        _INTR_STRING defineStr;
        for (uint32_t i = 0u; i < preProcessorDefines.size(); ++i)
        {
          _INTR_LOG_INFO("Adding preprocessor #define '%s'...",
                         preProcessorDefines[i].c_str());
          defineStr += preProcessorDefines[i] + "\n";
        }

        StringUtil::replace(glslString, "/* __PREPROCESSOR DEFINES__ */",
                            defineStr);
      }

      shaderHash =
          Math::hash(glslString.c_str(), sizeof(char) * glslString.length());

      if (!p_ForceRecompile)
      {
        if (isShaderUpToDate(shaderHash, gpuProgramName.c_str()))
        {
          loadShaderFromCache(gpuProgramName.c_str(), spirvBuffer);
          continue;
        }
      }
    }
    else
    {
      _INTR_LOG_WARNING(
          "Shader for GPU program '%s' not found, trying to load from cache...",
          _descGpuProgramName(ref).c_str());
      loadShaderFromCache(gpuProgramName.c_str(), spirvBuffer);
      continue;
    }

    _INTR_LOG_INFO("Compiling GPU program '%s'...",
                   _descGpuProgramName(ref).c_str());

    const char* glslStringChar = glslString.c_str();
    shader.setStrings(&glslStringChar, 1);

    if (!shader.parse(&_defaultResource, 100, ECoreProfile, false, false,
                      messages, _includer))
    {
      _INTR_LOG_WARNING("Parsing of GPU program failed...");
      _INTR_LOG_WARNING(shader.getInfoLog());
      _INTR_LOG_WARNING(shader.getInfoDebugLog());

      // Try to load the previous shader from the cache
      loadShaderFromCache(gpuProgramName.c_str(), spirvBuffer);
      continue;
    }

    program.addShader(&shader);

    if (!program.link(messages))
    {
      _INTR_LOG_WARNING("Linking of GPU program failed...");
      _INTR_LOG_WARNING(shader.getInfoLog());
      _INTR_LOG_WARNING(shader.getInfoDebugLog());

      // Try to load the previous shader from the cache
      loadShaderFromCache(gpuProgramName.c_str(), spirvBuffer);
      continue;
    }

    _INTR_LOG_WARNING(shader.getInfoLog());
    _INTR_LOG_WARNING(shader.getInfoDebugLog());

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirvBuffer);
    addShaderToCache(shaderHash, gpuProgramName.c_str(), spirvBuffer);

    changedGpuPrograms.push_back(ref);
  }

  // Update all pipelines which reference this GPU program
  PipelineRefArray changedPipelines;
  for (uint32_t gpIdx = 0u; gpIdx < changedGpuPrograms.size(); ++gpIdx)
  {
    GpuProgramRef ref = changedGpuPrograms[gpIdx];

    for (uint32_t pipIdx = 0u;
         pipIdx < Resources::PipelineManager::getActiveResourceCount();
         ++pipIdx)
    {
      Resources::PipelineRef pipelineRef =
          Resources::PipelineManager::getActiveResourceAtIndex(pipIdx);

      if (Resources::PipelineManager::_descVertexProgram(pipelineRef) == ref ||
          Resources::PipelineManager::_descFragmentProgram(pipelineRef) ==
              ref ||
          Resources::PipelineManager::_descGeometryProgram(pipelineRef) ==
              ref ||
          Resources::PipelineManager::_descComputeProgram(pipelineRef) == ref)
      {
        changedPipelines.push_back(pipelineRef);
      }
    }
  }

  if (p_UpdateResources)
  {
    PipelineManager::destroyResources(changedPipelines);
    GpuProgramManager::destroyResources(changedGpuPrograms);

    GpuProgramManager::createResources(changedGpuPrograms);
    PipelineManager::createResources(changedPipelines);
  }
}

void GpuProgramManager::compileShader(GpuProgramRef p_Ref,
                                      bool p_ForceRecompile,
                                      bool p_UpdateResources)
{
  GpuProgramRefArray refs = {p_Ref};
  compileShaders(refs, p_ForceRecompile, p_UpdateResources);
}

void GpuProgramManager::compileAllShaders(bool p_ForceRecompile,
                                          bool p_UpdateResources)
{
  compileShaders(_activeRefs, p_ForceRecompile, p_UpdateResources);
}

// <-

void GpuProgramManager::createResources(const GpuProgramRefArray& p_Refs)
{
  for (uint32_t i = 0u; i < p_Refs.size(); ++i)
  {
    GpuProgramRef ref = p_Refs[i];
    const SpirvBuffer& spirvBuffer = _spirvBuffer(ref);

    // No spirv buffer? Retrieve it from the cache or compile the shader
    if (spirvBuffer.empty())
    {
      compileShader(ref, false, false);
      if (spirvBuffer.empty())
      {
        continue;
      }
    }

    VkShaderModule& module = _vkShaderModule(ref);

    // Shader module
    {
      VkShaderModuleCreateInfo creationInfo;
      creationInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      creationInfo.pNext = nullptr;
      creationInfo.flags = 0;
      creationInfo.codeSize = spirvBuffer.size() * sizeof(uint32_t);
      creationInfo.pCode = spirvBuffer.data();

      _INTR_ASSERT(module == VK_NULL_HANDLE);
      VkResult result = vkCreateShaderModule(RenderSystem::_vkDevice,
                                             &creationInfo, nullptr, &module);
      _INTR_VK_CHECK_RESULT(result);
    }

    // Pipeline state
    {
      VkPipelineShaderStageCreateInfo& pipelineCreateInfo =
          _vkPipelineShaderStageCreateInfo(ref);
      pipelineCreateInfo.sType =
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      pipelineCreateInfo.pNext = nullptr;
      pipelineCreateInfo.pSpecializationInfo = nullptr;
      pipelineCreateInfo.flags = 0u;
      pipelineCreateInfo.stage = Helper::mapGpuProgramTypeToVkShaderStage(
          (GpuProgramType::Enum)_descGpuProgramType(ref));
      pipelineCreateInfo.pName = _descEntryPoint(ref).c_str();
      pipelineCreateInfo.module = module;
    }
  }
}

// <-
}
}
}
}
