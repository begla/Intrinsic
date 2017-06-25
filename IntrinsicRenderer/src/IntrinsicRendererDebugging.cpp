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

namespace Intrinsic
{
namespace Renderer
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

  Log::LogLevel::Enum logLevel = Log::LogLevel::kInfo;

  if (p_Flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
  {
    logLevel = Log::LogLevel::kError;
    errorLevel += "ERROR";
  }
  if (p_Flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
  {
    logLevel = Log::LogLevel::kWarning;
    errorLevel += "WARNING";
  }
  if (p_Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
  {
    logLevel = Log::LogLevel::kWarning;
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

  Log::Manager::log(logLevel, "Vulkan : [%s] [%s] Code %u : %s",
                    errorLevel.c_str(), p_LayerPrefix, p_MsgCode, p_Message);

  return VK_FALSE;
}

// <-

void Debugging::initDebugReportCallback()
{
  // Validation
  _createDebugReportCallback =
      (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
          RenderSystem::_vkInstance, "vkCreateDebugReportCallbackEXT");
  _destroyDebugReportCallback =
      (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          RenderSystem::_vkInstance, "vkCreateDebugReportCallbackEXT");
  _dbgBreakCallback = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(
      RenderSystem::_vkInstance, "vkDebugReportMessageEXT");

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
    VkResult result = _createDebugReportCallback(
        RenderSystem::_vkInstance, &dbgCreateInfo, nullptr, &_msgCallback);
    _INTR_VK_CHECK_RESULT(result);
  }
}

void Debugging::initDebugMarkers()
{
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
