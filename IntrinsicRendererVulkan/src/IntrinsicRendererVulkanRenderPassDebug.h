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
    kWorldBoundingSpheres = 0x01u,
    kBenchmarkPaths = 0x02u,
    kSelectedObject = 0x04u,
    kWireframeRendering = 0x08u
  };
};

struct Debug
{
  static void init();
  static void onReinitRendering();
  static void destroy();

  static void renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                         const glm::vec3& p_Color0, const glm::vec3& p_Color1);
  static void renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                         uint32_t p_Color0, uint32_t p_Color1);
  static void renderSphere(const glm::vec3& p_Center, float p_Radius,
                           const glm::vec3& p_Color);
  static void renderDecal(Dod::Ref p_Decal);

  static void render(float p_DeltaT, Components::CameraRef p_CameraRef);

  static uint32_t _activeDebugStageFlags;
};
}
}
}
}
