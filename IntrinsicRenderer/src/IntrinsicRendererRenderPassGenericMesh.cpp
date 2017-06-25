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
#include "stdafx_renderer.h"
#include "stdafx.h"

using namespace RResources;

namespace Intrinsic
{
namespace Renderer
{
namespace RenderPass
{
void GenericMesh::init(const rapidjson::Value& p_RenderPassDesc)
{
  Base::init(p_RenderPassDesc);

  const rapidjson::Value& materialPassDescs =
      p_RenderPassDesc["materialPasses"];

  for (uint32_t i = 0u; i < materialPassDescs.Size(); ++i)
  {
    _materialPassNames.push_back(materialPassDescs[i].GetString());
  }

  if (p_RenderPassDesc["renderOrder"] == "FrontToBack")
  {
    _renderOrder = RenderOrder::kFrontToBack;
  }
  else if (p_RenderPassDesc["renderOrder"] == "BackToFront")
  {
    _renderOrder = RenderOrder::kBackToFront;
  }
  else
  {
    _INTR_ASSERT(false && "Unknown render order");
  }
}

// <-

void GenericMesh::destroy()
{
  Base::destroy();

  _materialPassIds.clear();
  _materialPassNames.clear();
}

// <-

void GenericMesh::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU_DEFINE(GenericMeshCPU, "Render Pass", _name.c_str());
  _INTR_PROFILE_CPU_CUSTOM(GenericMeshCPU);
  _INTR_PROFILE_GPU_DEFINE(GenericMeshGPU, _name.c_str());
  _INTR_PROFILE_GPU_CUSTOM(GenericMeshGPU, _name.c_str());

  static DrawCallRefArray visibleDrawCalls;
  visibleDrawCalls.clear();

  if (_materialPassIds.size() != _materialPassNames.size())
  {
    _materialPassIds.clear();

    for (uint32_t i = 0u; i < _materialPassNames.size(); ++i)
    {
      _materialPassIds.push_back(
          MaterialManager::getMaterialPassId(_materialPassNames[i]));
    }
  }

  for (uint32_t matPassId = 0u; matPassId < _materialPassIds.size();
       ++matPassId)
  {
    const uint8_t materialPassId = _materialPassIds[matPassId];

    RenderProcess::Default::getVisibleDrawCalls(p_CameraRef, 0u, materialPassId)
        .copy(visibleDrawCalls);
  }

  if (_renderOrder == RenderOrder::kFrontToBack)
    DrawCallManager::sortDrawCallsFrontToBack(visibleDrawCalls);
  else if (_renderOrder == RenderOrder::kBackToFront)
    DrawCallManager::sortDrawCallsBackToFront(visibleDrawCalls);

  // Update per mesh uniform data
  {
    CComponents::MeshManager::updateUniformData(visibleDrawCalls);
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  FramebufferRef fbRef = _framebufferRefs[RenderSystem::_backbufferIndex %
                                          _framebufferRefs.size()];

  RenderSystem::beginRenderPass(
      _renderPassRef, fbRef, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
      (uint32_t)_clearValues.size(), _clearValues.data());
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef, fbRef);
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
