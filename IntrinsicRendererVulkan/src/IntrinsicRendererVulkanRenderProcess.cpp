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

using namespace RVResources;
using namespace CResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderProcess
{
namespace
{
typedef void (*RenderPassRenderFunction)(float, Components::CameraRef);
typedef void (*RenderPassUpdateResDepResFunction)(void);

namespace RenderStepType
{
enum Enum
{
  kImageMemoryBarrier,
  kSwitchCamera,

  kRenderPassGenericFullscreen,
  kRenderPassGenericMesh,
  kRenderPassGenericBlur,

  kRenderPassDebug,
  kRenderPassPerPixelPicking,
  kRenderPassShadow,
  kRenderPassClustering,
  kRenderPassVolumetricLighting,
  kRenderPassBloom
};
}

_INTR_HASH_MAP(Name, RenderStepType::Enum)
_renderStepTypeMapping = {
    {"RenderPassDebug", RenderStepType::kRenderPassDebug},
    {"RenderPassPerPixelPicking", RenderStepType::kRenderPassPerPixelPicking},
    {"RenderPassShadow", RenderStepType::kRenderPassShadow},
    {"RenderPassClustering", RenderStepType::kRenderPassClustering},
    {"RenderPassVolumetricLighting",
     RenderStepType::kRenderPassVolumetricLighting},
    {"RenderPassBloom", RenderStepType::kRenderPassBloom}};

struct RenderPassInterface
{
  RenderPassRenderFunction render;
  RenderPassUpdateResDepResFunction onReinitRendering;
};

_INTR_HASH_MAP(RenderStepType::Enum, RenderPassInterface)
_renderStepFunctionMapping = {
    {RenderStepType::kRenderPassDebug,
     {RenderPass::Debug::render, RenderPass::Debug::onReinitRendering}},
    {RenderStepType::kRenderPassPerPixelPicking,
     {RenderPass::PerPixelPicking::render,
      RenderPass::PerPixelPicking::onReinitRendering}},
    {RenderStepType::kRenderPassShadow,
     {RenderPass::Shadow::render, RenderPass::Shadow::onReinitRendering}},
    {RenderStepType::kRenderPassClustering,
     {RenderPass::Clustering::render,
      RenderPass::Clustering::onReinitRendering}},
    {RenderStepType::kRenderPassVolumetricLighting,
     {RenderPass::VolumetricLighting::render,
      RenderPass::VolumetricLighting::onReinitRendering}},
    {RenderStepType::kRenderPassBloom,
     {RenderPass::Bloom::render, RenderPass::Bloom::onReinitRendering}}};

struct RenderStep
{
  RenderStep(uint8_t p_Type, uint8_t p_RenderPassIndex)
  {
    data = (uint32_t)p_Type | (uint32_t)p_RenderPassIndex << 8u;
    resourceName = 0x0u;
  }

  RenderStep(uint8_t p_Type, uint8_t p_SourceLayout, uint8_t p_TargetLayout,
             const Name& p_ResourceName)
  {
    data = (uint32_t)p_Type | (uint32_t)p_SourceLayout << 8u |
           (uint32_t)p_TargetLayout << 16u;
    resourceName = p_ResourceName;
  }

  _INTR_INLINE uint8_t getType() const { return data & 0xFF; }
  _INTR_INLINE uint8_t getIndex() const { return (data >> 8u) & 0xFF; }
  _INTR_INLINE uint8_t getSourceLayout() const { return (data >> 8u) & 0xFF; }
  _INTR_INLINE uint8_t getTargetLayout() const { return (data >> 16u) & 0xFF; }

  Name resourceName;
  uint32_t data;
};

_INTR_ARRAY(RenderPass::GenericFullscreen) _renderPassesGenericFullScreen;
_INTR_ARRAY(RenderPass::GenericBlur) _renderPassesGenericBlur;
_INTR_ARRAY(RenderPass::GenericMesh) _renderPassesGenericMesh;
_INTR_ARRAY(RenderStep) _renderSteps;
_INTR_ARRAY(ImageRef) _images;

_INTR_ARRAY(Name) _cameraNames;
_INTR_ARRAY(Components::CameraRef) _cameras;

_INTR_INLINE void executeRenderSteps(float p_DeltaT)
{
  Components::CameraRef activeCamera = World::_activeCamera;
  Components::CameraRef currentActiveCamera = Components::CameraRef();

  for (uint32_t i = 0u; i < _renderSteps.size(); ++i)
  {
    const RenderStep& step = _renderSteps[i];

    if (activeCamera != currentActiveCamera)
    {
      currentActiveCamera = activeCamera;

      {
        _INTR_PROFILE_CPU("Render Process", "Update Main And RP Uniforms");

        Components::MeshManager::updatePerInstanceData(currentActiveCamera, 0u);
        UniformManager::updatePerFrameUniformBufferData(currentActiveCamera);
        UniformManager::updateUniformBuffers();
      }
    }

    switch (step.getType())
    {
    case RenderStepType::kRenderPassGenericFullscreen:
      _renderPassesGenericFullScreen[step.getIndex()].render(
          p_DeltaT, currentActiveCamera);
      continue;
    case RenderStepType::kRenderPassGenericBlur:
      _renderPassesGenericBlur[step.getIndex()].render(p_DeltaT,
                                                       currentActiveCamera);
      continue;
    case RenderStepType::kRenderPassGenericMesh:
      _renderPassesGenericMesh[step.getIndex()].render(p_DeltaT,
                                                       currentActiveCamera);
      continue;
    case RenderStepType::kImageMemoryBarrier:
      ImageManager::insertImageMemoryBarrier(
          ImageManager::_getResourceByName(step.resourceName),
          (VkImageLayout)step.getSourceLayout(),
          (VkImageLayout)step.getTargetLayout());
      continue;
    case RenderStepType::kSwitchCamera:
      activeCamera = _cameras[step.getIndex()];
      continue;
    }

    auto renderPassFunction =
        _renderStepFunctionMapping.find((RenderStepType::Enum)step.getType());
    if (renderPassFunction != _renderStepFunctionMapping.end())
    {
      renderPassFunction->second.render(p_DeltaT, currentActiveCamera);
      continue;
    }

    _INTR_ASSERT(false && "Failed to execute render step");
  }
}
}

// Static members
Dod::RefArray Default::_activeFrustums;

_INTR_HASH_MAP(Components::CameraRef, _INTR_ARRAY(Dod::Ref))
Default::_shadowFrustums;
_INTR_HASH_MAP(Components::CameraRef, uint8_t)
Default::_cameraToIdMapping;

LockFreeStack<Core::Dod::Ref, _INTR_MAX_DRAW_CALL_COUNT>
    RenderProcess::Default::_visibleDrawCallsPerMaterialPass
        [_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT][_INTR_MAX_MATERIAL_PASS_COUNT];
LockFreeStack<Dod::Ref, _INTR_MAX_MESH_COMPONENT_COUNT> RenderProcess::Default::
    _visibleMeshComponents[_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT];

// <-

void Default::loadRendererConfig()
{
  // Destroy render passes and images
  {
    for (uint32_t i = 0u; i < _renderPassesGenericFullScreen.size(); ++i)
    {
      _renderPassesGenericFullScreen[i].destroy();
    }
    _renderPassesGenericFullScreen.clear();
    for (uint32_t i = 0u; i < _renderPassesGenericMesh.size(); ++i)
    {
      _renderPassesGenericMesh[i].destroy();
    }
    _renderPassesGenericMesh.clear();

    ImageManager::destroyImagesAndResources(_images);
    _images.clear();
  }
  _renderSteps.clear();
  _cameraNames.clear();

  rapidjson::Document rendererConfig;
  {
    const _INTR_STRING rendererConfigFilePath =
        "config/" + Settings::Manager::_rendererConfig;

    FILE* fp = fopen(rendererConfigFilePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_WARNING("Failed to load renderer config from file '%s'...",
                        Settings::Manager::_rendererConfig.c_str());
      return;
    }

    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      rendererConfig.ParseStream<rapidjson::kParseCommentsFlag>(is);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(readBuffer);
  }

  _INTR_LOG_INFO("Loading renderer config '%s'...",
                 rendererConfig["name"].GetString());

  const rapidjson::Value& renderSteps = rendererConfig["renderSteps"];
  const rapidjson::Value& uniformBuffers = rendererConfig["uniformBuffers"];
  const rapidjson::Value& images = rendererConfig["images"];

  // Uniform buffers
  UniformManager::load(uniformBuffers);

  // Images
  {
    for (uint32_t i = 0u; i < images.Size(); ++i)
    {
      const rapidjson::Value& image = images[i];

      ImageRef imageRef = ImageManager::createImage(image["name"].GetString());
      {
        ImageManager::resetToDefault(imageRef);
        ImageManager::addResourceFlags(
            imageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
        ImageManager::_descMemoryPoolType(imageRef) =
            MemoryPoolType::kResolutionDependentImages;

        ImageManager::_descDimensions(imageRef) = glm::uvec3(
            RenderSystem::getAbsoluteRenderSize(
                Helper::mapRenderSize(image["renderSize"].GetString())),
            1u);

        const rapidjson::Value& imageFormat = image["imageFormat"];
        if (imageFormat == "Depth")
          ImageManager::_descImageFormat(imageRef) =
              RenderSystem::_depthStencilFormatToUse;
        else
        {
          ImageManager::_descImageFormat(imageRef) =
              Helper::mapFormat(imageFormat.GetString());
          // ImageManager::_descImageFlags(imageRef) |=
          // ImageFlags::kUsageStorage;
        }

        ImageManager::_descImageType(imageRef) = ImageType::kTexture;
      }
      _images.push_back(imageRef);
    }
  }
  ImageManager::createResources(_images);

  for (uint32_t i = 0u; i < renderSteps.Size(); ++i)
  {
    const rapidjson::Value& renderStepDesc = renderSteps[i];

    if (renderStepDesc["type"] == "ImageMemoryBarrier")
    {
      _renderSteps.push_back(
          RenderStep(RenderStepType::kImageMemoryBarrier,
                     Helper::mapImageLayout(
                         renderStepDesc["sourceImageLayout"].GetString()),
                     Helper::mapImageLayout(
                         renderStepDesc["targetImageLayout"].GetString()),
                     renderStepDesc["image"].GetString()));
    }
    else if (renderStepDesc["type"] == "SwitchCamera")
    {
      const _INTR_STRING camName = renderStepDesc["cameraName"].GetString();
      _cameraNames.push_back(camName);
      _renderSteps.push_back(RenderStep(RenderStepType::kSwitchCamera,
                                        (uint8_t)(_cameraNames.size() - 1u)));
    }
    else if (renderStepDesc["type"] == "RenderPassGenericFullscreen")
    {
      _renderPassesGenericFullScreen.push_back(RenderPass::GenericFullscreen());
      RenderPass::GenericFullscreen& renderPass =
          _renderPassesGenericFullScreen.back();
      renderPass.init(renderStepDesc);

      _renderSteps.push_back(
          RenderStep(RenderStepType::kRenderPassGenericFullscreen,
                     (uint8_t)_renderPassesGenericFullScreen.size() - 1u));
    }
    else if (renderStepDesc["type"] == "RenderPassGenericBlur")
    {
      _renderPassesGenericBlur.push_back(RenderPass::GenericBlur());
      RenderPass::GenericBlur& renderPass = _renderPassesGenericBlur.back();
      renderPass.init(renderStepDesc);

      _renderSteps.push_back(
          RenderStep(RenderStepType::kRenderPassGenericBlur,
                     (uint8_t)_renderPassesGenericBlur.size() - 1u));
    }
    else if (renderStepDesc["type"] == "RenderPassGenericMesh")
    {
      _renderPassesGenericMesh.push_back(RenderPass::GenericMesh());
      RenderPass::GenericMesh& renderPass = _renderPassesGenericMesh.back();
      renderPass.init(renderStepDesc);

      _renderSteps.push_back(
          RenderStep(RenderStepType::kRenderPassGenericMesh,
                     (uint8_t)_renderPassesGenericMesh.size() - 1u));
    }
    else if (_renderStepTypeMapping.find(renderStepDesc["type"].GetString()) !=
             _renderStepTypeMapping.end())
    {
      RenderStep renderStep =
          RenderStep(_renderStepTypeMapping[renderStepDesc["type"].GetString()],
                     (uint8_t)-1);
      _renderStepFunctionMapping[(RenderStepType::Enum)renderStep.getType()]
          .onReinitRendering();
      _renderSteps.push_back(renderStep);
    }
    else
    {
      _INTR_ASSERT(false && "Invalid render step type provided");
    }
  }
}

void Default::renderFrame(float p_DeltaT)
{
  // Resize the swap chain (if necessary)
  RenderSystem::resizeSwapChain();

  RenderSystem::beginFrame();
  {
    _INTR_PROFILE_GPU("Render Frame");
    _INTR_PROFILE_CPU("Render Process", "Render Frame");

    // Preparation
    {
      _INTR_PROFILE_CPU("Render Process", "Culling");

      // Update camera array
      {
        _cameras.clear();
        if (!_cameraNames.empty())
        {
          for (uint32_t i = 0u; i < _cameraNames.size(); ++i)
          {
            const Name& cameraName = _cameraNames[i];

            Components::CameraRef cam = World::_activeCamera;
            if (cameraName != _N(ActiveCamera))
              cam = Components::CameraManager::getComponentForEntity(
                  Entity::EntityManager::getEntityByName(cameraName));

            _cameras.push_back(cam);
          }
        }
        else
        {
          _cameras.push_back(World::_activeCamera);
        }
      }

      // Collect frustums for culling
      _activeFrustums.clear();
      for (uint32_t i = 0u; i < _cameras.size(); ++i)
      {
        Components::CameraRef camRef = _cameras[i];

        FrustumRef frustumRef = Components::CameraManager::_frustum(camRef);
        _cameraToIdMapping[camRef] = (uint8_t)_activeFrustums.size();
        _activeFrustums.push_back(frustumRef);

        // Only allow shadows for the main view
        if (_cameraNames[i] == _N(ActiveCamera))
        {
          _INTR_ARRAY(FrustumRef)& shadowFrustums = _shadowFrustums[camRef];
          RenderPass::Shadow::prepareFrustums(camRef, shadowFrustums);
          _activeFrustums.insert(RenderProcess::Default::_activeFrustums.end(),
                                 shadowFrustums.begin(), shadowFrustums.end());
        }
      }

      Components::CameraManager::updateFrustumsAndMatrices(
          Components::CameraManager::_activeRefs);
      FrustumManager::prepareForRendering(FrustumManager::_activeRefs);

      FrustumManager::cullNodes(RenderProcess::Default::_activeFrustums);

      // Collect visible draw calls and mesh components
      Components::MeshManager::collectDrawCallsAndMeshComponents();
      UniformManager::resetAllocator();
    }

    // Execute render steps
    {
      executeRenderSteps(p_DeltaT);
    }
  }
  RenderSystem::endFrame();
}
}
}
}
}
