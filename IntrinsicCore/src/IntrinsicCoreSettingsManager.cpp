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
PresentMode::Enum Manager::_presentMode = PresentMode::kFifo;
_INTR_STRING Manager::_rendererConfig = "renderer_config.json";
_INTR_STRING Manager::_materialPassConfig = "material_pass_config.json";

namespace
{
template <typename T>
_INTR_INLINE void readSetting(rapidjson::Document& p_Doc, const Name& p_Name,
                              T& p_Target)
{
  if (p_Doc.HasMember(p_Name._string.c_str()))
  {
    p_Target = p_Doc[p_Name._string.c_str()].Get<T>();
    _INTR_LOG_INFO("%s = '%s'", p_Name._string.c_str(),
                   StringUtil::toString(p_Target).c_str());
  }
}

template <>
_INTR_INLINE void readSetting(rapidjson::Document& p_Doc, const Name& p_Name,
                              _INTR_STRING& p_Target)
{
  if (p_Doc.HasMember(p_Name._string.c_str()))
  {
    p_Target = p_Doc[p_Name._string.c_str()].GetString();
    _INTR_LOG_INFO("%s = '%s'", p_Name._string.c_str(), p_Target.c_str());
  }
}
}

// <-

void Manager::loadSettings()
{
  FILE* fp = fopen("settings.json", "rb");

  if (fp == nullptr)
  {
    _INTR_LOG_WARNING("No settings file available - using default values...");
    return;
  }

  _INTR_LOG_INFO("Reading settings file...");
  _INTR_LOG_PUSH();

  rapidjson::Document doc;

  char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
  {
    rapidjson::FileReadStream is(fp, readBuffer, 65536u);
    doc.ParseStream<rapidjson::kParseCommentsFlag>(is);
    fclose(fp);
  }
  Tlsf::MainAllocator::free(readBuffer);

  {
    bool rendererValidationEnabled =
        (_rendererFlags & RendererFlags::kValidationEnabled) > 0u;
    readSetting(doc, _N(rendererValidationEnabled), rendererValidationEnabled);
    if (rendererValidationEnabled)
      _rendererFlags |= RendererFlags::kValidationEnabled;
    else
      _rendererFlags &= ~RendererFlags::kValidationEnabled;

    readSetting(doc, _N(rendererConfig), _rendererConfig);
    readSetting(doc, _N(materialPassConfig), _materialPassConfig);
    readSetting(doc, _N(targetFrameRate), _targetFrameRate);
    readSetting(doc, _N(windowMode), (uint32_t&)_windowMode);
    readSetting(doc, _N(initialGameState), (uint32_t&)_initialGameState);
    readSetting(doc, _N(screenResolutionWidth), _screenResolutionWidth);
    readSetting(doc, _N(screenResolutionHeight), _screenResolutionHeight);
    readSetting(doc, _N(initialWorld), _initialWorld);
    readSetting(doc, _N(assetMeshPath), _assetMeshPath);
    readSetting(doc, _N(assetTexturePath), _assetTexturePath);
    readSetting(doc, _N(presentMode), (uint32_t&)_presentMode);
  }

  _INTR_LOG_POP();
}
}
}
}
