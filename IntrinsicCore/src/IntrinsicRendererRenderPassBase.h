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

namespace Intrinsic
{
namespace Renderer
{
namespace RenderPass
{
struct Base
{
  void init(const rapidjson::Value& p_RenderPassDesc);
  void destroy();

protected:
  _INTR_STRING _name;

  Resources::FramebufferRefArray _framebufferRefs;
  Resources::RenderPassRef _renderPassRef;
  Resources::ImageRefArray _imageRefs;

  _INTR_ARRAY(VkClearValue) _clearValues;
};
}
}
}
