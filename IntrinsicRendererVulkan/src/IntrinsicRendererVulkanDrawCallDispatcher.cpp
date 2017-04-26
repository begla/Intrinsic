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
struct DrawCallParallelTaskSet : enki::ITaskSet
{
  void ExecuteRange(enki::TaskSetPartition p_Range, uint32_t p_ThreadNum)
  {
    _INTR_PROFILE_CPU("Draw Calls", "Dispatch Draw Calls");

    RenderSystem::beginSecondaryCommandBuffer(
        _secondaryCmdBufferIdx,
        Resources::RenderPassManager::_vkRenderPass(_renderPassRef),
        Resources::FramebufferManager::_vkFrameBuffer(_framebufferRef));

    VkCommandBuffer secondCmdBuffer =
        *RenderSystem::getSecondaryCommandBuffers(_secondaryCmdBufferIdx);

    VkPipeline currentPipeline = VK_NULL_HANDLE;

    for (uint32_t dcIdx = _rangeStart; dcIdx < _rangeEnd; ++dcIdx)
    {
      Resources::DrawCallRef drawCallRef = (*_visibleDrawCallRefs)[dcIdx];
      _INTR_ASSERT(Resources::DrawCallManager::isAlive(drawCallRef));

      Resources::PipelineRef pipelineRef =
          Resources::DrawCallManager::_descPipeline(drawCallRef);
      Resources::PipelineLayoutRef pipelineLayoutRef =
          Resources::PipelineManager::_descPipelineLayout(pipelineRef);

      VkPipeline newPipeline =
          Resources::PipelineManager::_vkPipeline(pipelineRef);
      if (newPipeline != currentPipeline)
      {
        vkCmdBindPipeline(secondCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          newPipeline);
        currentPipeline = newPipeline;
      }

      _INTR_ASSERT(Resources::DrawCallManager::_vkDescriptorSet(drawCallRef));
      vkCmdBindDescriptorSets(
          secondCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          Resources::PipelineLayoutManager::_vkPipelineLayout(
              pipelineLayoutRef),
          0u, 1u, &Resources::DrawCallManager::_vkDescriptorSet(drawCallRef),
          (uint32_t)Resources::DrawCallManager::_dynamicOffsets(drawCallRef)
              .size(),
          Resources::DrawCallManager::_dynamicOffsets(drawCallRef).data());

      // Bind vertex buffers
      {
        _INTR_ARRAY(VkBuffer)& vtxBuffers =
            Resources::DrawCallManager::_vertexBuffers(drawCallRef);
        vkCmdBindVertexBuffers(
            secondCmdBuffer, 0u, (uint32_t)vtxBuffers.size(), vtxBuffers.data(),
            Resources::DrawCallManager::_vertexBufferOffsets(drawCallRef)
                .data());
      }

      // Draw
      {
        Resources::BufferRef indexBufferRef =
            Resources::DrawCallManager::_descIndexBuffer(drawCallRef);
        if (indexBufferRef.isValid())
        {
          const VkIndexType indexType =
              Resources::BufferManager::_descBufferType(indexBufferRef) ==
                      BufferType::kIndex16
                  ? VK_INDEX_TYPE_UINT16
                  : VK_INDEX_TYPE_UINT32;
          vkCmdBindIndexBuffer(
              secondCmdBuffer,
              Resources::BufferManager::_vkBuffer(indexBufferRef),
              Resources::DrawCallManager::_indexBufferOffset(drawCallRef),
              indexType);
          vkCmdDrawIndexed(
              secondCmdBuffer,
              Resources::DrawCallManager::_descIndexCount(drawCallRef),
              Resources::DrawCallManager::_descInstanceCount(drawCallRef), 0u,
              0u, 0u);
        }
        else
        {
          vkCmdDraw(secondCmdBuffer,
                    Resources::DrawCallManager::_descVertexCount(drawCallRef),
                    Resources::DrawCallManager::_descInstanceCount(drawCallRef),
                    0u, 0u);
        }

        DrawCallDispatcher::_dispatchedDrawCallCount++;
      }
    }

    RenderSystem::endSecondaryCommandBuffer(_secondaryCmdBufferIdx);
  }

