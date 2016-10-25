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

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
// Static members
PipelineRef MaterialManager::_pipelines[MaterialPass::kCount];
PipelineRef MaterialManager::_pipelineLayouts[MaterialPass::kCount];

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
    managerEntry.loadFromSingleFileFunction =
        MaterialManager::loadFromSingleFile;
    managerEntry.saveToSingleFileFunction = MaterialManager::saveToSingleFile;
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

void MaterialManager::initMaterialPasses()
{
  PipelineRefArray pipelinesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Surface
  {
    _pipelineLayouts[MaterialPass::kSurface] =
        PipelineLayoutManager::createPipelineLayout(_N(MaterialPassSurface));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kSurface]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName("gbuffer.vert"),
         GpuProgramManager::getResourceByName("gbuffer.frag")},
        _pipelineLayouts[MaterialPass::kSurface]);

    pipelineLayoutsToCreate.push_back(_pipelineLayouts[MaterialPass::kSurface]);
  }

  // Surface (Layered)
  {
    _pipelineLayouts[MaterialPass::kSurfaceLayered] =
        PipelineLayoutManager::createPipelineLayout(_N(MaterialPassSurface));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kSurfaceLayered]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName(
             "gbuffer_layered.vert"),
         GpuProgramManager::getResourceByName("gbuffer_layered.frag")},
        _pipelineLayouts[MaterialPass::kSurfaceLayered]);

    pipelineLayoutsToCreate.push_back(
        _pipelineLayouts[MaterialPass::kSurfaceLayered]);
  }

  // Surface (Water)
  {
    _pipelineLayouts[MaterialPass::kSurfaceWater] =
        PipelineLayoutManager::createPipelineLayout(_N(MaterialPassSurface));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kSurfaceWater]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName("gbuffer_water.vert"),
         GpuProgramManager::getResourceByName("gbuffer_water.frag")},
        _pipelineLayouts[MaterialPass::kSurfaceWater]);

    pipelineLayoutsToCreate.push_back(
        _pipelineLayouts[MaterialPass::kSurfaceWater]);
  }

  // Shadow
  {
    _pipelineLayouts[MaterialPass::kShadow] =
        PipelineLayoutManager::createPipelineLayout(_N(MaterialPassShadow));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kShadow]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName("shadow.vert")},
        _pipelineLayouts[MaterialPass::kShadow]);

    pipelineLayoutsToCreate.push_back(_pipelineLayouts[MaterialPass::kShadow]);
  }

  // Shadow foliage
  {
    _pipelineLayouts[MaterialPass::kShadowFoliage] =
        PipelineLayoutManager::createPipelineLayout(
            _N(MaterialPassShadowFoliage));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kShadowFoliage]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName("shadow_foliage.vert"),
         GpuProgramManager::getResourceByName("shadow_foliage.frag")},
        _pipelineLayouts[MaterialPass::kShadowFoliage]);

    pipelineLayoutsToCreate.push_back(
        _pipelineLayouts[MaterialPass::kShadowFoliage]);
  }

  // Per Pixel Picking
  {
    _pipelineLayouts[MaterialPass::kPerPixelPicking] =
        PipelineLayoutManager::createPipelineLayout(
            _N(MaterialPassPerPixelPicking));
    PipelineLayoutManager::resetToDefault(
        _pipelineLayouts[MaterialPass::kPerPixelPicking]);

    GpuProgramManager::reflectPipelineLayout(
        _INTR_MAX_DRAW_CALL_COUNT,
        {Resources::GpuProgramManager::getResourceByName(
             "per_pixel_picking.vert"),
         GpuProgramManager::getResourceByName("per_pixel_picking.frag")},
        _pipelineLayouts[MaterialPass::kPerPixelPicking]);

    pipelineLayoutsToCreate.push_back(
        _pipelineLayouts[MaterialPass::kPerPixelPicking]);
  }

  {
    _pipelineLayouts[MaterialPass::kDebugGizmo] =
        _pipelineLayouts[MaterialPass::kSurface];
    _pipelineLayouts[MaterialPass::kDebugGrid] =
        _pipelineLayouts[MaterialPass::kSurface];
    _pipelineLayouts[MaterialPass::kSky] =
        _pipelineLayouts[MaterialPass::kSurface];
    _pipelineLayouts[MaterialPass::kFoliage] =
        _pipelineLayouts[MaterialPass::kSurface];
  }

  // Surface
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kSurface];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassSurface));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(GBuffer));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kSurface];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Surface (Layered)
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kSurfaceLayered];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassSurface));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer_layered.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer_layered.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(GBuffer));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kSurfaceLayered];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Surface (Water)
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kSurfaceWater];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassSurface));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer_water.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("gbuffer_water.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(GBufferTransparents));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kSurfaceWater];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Shadow (Foliage)
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kShadowFoliage];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassShadow));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("shadow_foliage.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("shadow_foliage.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(Shadow));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kShadowFoliage];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descRasterizationState(pipeline) =
        RasterizationStates::kDoubleSided;
    PipelineManager::_descScissorRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descViewportRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descAbsoluteScissorDimensions(pipeline) =
        RenderPass::Shadow::_shadowMapSize;
    PipelineManager::_descAbsoluteViewportDimensions(pipeline) =
        RenderPass::Shadow::_shadowMapSize;
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descBlendStates(pipeline).clear();

    pipelinesToCreate.push_back(pipeline);
  }

  // Shadow
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kShadow];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassShadow));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("shadow.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(Shadow));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kShadow];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descScissorRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descViewportRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descAbsoluteScissorDimensions(pipeline) =
        RenderPass::Shadow::_shadowMapSize;
    PipelineManager::_descAbsoluteViewportDimensions(pipeline) =
        RenderPass::Shadow::_shadowMapSize;
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descBlendStates(pipeline).clear();

    pipelinesToCreate.push_back(pipeline);
  }

  // Gizmo
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kDebugGizmo];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassGizmo));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("gizmo.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("gizmo.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(GBuffer));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kDebugGizmo];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Grid
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kDebugGrid];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassGrid));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("grid.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("grid.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(GBuffer));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kDebugGrid];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Sky
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kSky];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassSky));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("sky.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("sky.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(Sky));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kSky];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descDepthStencilState(pipeline) =
        DepthStencilStates::kDefaultNoWrite;
    PipelineManager::_descRasterizationState(pipeline) =
        RasterizationStates::kInvertedCulling;

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Foliage
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kFoliage];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassFoliage));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("foliage.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("foliage.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(Foliage));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kFoliage];
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));
    PipelineManager::_descRasterizationState(pipeline) =
        RasterizationStates::kDoubleSided;

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  // Per Pixel Picking
  {
    PipelineRef& pipeline = _pipelines[MaterialPass::kPerPixelPicking];
    pipeline = PipelineManager::createPipeline(_N(MaterialPassPerPixelPicking));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("per_pixel_picking.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("per_pixel_picking.vert");
    PipelineManager::_descRenderPass(pipeline) =
        RenderPassManager::getResourceByName(_N(PerPixelPicking));
    PipelineManager::_descPipelineLayout(pipeline) =
        _pipelineLayouts[MaterialPass::kPerPixelPicking];
    PipelineManager::_descScissorRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descViewportRenderSize(pipeline) = RenderSize::kCustom;
    PipelineManager::_descAbsoluteScissorDimensions(pipeline) =
        RenderPass::PerPixelPicking::_perPixelPickingSize;
    PipelineManager::_descAbsoluteViewportDimensions(pipeline) =
        RenderPass::PerPixelPicking::_perPixelPickingSize;
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(Mesh));

    pipelinesToCreate.push_back(pipeline);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  PipelineManager::createResources(pipelinesToCreate);
}

