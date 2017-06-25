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

namespace Intrinsic
{
namespace Renderer
{
namespace InputAssemblyStates
{
enum Enum
{
  kTriangleList,
  kLineList,

  kCount
};
}

namespace RasterizationStates
{
enum Enum
{
  kDefault,
  kInvertedCulling,
  kDoubleSided,
  kWireframe,

  kCount
};
}

namespace BlendStates
{
enum Enum
{
  kDefault,
  kAlphaBlend,

  kCount
};
}

namespace DepthStencilStates
{
enum Enum
{
  kDefault,
  kDefaultNoDepthTestAndWrite,
  kDefaultNoWrite,
  kDefaultNoDepthTest,

  kCount
};
}

struct RenderStates
{
  static void init();

  static _INTR_ARRAY(VkPipelineInputAssemblyStateCreateInfo)
      inputAssemblyStates;
  static _INTR_ARRAY(VkPipelineRasterizationStateCreateInfo)
      rasterizationStates;
  static _INTR_ARRAY(VkPipelineDepthStencilStateCreateInfo) depthStencilStates;
  static _INTR_ARRAY(VkPipelineColorBlendAttachmentState) blendStates;
};
}
}
