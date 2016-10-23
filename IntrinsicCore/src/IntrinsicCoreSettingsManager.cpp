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

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Settings
{
// Static members
_INTR_STRING Manager::_initialWorld = "Default.world.json";

uint32_t Manager::_rendererFlags = 0u;
float Manager::_targetFrameRate = 0.016f;

WindowMode::Enum Manager::_windowMode = WindowMode::kWindowed;
uint32_t Manager::_screenResolutionWidth = 1280u;
uint32_t Manager::_screenResolutionHeight = 720u;

// <-

void Manager::loadSettings()
{
  FILE* fp = fopen("settings.json", "rb");

  if (fp == nullptr)
  {
    _INTR_LOG_WARNING("No settings file available - using default values...");
    return;
  }

  rapidjson::Document doc;

  char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileReadStream is(fp, readBuffer, 65536u);
    doc.ParseStream(is);
    fclose(fp);
  }
  Tlsf::MainAllocator::free(readBuffer);

  {
    if (doc.HasMember("rendererValidationEnabled"))
    {
      if (doc["rendererValidationEnabled"].GetBool())
      {
        _rendererFlags |= RendererFlags::kValidationEnabled;
      }
      else
      {
        _rendererFlags &= ~RendererFlags::kValidationEnabled;
      }
    }
    if (doc.HasMember("targetFrameRate"))
    {
      _targetFrameRate = doc["targetFrameRate"].GetFloat();
    }

    if (doc.HasMember("windowMode"))
    {
      _windowMode = (WindowMode::Enum)doc["windowMode"].GetUint();
    }
    if (doc.HasMember("screenResolutionWidth"))
    {
      _screenResolutionWidth =
          (WindowMode::Enum)doc["screenResolutionWidth"].GetUint();
    }
    if (doc.HasMember("screenResolutionHeight"))
    {
      _screenResolutionHeight =
          (WindowMode::Enum)doc["screenResolutionHeight"].GetUint();
    }
    if (doc.HasMember("initialWorld"))
    {
      _initialWorld = doc["initialWorld"].GetString();
    }
  }
}
}
}
}