void MaterialManager::createResources(const MaterialRefArray& p_Materiales)
{
  Components::MeshRefArray componentsToRecreate;

  for (uint32_t i = 0u; i < p_Materiales.size(); ++i)
  {
    MaterialRef matRef = p_Materiales[i];

    UniformManager::allocatePerMaterialDataMemory(
        sizeof(PerMaterialDataVertex), _perMaterialDataVertexOffset(matRef));
    UniformManager::allocatePerMaterialDataMemory(
        sizeof(PerMaterialDataFragment),
        _perMaterialDataFragmentOffset(matRef));

    uint32_t& materialBufferEntryIdx = _materialBufferEntryIndex(matRef);
    {
      materialBufferEntryIdx = MaterialBuffer::allocateMaterialBufferEntry();

      MaterialBufferEntry entry = {};
      {
        entry.refractionFactor = _descRefractionFactor(matRef);
        entry.translucencyThicknessFactor = _descTranslucencyThickness(matRef);
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
      fragmentData.pbrBias =
          glm::vec4(_descPbrBias(matRef), _descFoamFadeDistance(matRef));
      fragmentData.uvAnimation =
          glm::vec4(_descUvAnimation(matRef), 0.0f, 0.0f);
      fragmentData.data0[0] = materialBufferEntryIdx;

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
}
}
}
}
