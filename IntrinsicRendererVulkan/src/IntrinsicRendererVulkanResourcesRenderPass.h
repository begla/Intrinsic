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
namespace Resources
{
typedef Dod::Ref RenderPassRef;
typedef Dod::RefArray RenderPassRefArray;

typedef _INTR_ARRAY(AttachmentDescription) AttachmentArray;

struct RenderPassData : Dod::Resources::ResourceDataBase
{
  RenderPassData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_RENDER_PASS_COUNT)
  {
    descAttachments.resize(_INTR_MAX_RENDER_PASS_COUNT);

    vkRenderPass.resize(_INTR_MAX_RENDER_PASS_COUNT);
  }

  // Description
  _INTR_ARRAY(_INTR_ARRAY(AttachmentDescription)) descAttachments;

  // GPU resources
  _INTR_ARRAY(VkRenderPass) vkRenderPass;
};

struct RenderPassManager
    : Dod::Resources::ResourceManagerBase<RenderPassData,
                                          _INTR_MAX_RENDER_PASS_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Render Pass Manager...");

    Dod::Resources::ResourceManagerBase<
        RenderPassData, _INTR_MAX_RENDER_PASS_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static RenderPassRef createRenderPass(const Name& p_Name)
  {
    RenderPassRef ref = Dod::Resources::ResourceManagerBase<
        RenderPassData, _INTR_MAX_RENDER_PASS_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(RenderPassRef p_Ref)
  {
    _descAttachments(p_Ref).clear();
  }

  _INTR_INLINE static void destroyRenderPass(RenderPassRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        RenderPassData, _INTR_MAX_RENDER_PASS_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(RenderPassRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        RenderPassData,
        _INTR_MAX_RENDER_PASS_COUNT>::_compileDescriptor(p_Ref, p_Properties,
                                                         p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(RenderPassRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        RenderPassData,
        _INTR_MAX_RENDER_PASS_COUNT>::_initFromDescriptor(p_Ref, p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        RenderPassData,
        _INTR_MAX_RENDER_PASS_COUNT>::_saveToSingleFile(p_FileName,
                                                        compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        RenderPassData,
        _INTR_MAX_RENDER_PASS_COUNT>::_loadFromSingleFile(p_FileName,
                                                          initFromDescriptor,
                                                          resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const RenderPassRefArray& p_RenderPasses);

  _INTR_INLINE static void
  destroyResources(const RenderPassRefArray& p_RenderPasses)
  {
    for (uint32_t i = 0u; i < p_RenderPasses.size(); ++i)
    {
      RenderPassRef ref = p_RenderPasses[i];
      VkRenderPass& renderPass = _vkRenderPass(ref);

      if (renderPass != VK_NULL_HANDLE)
      {
        vkDestroyRenderPass(RenderSystem::_vkDevice, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
      }
    }
  }

  // Getter/Setter
  // ->

  // Description
  _INTR_INLINE static _INTR_ARRAY(AttachmentDescription) &
      _descAttachments(RenderPassRef p_Ref)
  {
    return _data.descAttachments[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkRenderPass& _vkRenderPass(RenderPassRef p_Ref)
  {
    return _data.vkRenderPass[p_Ref._id];
  }
};
}
}
}
}
