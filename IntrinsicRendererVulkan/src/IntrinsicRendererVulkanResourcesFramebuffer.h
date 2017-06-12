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
namespace Resources
{
typedef Dod::Ref FramebufferRef;
typedef _INTR_ARRAY(FramebufferRef) FramebufferRefArray;
typedef _INTR_ARRAY(AttachmentInfo) AttachmentInfoArray;

struct FramebufferData : Dod::Resources::ResourceDataBase
{
  FramebufferData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_FRAMEBUFFER_COUNT)
  {
    descRenderPass.resize(_INTR_MAX_FRAMEBUFFER_COUNT);
    descAttachedImages.resize(_INTR_MAX_FRAMEBUFFER_COUNT);
    descDimensions.resize(_INTR_MAX_FRAMEBUFFER_COUNT);

    vkFramebuffer.resize(_INTR_MAX_FRAMEBUFFER_COUNT);
  }

  _INTR_ARRAY(Resources::RenderPassRef) descRenderPass;
  _INTR_ARRAY(AttachmentInfoArray) descAttachedImages;
  _INTR_ARRAY(glm::uvec2) descDimensions;

  // GPU resources
  _INTR_ARRAY(VkFramebuffer) vkFramebuffer;
};

struct FramebufferManager
    : Dod::Resources::ResourceManagerBase<FramebufferData,
                                          _INTR_MAX_FRAMEBUFFER_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Framebuffer Manager...");

    Dod::Resources::ResourceManagerBase<
        FramebufferData, _INTR_MAX_FRAMEBUFFER_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static FramebufferRef createFramebuffer(const Name& p_Name)
  {
    FramebufferRef ref = Dod::Resources::ResourceManagerBase<
        FramebufferData, _INTR_MAX_FRAMEBUFFER_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(FramebufferRef p_Ref)
  {
    _descRenderPass(p_Ref) = Resources::RenderPassRef();
    _descAttachedImages(p_Ref).clear();
    _descDimensions(p_Ref) = glm::uvec2(0u, 0u);
  }

  _INTR_INLINE static void destroyFramebuffer(FramebufferRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        FramebufferData, _INTR_MAX_FRAMEBUFFER_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(FramebufferRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        FramebufferData,
        _INTR_MAX_FRAMEBUFFER_COUNT>::_compileDescriptor(p_Ref, p_GenerateDesc,
                                                         p_Properties,
                                                         p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(FramebufferRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        FramebufferData,
        _INTR_MAX_FRAMEBUFFER_COUNT>::_initFromDescriptor(p_Ref, p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        FramebufferData,
        _INTR_MAX_FRAMEBUFFER_COUNT>::_saveToSingleFile(p_FileName,
                                                        compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        FramebufferData,
        _INTR_MAX_FRAMEBUFFER_COUNT>::_loadFromSingleFile(p_FileName,
                                                          initFromDescriptor,
                                                          resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const FramebufferRefArray& p_Framebuffers);

  _INTR_INLINE static void
  destroyResources(const FramebufferRefArray& p_Framebuffers)
  {
    for (uint32_t i = 0u; i < p_Framebuffers.size(); ++i)
    {
      FramebufferRef ref = p_Framebuffers[i];
      VkFramebuffer& framebuffer = _vkFrameBuffer(ref);

      if (framebuffer != VK_NULL_HANDLE)
      {
        vkDestroyFramebuffer(RenderSystem::_vkDevice, framebuffer, nullptr);
        framebuffer = VK_NULL_HANDLE;
      }
    }
  }

  _INTR_INLINE static void
  destroyFramebuffersAndResources(const FramebufferRefArray& p_Framebuffers)
  {
    destroyResources(p_Framebuffers);

    for (uint32_t i = 0u; i < p_Framebuffers.size(); ++i)
    {
      destroyFramebuffer(p_Framebuffers[i]);
    }
  }

  // Accessors
  // ->
  _INTR_INLINE static Resources::RenderPassRef&
  _descRenderPass(FramebufferRef p_Ref)
  {
    return _data.descRenderPass[p_Ref._id];
  }
  _INTR_INLINE static AttachmentInfoArray&
  _descAttachedImages(FramebufferRef p_Ref)
  {
    return _data.descAttachedImages[p_Ref._id];
  }
  _INTR_INLINE static glm::uvec2& _descDimensions(FramebufferRef p_Ref)
  {
    return _data.descDimensions[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkFramebuffer& _vkFrameBuffer(FramebufferRef p_Ref)
  {
    return _data.vkFramebuffer[p_Ref._id];
  }
};
}
}
}
}
