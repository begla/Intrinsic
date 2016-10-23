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
  // Intrinsic

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
