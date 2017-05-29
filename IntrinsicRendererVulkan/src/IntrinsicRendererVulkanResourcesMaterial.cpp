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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

// Lib. includes
#include <gli/gli.hpp>

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
// Static members
_INTR_ARRAY(MaterialPass::MaterialPass) MaterialManager::_materialPasses;
_INTR_ARRAY(MaterialPass::BoundResources)
MaterialManager::_materialPassBoundResources;
_INTR_HASH_MAP(Name, MaterialResourceFunction)
MaterialManager::_materialResourceFunctionMapping = {
    {_N(MaterialAlbedo), MaterialManager::_descAlbedoTextureName},
    {_N(MaterialEmissive), MaterialManager::_descEmissiveTextureName},
    {_N(MaterialNormal), MaterialManager::_descNormalTextureName},
    {_N(MaterialRoughness), MaterialManager::_descPbrTextureName},
    {_N(MaterialFoam), MaterialManager::_descFoamTextureName},
    {_N(MaterialAlbedo1), MaterialManager::_descAlbedo1TextureName},
    {_N(MaterialAlbedo2), MaterialManager::_descAlbedo2TextureName},
    {_N(MaterialNormal1), MaterialManager::_descNormal1TextureName},
    {_N(MaterialNormal2), MaterialManager::_descNormal2TextureName},
    {_N(MaterialRoughness1), MaterialManager::_descPbr1TextureName},
    {_N(MaterialRoughness2), MaterialManager::_descPbr2TextureName},
    {_N(MaterialBlendMask), MaterialManager::_descBlendMaskTextureName}};
_INTR_HASH_MAP(Name, uint8_t) MaterialManager::_materialPassMapping;
_INTR_ARRAY(Dod::Ref) MaterialManager::_materialPassPipelines;
_INTR_ARRAY(Dod::Ref) MaterialManager::_materialPassPipelineLayouts;

// <-

void MaterialManager::init()
{
  _INTR_LOG_INFO("Inititializing Material Manager...");

  Dod::Resources::ResourceManagerBase<
      MaterialData, _INTR_MAX_MATERIAL_COUNT>::_initResourceManager();

  {
    Dod::Resources::ResourceManagerEntry managerEntry;
    managerEntry.createFunction = MaterialManager::createMaterial;
    managerEntry.destroyFunction = MaterialManager::destroyMaterial;
    managerEntry.createResourcesFunction = MaterialManager::createResources;
    managerEntry.destroyResourcesFunction = MaterialManager::destroyResources;
    managerEntry.getActiveResourceAtIndexFunction =
        MaterialManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        MaterialManager::getActiveResourceCount;
    managerEntry.loadFromMultipleFilesFunction =
        MaterialManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction =
        MaterialManager::saveToMultipleFiles;
    managerEntry.getResourceFlagsFunction = MaterialManager::_resourceFlags;
    Application::_resourceManagerMapping[_N(Material)] = managerEntry;
  }

  {
    Dod::PropertyCompilerEntry compilerEntry;
    compilerEntry.compileFunction = MaterialManager::compileDescriptor;
    compilerEntry.initFunction = MaterialManager::initFromDescriptor;
    compilerEntry.ref = Dod::Ref();
    Application::_resourcePropertyCompilerMapping[_N(Material)] = compilerEntry;
  }

  _defaultResourceName = _N(default);
}

