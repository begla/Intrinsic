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
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Settings
{
// Static members
_INTR_STRING Manager::_initialWorld = "Default.world.json";
_INTR_STRING Manager::_assetMeshPath =
    "../../Intrinsic_Assets/app/assets/meshes";
_INTR_STRING Manager::_assetTexturePath =
    "../../Intrinsic_Assets/app/assets/textures";

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
    if (doc.HasMember("assetMeshPath"))
    {
      _assetMeshPath = doc["assetMeshPath"].GetString();
    }
    if (doc.HasMember("assetTexturePath"))
    {
      _assetTexturePath = doc["assetTexturePath"].GetString();
    }
  }
}
}
}
}