  uint32_t _secondaryCmdBufferIdx;
  Resources::DrawCallRefArray* _visibleDrawCallRefs;
  Resources::FramebufferRef _framebufferRef;
  Resources::RenderPassRef _renderPassRef;

  uint32_t _rangeStart;
  uint32_t _rangeEnd;
};

DrawCallParallelTaskSet _tasks[_INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT] = {};
uint32_t _activeTaskCount = 0u;
}

std::atomic<uint32_t> DrawCallDispatcher::_dispatchedDrawCallCount;
uint32_t DrawCallDispatcher::_totalDispatchedDrawCallCountPerFrame = 0u;
uint32_t DrawCallDispatcher::_totalDispatchCallsPerFrame = 0u;

// <-

void DrawCallDispatcher::queueDrawCalls(Core::Dod::RefArray& p_DrawCalls,
                                        Core::Dod::Ref p_RenderPass,
                                        Core::Dod::Ref p_Framebuffer)
{
  _INTR_PROFILE_CPU("Draw Calls", "Queue Draw Calls");

  _dispatchedDrawCallCount = 0u;
  const uint32_t dcCount = (uint32_t)p_DrawCalls.size();

  if (dcCount == 0u)
  {
    return;
  }

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  uint32_t dcsPerBatch =
      std::max(dcCount / Application::_scheduler.GetNumTaskThreads(), 1u);
  uint32_t dcRangeLeft = dcCount;
  uint32_t firstTaskIndex = _activeTaskCount;
  uint32_t tasksQueued = 0u;

  while (dcRangeLeft > 0)
  {
    if (dcsPerBatch > dcRangeLeft)
    {
      dcsPerBatch = dcRangeLeft;
    }

    DrawCallParallelTaskSet& task = _tasks[_activeTaskCount];
    task._framebufferRef = p_Framebuffer;
    task._renderPassRef = p_RenderPass;
    task._visibleDrawCallRefs = &p_DrawCalls;
    task._rangeStart = dcCount - dcRangeLeft;
    task._rangeEnd = task._rangeStart + dcsPerBatch;
    task._secondaryCmdBufferIdx =
        RenderSystem::requestSecondaryCommandBuffers(1u);

    Application::_scheduler.AddTaskSetToPipe(&task);

    dcRangeLeft -= dcsPerBatch;
    ++_activeTaskCount;
    ++tasksQueued;
  }

  {
    _INTR_PROFILE_CPU("Draw Calls", "Wait For Draw Calls");

    for (uint32_t taskIdx = firstTaskIndex;
         taskIdx < firstTaskIndex + tasksQueued; ++taskIdx)
    {
      Application::_scheduler.WaitforTaskSet(&_tasks[taskIdx]);
      vkCmdExecuteCommands(primaryCmdBuffer, 1u,
                           RenderSystem::getSecondaryCommandBuffers(
                               _tasks[taskIdx]._secondaryCmdBufferIdx));
    }
  }

  _totalDispatchedDrawCallCountPerFrame += _dispatchedDrawCallCount;
  ++_totalDispatchCallsPerFrame;
}

// <-

void DrawCallDispatcher::reset()
{
  _INTR_PROFILE_COUNTER_SET("Total Dispatched Draw Calls",
                            _totalDispatchedDrawCallCountPerFrame);
  _INTR_PROFILE_COUNTER_SET("Total Draw Call Dispatch Calls",
                            _totalDispatchCallsPerFrame);

  _totalDispatchCallsPerFrame = 0u;
  _totalDispatchedDrawCallCountPerFrame = 0u;
  _activeTaskCount = 0u;
}
}
}
}
