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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
struct Debugging
{
  static void initDebugReportCallback();
  static void initDebugMarkers();

  static VkDebugReportCallbackEXT _msgCallback;
  static PFN_vkCreateDebugReportCallbackEXT _createDebugReportCallback;
  static PFN_vkDestroyDebugReportCallbackEXT _destroyDebugReportCallback;
  static PFN_vkDebugReportMessageEXT _dbgBreakCallback;

  static PFN_vkDebugMarkerSetObjectTagEXT _debugMarkerSetObjectTag;
  static PFN_vkDebugMarkerSetObjectNameEXT _debugMarkerSetObjectName;
  static PFN_vkCmdDebugMarkerBeginEXT _cmdDebugMarkerBegin;
  static PFN_vkCmdDebugMarkerEndEXT _cmdDebugMarkerEnd;
  static PFN_vkCmdDebugMarkerInsertEXT _cmdDebugMarkerInsert;
};

struct GpuMarker
{
  GpuMarker(const char* p_Name, VkCommandBuffer p_CommandBuffer)
  {
    static float color[4] = {0.0f, 1.0f, 0.0f, 1.0f};

    VkDebugMarkerMarkerInfoEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    memcpy(markerInfo.color, color, sizeof(float) * 4);
    markerInfo.pMarkerName = p_Name;

    if (Debugging::_cmdDebugMarkerInsert)
      Debugging::_cmdDebugMarkerInsert(p_CommandBuffer, &markerInfo);
  }
};

struct GpuMarkerRegion
{
  GpuMarkerRegion(const char* p_Name, VkCommandBuffer p_CommandBuffer = nullptr)
  {
    if (p_CommandBuffer == nullptr)
    {
      p_CommandBuffer = RenderSystem::getPrimaryCommandBuffer();
    }

    static float color[4] = {0.0f, 1.0f, 0.0f, 1.0f};

    VkDebugMarkerMarkerInfoEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
    memcpy(markerInfo.color, color, sizeof(float) * 4);
    markerInfo.pMarkerName = p_Name;

    if (Debugging::_cmdDebugMarkerBegin)
      Debugging::_cmdDebugMarkerBegin(p_CommandBuffer, &markerInfo);
    _vkCommandBuffer = p_CommandBuffer;
  }

  ~GpuMarkerRegion()
  {
    if (Debugging::_cmdDebugMarkerEnd)
      Debugging::_cmdDebugMarkerEnd(_vkCommandBuffer);
  }

  VkCommandBuffer _vkCommandBuffer;
};
}
}
}
