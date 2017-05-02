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
namespace RenderPass
{
void GenericMesh::init(const rapidjson::Value& p_RenderPassDesc)
{
  using namespace Resources;

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
  using namespace Resources;

  Base::destroy();

  _materialPassIds.clear();
  _materialPassNames.clear();
}

// <-

void GenericMesh::render(float p_DeltaT)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Renderer", _name.c_str());
  _INTR_PROFILE_GPU(_name.c_str());

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
    RenderProcess::Default::_visibleDrawCallsPerMaterialPass
        [0u][_materialPassIds[matPassId]]
            .copy(visibleDrawCalls);
  }

  if (_renderOrder == RenderOrder::kFrontToBack)
    DrawCallManager::sortDrawCallsFrontToBack(visibleDrawCalls);
  else if (_renderOrder == RenderOrder::kBackToFront)
    DrawCallManager::sortDrawCallsBackToFront(visibleDrawCalls);

  // Update per mesh uniform data
  {
    Core::Components::MeshManager::updateUniformData(visibleDrawCalls);
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  FramebufferRef fbRef = _framebufferRefs[RenderSystem::_backbufferIndex %
                                          _framebufferRefs.size()];

  RenderSystem::beginRenderPass(
      _renderPassRef, fbRef, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
      (uint32_t)_clearValues.size(), _clearValues.data());
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef, fbRef);
    _INTR_PROFILE_COUNTER_SET(("Dispatched Draw Calls (" + _name + ")").c_str(),
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);
}
}
}
}
}