void MaterialManager::createResources(const MaterialRefArray& p_Materiales)
{
  Components::MeshRefArray componentsToRecreate;

  for (uint32_t i = 0u; i < p_Materiales.size(); ++i)
  {
    MaterialRef matRef = p_Materiales[i];

    _perMaterialDataVertexOffset(matRef) =
        UniformManager::allocatePerMaterialDataMemory();
    _perMaterialDataFragmentOffset(matRef) =
        UniformManager::allocatePerMaterialDataMemory();

    // Update the average normal length
    ImageRef normalRef =
        ImageManager::_getResourceByName(_descNormalTextureName(matRef));

    float avgNormalLength = 1.0f;
    // if (normalRef.isValid())
    //{
    //  _INTR_STRING texturePath =
    //      "media/textures/" + ImageManager::_descFileName(normalRef);

    //  gli::texture2d normalTexture =
    //      gli::texture2d(gli::load(texturePath.c_str()));
    //  gli::texture2d normalTexDec =
    //      gli::convert(normalTexture, gli::FORMAT_RGB32_SFLOAT_PACK32);

    //  glm::vec3 avgNormal = glm::vec3(0.0f);
    //  for (int32_t y = 0u; y < normalTexDec.extent().y; ++y)
    //  {
    //    for (int32_t x = 0u; x < normalTexDec.extent().x; ++x)
    //    {
    //      const gli::vec3 normal =
    //      gli::normalize(normalTexDec.load<gli::vec3>(
    //                                   gli::extent2d(x, y), 0u)) *
    //                                   2.0f -
    //                               1.0f;
    //      avgNormal += normal;
    //    }
    //  }

    // avgNormal /= normalTexDec.extent().x * normalTexDec.extent().y;
    //  avgNormalLength = glm::length(avgNormal);
    //}

    // Update material pass flags
    {
      uint32_t& materialPassMask = _materialPassMask(matRef);
      materialPassMask = 0u;

      const _INTR_ARRAY(Name)& materialPassMaskNames =
          _descMaterialPassMask(matRef);
      for (uint32_t i = 0u; i < materialPassMaskNames.size(); ++i)
      {
        for (uint32_t materialPassIdx = 0u;
             materialPassIdx < _materialPasses.size(); ++materialPassIdx)
        {
          if (materialPassMaskNames[i] == _materialPasses[materialPassIdx].name)
          {
            materialPassMask |= 1u << materialPassIdx;
            break;
          }
        }
      }
    }

    uint32_t& materialBufferEntryIdx = _materialBufferEntryIndex(matRef);
    {
      materialBufferEntryIdx = MaterialBuffer::allocateMaterialBufferEntry();

      MaterialBufferEntry entry = {};
      {
        entry.refractionFactor = _descRefractionFactor(matRef);
        entry.translucencyThicknessFactor = _descTranslucencyThickness(matRef);
        entry.emissveIntensity = _descEmissiveIntensity(matRef);
      }
      MaterialBuffer::updateMaterialBufferEntry(materialBufferEntryIdx, entry);
    }

    {
      PerMaterialDataVertex vtxData;
      vtxData._dummy = 42.0f;

      UniformManager::updatePerMaterialDataMemory(
          &vtxData, sizeof(PerMaterialDataVertex),
          _perMaterialDataVertexOffset(matRef));
    }

    {
      PerMaterialDataFragment fragmentData;
      fragmentData.uvOffsetScale = _descUvOffsetScale(matRef);
      fragmentData.waterParams = _descWaterParams(matRef);
      fragmentData.pbrBias = glm::vec4(_descPbrBias(matRef), 0.0f);
      fragmentData.uvAnimation =
          glm::vec4(_descUvAnimation(matRef), 0.0f, 0.0f);
      fragmentData.data0.x = materialBufferEntryIdx;
      fragmentData.data1.x = avgNormalLength;

      UniformManager::updatePerMaterialDataMemory(
          &fragmentData, sizeof(PerMaterialDataFragment),
          _perMaterialDataFragmentOffset(matRef));
    }

    for (uint32_t i = 0u;
         i < Core::Components::MeshManager::getActiveResourceCount(); ++i)
    {
      Components::MeshRef meshCompRef =
          Components::MeshManager::getActiveResourceAtIndex(i);
      Core::Resources::MeshRef meshRef =
          Core::Resources::MeshManager::getResourceByName(
              Components::MeshManager::_descMeshName(meshCompRef));

      Core::Resources::MaterialNamesPerSubMeshArray& materialNamesPerSubMesh =
          Core::Resources::MeshManager::_descMaterialNamesPerSubMesh(meshRef);
      const uint32_t subMeshCount = (uint32_t)materialNamesPerSubMesh.size();

      for (uint32_t i = 0u; i < subMeshCount; ++i)
      {
        if (materialNamesPerSubMesh[i] == _name(matRef))
        {
          componentsToRecreate.push_back(meshCompRef);
        }
      }
    }
  }

  Core::Components::MeshManager::destroyResources(componentsToRecreate);
  Core::Components::MeshManager::createResources(componentsToRecreate);
}

// <-

void MaterialManager::destroyResources(const MaterialRefArray& p_Materials)
{
  Dod::RefArray pipelinesToDestroy;
  Dod::RefArray pipelineLayoutsToDestroy;

  for (uint32_t i = 0u; i < p_Materials.size(); ++i)
  {
    MaterialRef matRef = p_Materials[i];

    uint32_t& vertOffset = _perMaterialDataVertexOffset(matRef);
    if (vertOffset != (uint32_t)-1)
    {
      UniformManager::freePerMaterialDataMemory(vertOffset);
      vertOffset = (uint32_t)-1;
    }

    uint32_t& fragOffset = _perMaterialDataFragmentOffset(matRef);
    if (fragOffset != (uint32_t)-1)
    {
      UniformManager::freePerMaterialDataMemory(fragOffset);
      fragOffset = (uint32_t)-1;
    }

    uint32_t& materialBufferEntryIdx = _materialBufferEntryIndex(matRef);
    if (materialBufferEntryIdx != (uint32_t)-1)
    {
      MaterialBuffer::freeMaterialBufferEntry(materialBufferEntryIdx);
      materialBufferEntryIdx = (uint32_t)-1;
    }
  }
}

