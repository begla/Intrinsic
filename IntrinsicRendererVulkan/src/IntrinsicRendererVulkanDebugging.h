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

#pragma once

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
struct Debugging
{
  static void init(VkInstance& p_Instance);

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
