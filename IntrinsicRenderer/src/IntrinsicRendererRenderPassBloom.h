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

#pragma once

// Needs to be synced with the shader
#define BLUR_THREADS 128
#define BLUR_HALF_BLUR_WIDTH 7

namespace Intrinsic
{
namespace Renderer
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
  static void onReinitRendering();

  static void destroy();

  static void render(float p_DeltaT, Components::CameraRef p_CameraRef);

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