// <-

void MaterialManager::loadMaterialPassConfig()
{
  const _INTR_STRING materialPassConfigFilePath =
      "config/" + Settings::Manager::_materialPassConfig;

  rapidjson::Document materialPassConfig;
  {
    FILE* fp = fopen(materialPassConfigFilePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_WARNING("Failed to load renderer config from file '%s'...",
                        Settings::Manager::_rendererConfig.c_str());
      return;
    }

    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      materialPassConfig.ParseStream<rapidjson::kParseCommentsFlag>(is);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(readBuffer);
  }

  _INTR_LOG_INFO("Loading material pass config '%s'...",
                 materialPassConfig["name"].GetString());

  // Cleanup existing material pass data
  _materialPasses.clear();
  _materialPassBoundResources.clear();
  _materialPassMapping.clear();

  for (uint32_t matPassIdx = 0u; matPassIdx < _materialPasses.size();
       ++matPassIdx)
  {
    for (uint32_t i = 0u; i < _INTR_MAX_FRUSTUMS_PER_FRAME_COUNT; ++i)
      RenderProcess::Default::_visibleDrawCallsPerMaterialPass[i][matPassIdx]
          .clear();
  }

  Resources::PipelineManager::destroyPipelinesAndResources(
      _materialPassPipelines);
  _materialPassPipelines.clear();
  Resources::PipelineLayoutManager::destroyPipelineLayoutsAndResources(
      _materialPassPipelineLayouts);
  _materialPassPipelineLayouts.clear();

  // Load bound resources
  {
    rapidjson::Value& boundResourcesDesc = materialPassConfig["boundResources"];

    for (uint32_t boundResourcesIdx = 0u;
         boundResourcesIdx < boundResourcesDesc.Size(); ++boundResourcesIdx)
    {
      rapidjson::Value& boundResourceDesc =
          boundResourcesDesc[boundResourcesIdx];
      rapidjson::Value& resourceEntriesDesc = boundResourceDesc["resources"];

      MaterialPass::BoundResources boundResources = {};
      {
        boundResources.name = boundResourceDesc["name"].GetString();

        for (uint32_t resourceEntryIdx = 0u;
             resourceEntryIdx < resourceEntriesDesc.Size(); ++resourceEntryIdx)
        {
          rapidjson::Value& resourceEntryDesc =
              resourceEntriesDesc[resourceEntryIdx];

          MaterialPass::BoundResourceEntry entry = {};
          {
            entry.resourceName = resourceEntryDesc[1].GetString();
            entry.shaderStage =
                Helper::mapGpuProgramType(resourceEntryDesc[3].GetString());
            entry.slotName = resourceEntryDesc[2].GetString();

            if (resourceEntryDesc[0] == "Buffer")
              entry.type = MaterialPass::BoundResourceType::kBuffer;
            else if (resourceEntryDesc[0] == "Image")
              entry.type = MaterialPass::BoundResourceType::kImage;
            else
              _INTR_ASSERT(false && "Bound resource type unknown");
          }

          boundResources.boundResourceEntries.push_back(entry);
        }
      }

      _materialPassBoundResources.push_back(std::move(boundResources));
    }
  }

  // Load material passes
  {
    rapidjson::Value& materialPassesDesc = materialPassConfig["materialPasses"];

    for (uint32_t i = 0u; i < materialPassesDesc.Size(); ++i)
    {
      rapidjson::Value& materialPassDesc = materialPassesDesc[i];
      const Name materialPassName = materialPassDesc["name"].GetString();

      MaterialPass::MaterialPass matPass = {};
      matPass.name = materialPassName;

      {
        RenderPassRef renderPassRef = RenderPassManager::_getResourceByName(
            materialPassDesc["renderPass"].GetString());

        if (!renderPassRef.isValid())
        {
          _INTR_LOG_WARNING(
              "Render pass '%s' not found, skipping material pass '%s'...",
              materialPassDesc["renderPass"].GetString(),
              materialPassDesc["name"].GetString());
          _materialPasses.push_back(MaterialPass::MaterialPass());
          continue;
        }

        for (uint32_t i = 0u; i < _materialPassBoundResources.size(); ++i)
        {
          if (_materialPassBoundResources[i].name ==
              materialPassDesc["boundResources"].GetString())
          {
            matPass.boundResoucesIdx = i;
            break;
          }
        }

        // Pipeline layout
        Resources::PipelineLayoutRef pipelineLayoutRef;
        {
          pipelineLayoutRef =
              PipelineLayoutManager::createPipelineLayout(materialPassName);
          PipelineLayoutManager::resetToDefault(pipelineLayoutRef);

          GpuProgramRefArray programsToReflect;
          if (materialPassDesc.HasMember("baseFragmentGpuProgram"))
          {
            GpuProgramRef fragmentProgram =
                Resources::GpuProgramManager::getResourceByName(
                    materialPassDesc["baseFragmentGpuProgram"].GetString());
            programsToReflect.push_back(fragmentProgram);
          }
          if (materialPassDesc.HasMember("baseVertexGpuProgram"))
          {
            GpuProgramRef vertexProgram =
                Resources::GpuProgramManager::getResourceByName(
                    materialPassDesc["baseVertexGpuProgram"].GetString());
            programsToReflect.push_back(vertexProgram);
          }

          GpuProgramManager::reflectPipelineLayout(
              _INTR_MAX_DRAW_CALL_COUNT, programsToReflect, pipelineLayoutRef);

          _materialPassPipelineLayouts.push_back(pipelineLayoutRef);
          matPass.pipelineLayoutIdx =
              (uint8_t)(_materialPassPipelineLayouts.size() - 1u);
        }

        // Pipeline
        PipelineRef pipelineRef;
        {
          pipelineRef = PipelineManager::createPipeline(materialPassName);
          PipelineManager::resetToDefault(pipelineRef);

          if (materialPassDesc.HasMember("baseFragmentGpuProgram"))
          {
            PipelineManager::_descFragmentProgram(pipelineRef) =
                GpuProgramManager::getResourceByName(
                    materialPassDesc["baseFragmentGpuProgram"].GetString());
          }
          if (materialPassDesc.HasMember("baseVertexGpuProgram"))
          {
            PipelineManager::_descVertexProgram(pipelineRef) =
                GpuProgramManager::getResourceByName(
                    materialPassDesc["baseVertexGpuProgram"].GetString());
          }
          PipelineManager::_descRenderPass(pipelineRef) = renderPassRef;
          PipelineManager::_descPipelineLayout(pipelineRef) = pipelineLayoutRef;
          PipelineManager::_descVertexLayout(pipelineRef) =
              VertexLayoutManager::getResourceByName(_N(Mesh));

          if (materialPassDesc.HasMember("viewportSize"))
          {
            PipelineManager::_descScissorRenderSize(pipelineRef) =
                RenderSize::kCustom;
            PipelineManager::_descViewportRenderSize(pipelineRef) =
                RenderSize::kCustom;

            glm::uvec2 dim = RenderSystem::_backbufferDimensions;
            if (materialPassDesc["viewportSize"] == "ShadowMap")
            {
              dim = RenderPass::Shadow::_shadowMapSize;
            }
            else if (materialPassDesc["viewportSize"] == "PerPixelPicking")
            {
              dim = RenderPass::PerPixelPicking::_perPixelPickingSize;
            }
            else
            {
              _INTR_ASSERT(false && "Unknown view port size");
            }

            PipelineManager::_descAbsoluteScissorDimensions(pipelineRef) = dim;
            PipelineManager::_descAbsoluteViewportDimensions(pipelineRef) = dim;
          }

          if (materialPassDesc.HasMember("rasterizationState"))
          {
            if (materialPassDesc["rasterizationState"] == "InvertedCulling")
            {
              PipelineManager::_descRasterizationState(pipelineRef) =
                  RasterizationStates::kInvertedCulling;
            }
            else if (materialPassDesc["rasterizationState"] == "DoubleSided")
            {
              PipelineManager::_descRasterizationState(pipelineRef) =
                  RasterizationStates::kDoubleSided;
            }
          }

          if (materialPassDesc.HasMember("depthStencilState"))
          {
            PipelineManager::_descDepthStencilState(pipelineRef) =
                Helper::mapDepthStencilState(
                    materialPassDesc["depthStencilState"].GetString());
          }

          PipelineManager::_descBlendStates(pipelineRef).clear();
          if (materialPassDesc.HasMember("blendStates"))
          {
            rapidjson::Value& blendStatesDesc = materialPassDesc["blendStates"];

            for (uint32_t i = 0u; i < blendStatesDesc.Size(); ++i)
            {
              rapidjson::Value& blendStateDesc = blendStatesDesc[i];

              PipelineManager::_descBlendStates(pipelineRef)
                  .push_back(Helper::mapBlendState(blendStateDesc.GetString()));
            }
          }

          _materialPassPipelines.push_back(pipelineRef);
          matPass.pipelineIdx = (uint8_t)(_materialPassPipelines.size() - 1u);
        }

        _materialPasses.push_back(matPass);
        _materialPassMapping[materialPassName] =
            (uint8_t)(_materialPasses.size() - 1u);
      }
    }
  }

  PipelineLayoutManager::createResources(_materialPassPipelineLayouts);
  PipelineManager::createResources(_materialPassPipelines);

  DrawCallManager::_drawCallsPerMaterialPass.resize(_materialPasses.size());
}
}
}
}
}
