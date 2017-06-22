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
namespace Vulkan
{
struct DrawCallDispatcher
{
  static void onFrameEnded();
  static void queueDrawCalls(Core::Dod::RefArray& p_DrawCalls,
                             Core::Dod::Ref p_RenderPass,
                             Core::Dod::Ref p_Framebuffer);

  static std::atomic<uint32_t> _dispatchedDrawCallCount;
  static uint32_t _totalDispatchedDrawCallCountPerFrame;
  static uint32_t _totalDispatchCallsPerFrame;
};
}
}
}
