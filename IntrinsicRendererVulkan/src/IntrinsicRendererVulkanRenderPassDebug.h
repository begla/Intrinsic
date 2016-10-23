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
namespace RenderPass
{
struct DebugStageFlags
{
  enum Flags
  {
    kWorldBoundingSpheres = 0x01
  };
};

struct Debug
{
  static void init();
  static void updateResolutionDependentResources();
  static void destroy();

  static void renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                         const glm::vec3& p_Color0, const glm::vec3& p_Color1);
  static void renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                         uint32_t p_Color0, uint32_t p_Color1);
  static void renderSphere(const glm::vec3& p_Center, float p_Radius,
                           const glm::vec4& p_Color);

  static void render(float p_DeltaT);

  static uint32_t _activeDebugStageFlags;
};
}
}
}
}
