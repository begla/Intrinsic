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
// Static members

Dod::RefArray DefaultRenderProcess::_activeFrustums;

// <-

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

        RenderPass::PreCombine::render(p_DeltaT);
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
