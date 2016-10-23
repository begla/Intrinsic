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

#define BLUR_X_THREADS_X 4
#define BLUR_X_THREADS_Y 64

#define BLUR_Y_THREADS_X 64
#define BLUR_Y_THREADS_Y 4

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
struct PerInstanceDataBlur
{
  uint32_t mipLevel[4];
};

struct Bloom
{
  static void init();
  static void updateResolutionDependentResources();

  static void destroy();

  static void render(float p_DeltaT);

  // <-

  _INTR_INLINE static uint32_t calculateThreadGroups(uint32_t p_WorkItems,
                                                     uint32_t p_GroupItems)
  {
    uint32_t groups = p_WorkItems / p_GroupItems;

    if (p_WorkItems % p_GroupItems > 0u)
    {
      groups++;
    }

    return groups;
  }
};
}
}
}
}
