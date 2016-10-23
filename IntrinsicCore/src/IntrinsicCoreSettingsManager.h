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

  static uint32_t _rendererFlags;
  static float _targetFrameRate;

  static WindowMode::Enum _windowMode;
  static uint32_t _screenResolutionWidth;
  static uint32_t _screenResolutionHeight;
};
}
}
}
