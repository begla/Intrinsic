// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
