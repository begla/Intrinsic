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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Settings
{
namespace WindowMode
{
enum Enum
{
  kWindowed,
  kBorderlessWindow,
  kFullscreen,
  kFullscreenDesktop
};
}

namespace RendererFlags
{
enum Flags
{
  kValidationEnabled = 0x01u
};
}

struct Manager
{
  static void loadSettings();

  static _INTR_STRING _initialWorld;
  static _INTR_STRING _assetMeshPath;
  static _INTR_STRING _assetTexturePath;

  static uint32_t _rendererFlags;
  static float _targetFrameRate;

  static WindowMode::Enum _windowMode;
  static uint32_t _screenResolutionWidth;
  static uint32_t _screenResolutionHeight;
};
}
}
}
