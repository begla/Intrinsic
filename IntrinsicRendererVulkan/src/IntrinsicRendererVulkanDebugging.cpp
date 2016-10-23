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
// ->

VkDebugReportCallbackEXT Debugging::_msgCallback = 0u;
PFN_vkCreateDebugReportCallbackEXT Debugging::_createDebugReportCallback =
    nullptr;
PFN_vkDestroyDebugReportCallbackEXT Debugging::_destroyDebugReportCallback =
    nullptr;
PFN_vkDebugReportMessageEXT Debugging::_dbgBreakCallback = nullptr;

PFN_vkDebugMarkerSetObjectTagEXT Debugging::_debugMarkerSetObjectTag = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT Debugging::_debugMarkerSetObjectName =
    nullptr;
PFN_vkCmdDebugMarkerBeginEXT Debugging::_cmdDebugMarkerBegin = nullptr;
PFN_vkCmdDebugMarkerEndEXT Debugging::_cmdDebugMarkerEnd = nullptr;
PFN_vkCmdDebugMarkerInsertEXT Debugging::_cmdDebugMarkerInsert = nullptr;

// <-

VkBool32 messageCallback(VkDebugReportFlagsEXT p_Flags,
                         VkDebugReportObjectTypeEXT p_ObjType,
                         uint64_t p_SrcObject, size_t p_Location,
                         int32_t p_MsgCode, const char* p_LayerPrefix,
                         const char* p_Message, void* p_userData)
{
  _INTR_STRING text = p_Message;
  _INTR_STRING errorLevel = "";

  Intrinsic::Core::Log::LogLevel::Enum logLevel =
      Intrinsic::Core::Log::LogLevel::kInfo;

  if (p_Flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
  {
    logLevel = Intrinsic::Core::Log::LogLevel::kError;
    errorLevel += "ERROR";
  }
  if (p_Flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
  {
    logLevel = Intrinsic::Core::Log::LogLevel::kWarning;
    errorLevel += "WARNING";
  }
  if (p_Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
  {
    logLevel = Intrinsic::Core::Log::LogLevel::kWarning;
    errorLevel += "PERFORMANCE";
  }
  if (p_Flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
  {
    errorLevel += "INFO";
  }
  if (p_Flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
  {
    errorLevel += "DEBUG";
  }

  Intrinsic::Core::Log::Manager::log(
      logLevel, "Vulkan : [%s] [%s] Code %u : %s", errorLevel.c_str(),
      p_LayerPrefix, p_MsgCode, p_Message);

  return VK_FALSE;
}

// <-

void Debugging::init(VkInstance& p_Instance)
{
  // Validation
  _createDebugReportCallback =
      (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
          p_Instance, "vkCreateDebugReportCallbackEXT");
  _destroyDebugReportCallback =
      (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          p_Instance, "vkCreateDebugReportCallbackEXT");
  _dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(
      p_Instance, "vkDebugReportMessageEXT");

  VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
  {
    dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
    dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                          VK_DEBUG_REPORT_WARNING_BIT_EXT |
                          VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
  }

  if (_createDebugReportCallback)
  {
    VkResult result = _createDebugReportCallback(p_Instance, &dbgCreateInfo,
                                                 nullptr, &_msgCallback);
    _INTR_VK_CHECK_RESULT(result);
  }

  // Markers
  _debugMarkerSetObjectTag =
      (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(
          RenderSystem::_vkDevice, "vkDebugMarkerSetObjectTagEXT");
  _debugMarkerSetObjectName =
      (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(
          RenderSystem::_vkDevice, "vkDebugMarkerSetObjectNameEXT");
  _cmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(
      RenderSystem::_vkDevice, "vkCmdDebugMarkerBeginEXT");
  _cmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(
      RenderSystem::_vkDevice, "vkCmdDebugMarkerEndEXT");
  _cmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(
      RenderSystem::_vkDevice, "vkCmdDebugMarkerInsertEXT");
}
}
}
}
