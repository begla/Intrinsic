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
namespace
{
namespace RenderStepType
{
enum Enum
{
  kImageMemoryBarrier,

  kRenderPassGenericFullscreen
};
}

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
_INTR_ARRAY(RenderStep) _renderSteps;

_INTR_INLINE void executeRenderSteps(float p_DeltaT)
{
  for (uint32_t i = 0u; i < _renderSteps.size(); ++i)
  {
    const RenderStep& step = _renderSteps[i];

    if (step.getType() == RenderStepType::kRenderPassGenericFullscreen)
    {
      _renderPassesGenericFullScreen[step.getIndex()].render(p_DeltaT);
    }
    else if (step.getType() == RenderStepType::kImageMemoryBarrier)
    {
      using namespace Resources;

      ImageManager::insertImageMemoryBarrier(
          ImageManager::_getResourceByName(step.resourceName),
          (VkImageLayout)step.getSourceLayout(),
          (VkImageLayout)step.getTargetLayout());
    }
  }
}
}

// Static members
Dod::RefArray DefaultRenderProcess::_activeFrustums;

// <-

void DefaultRenderProcess::load()
{
  for (uint32_t i = 0u; i < _renderPassesGenericFullScreen.size(); ++i)
  {
    _renderPassesGenericFullScreen[i].destroy();
  }
  _renderPassesGenericFullScreen.clear();
  _renderSteps.clear();

  rapidjson::Document rendererConfig;
  {
    FILE* fp = fopen(Settings::Manager::_rendererConfig.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_WARNING("Failed to load resources from file '%s'...",
                        Settings::Manager::_rendererConfig.c_str());
      return;
    }

    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      rendererConfig.ParseStream(is);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(readBuffer);
  }

  _INTR_LOG_INFO("Loading renderer config '%s'...",
                 rendererConfig["name"].GetString());
  const rapidjson::Value& renderSteps = rendererConfig["renderSteps"];

  for (uint32_t i = 0u; i < renderSteps.Size(); ++i)
  {
    const rapidjson::Value& renderStep = renderSteps[i];

    if (renderStep["type"] == "ImageMemoryBarrier")
    {
      // TODO
      _renderSteps.push_back(RenderStep(
          RenderStepType::kImageMemoryBarrier, VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          renderStep["image"].GetString()));
    }
    else if (renderStep["type"] == "RenderPassGenericFullscreen")
    {
      _renderPassesGenericFullScreen.push_back(RenderPass::GenericFullscreen());
      RenderPass::GenericFullscreen& renderPass =
          _renderPassesGenericFullScreen.back();
      renderPass.init(renderStep);

      _renderSteps.push_back(
          RenderStep(RenderStepType::kRenderPassGenericFullscreen,
                     (uint8_t)_renderPassesGenericFullScreen.size() - 1u));
    }
  }
}

void DefaultRenderProcess::renderFrame(float p_DeltaT)
{
  // Resize the swap chain (if necessary)
  RenderSystem::resizeSwapChain();

  RenderSystem::beginFrame();
  {
    _INTR_PROFILE_GPU("Render Frame");
    _INTR_PROFILE_CPU("Render System", "Render Frame");

    // Preparation and culling
    {
      _INTR_PROFILE_CPU("Render System", "Preparation and Culling");

      Components::CameraManager::updateFrustums(
          Components::CameraManager::_activeRefs);
      RenderPass::Shadow::prepareFrustums();
      Core::Resources::FrustumManager::prepareForRendering(
          Core::Resources::FrustumManager::_activeRefs);

      _activeFrustums.clear();
      _activeFrustums.push_back(
          Components::CameraManager::_frustum(World::getActiveCamera()));
      _activeFrustums.insert(_activeFrustums.end(),
                             RenderPass::Shadow::_shadowFrustums.begin(),
                             RenderPass::Shadow::_shadowFrustums.end());

      Core::Resources::FrustumManager::cullNodes(_activeFrustums);
    }

    // Collect visible draw calls and mesh components
    {
      Components::MeshManager::collectDrawCallsAndMeshComponents();
    }

    // Render passes
    {
      // GBuffer passes
      {
        _INTR_PROFILE_CPU("Render System", "GBuffer");
        _INTR_PROFILE_GPU("GBuffer");

        Components::MeshManager::updatePerInstanceData(0u);

        RenderPass::GBuffer::render(p_DeltaT);
        RenderPass::Foliage::render(p_DeltaT);
        RenderPass::Sky::render(p_DeltaT);
        RenderPass::Debug::render(p_DeltaT);
        RenderPass::GBufferTransparents::render(p_DeltaT);
        RenderPass::PerPixelPicking::render(p_DeltaT);
      }

      // Lighting passes
      {
        _INTR_PROFILE_CPU("Render System", "Shadows/Lighting");
        _INTR_PROFILE_GPU("Shadows/Lighting");

        RenderPass::Shadow::render(p_DeltaT);
        RenderPass::Lighting::render(p_DeltaT);
        RenderPass::VolumetricLighting::render(p_DeltaT);
      }

      // Post and combine passes
      {
        _INTR_PROFILE_CPU("Render System", "Post/Combine");
        _INTR_PROFILE_GPU("Post/Combine");

        executeRenderSteps(p_DeltaT);
        {
          RenderPass::Bloom::render(p_DeltaT);
          RenderPass::LensFlare::render(p_DeltaT);
        }
        RenderPass::PostCombine::render(p_DeltaT);
      }
    }
  }

  RenderSystem::endFrame();
}
}
}
}
