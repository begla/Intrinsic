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
typedef Dod::Ref ImageRef;
typedef _INTR_ARRAY(ImageRef) ImageRefArray;
typedef _INTR_ARRAY(_INTR_ARRAY(VkImageView)) ImageViewArray;

struct ImageData : Dod::Resources::ResourceDataBase
{
  ImageData() : Dod::Resources::ResourceDataBase(_INTR_MAX_IMAGE_COUNT)
  {
    descImageType.resize(_INTR_MAX_IMAGE_COUNT);
    descImageFormat.resize(_INTR_MAX_IMAGE_COUNT);
    descImageFlags.resize(_INTR_MAX_IMAGE_COUNT);
    descDimensions.resize(_INTR_MAX_IMAGE_COUNT);
    descArrayLayerCount.resize(_INTR_MAX_IMAGE_COUNT);
    descMipLevelCount.resize(_INTR_MAX_IMAGE_COUNT);
    descFileName.resize(_INTR_MAX_IMAGE_COUNT);
    descMemoryPoolType.resize(_INTR_MAX_IMAGE_COUNT);

    vkImage.resize(_INTR_MAX_IMAGE_COUNT);
    vkImageView.resize(_INTR_MAX_IMAGE_COUNT);
    vkSubResourceImageViews.resize(_INTR_MAX_IMAGE_COUNT);
    memoryAllocationInfo.resize(_INTR_MAX_IMAGE_COUNT);
  }

  // Description
  _INTR_ARRAY(ImageType::Enum) descImageType;
  _INTR_ARRAY(MemoryPoolType::Enum) descMemoryPoolType;
  _INTR_ARRAY(Format::Enum) descImageFormat;
  _INTR_ARRAY(uint8_t) descImageFlags;
  _INTR_ARRAY(glm::uvec3) descDimensions;
  _INTR_ARRAY(uint32_t) descArrayLayerCount;
  _INTR_ARRAY(uint32_t) descMipLevelCount;
  _INTR_ARRAY(_INTR_STRING) descFileName;

  // Resources
  _INTR_ARRAY(VkImage) vkImage;
  _INTR_ARRAY(VkImageView) vkImageView;
  _INTR_ARRAY(ImageViewArray) vkSubResourceImageViews;
  _INTR_ARRAY(MemoryAllocationInfo) memoryAllocationInfo;
};

struct ImageManager
    : Dod::Resources::ResourceManagerBase<ImageData, _INTR_MAX_IMAGE_COUNT>
{
  static void init();

  _INTR_INLINE static ImageRef createImage(const Name& p_Name)
  {
    ImageRef ref = Dod::Resources::ResourceManagerBase<
        ImageData, _INTR_MAX_IMAGE_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(ImageRef p_Ref)
  {
    _descMemoryPoolType(p_Ref) = MemoryPoolType::kStaticImages;
    _descImageType(p_Ref) = ImageType::kExternal;
    _descImageFormat(p_Ref) = Format::kR32G32B32A32SFloat;
    _descImageFlags(p_Ref) =
        ImageFlags::kUsageAttachment | ImageFlags::kUsageSampled;
    _descDimensions(p_Ref) = glm::uvec3(0u, 0u, 0u);
    _descArrayLayerCount(p_Ref) = 1u;
    _descMipLevelCount(p_Ref) = 1u;
    _descFileName(p_Ref) = "";
  }

  _INTR_INLINE static void destroyImage(ImageRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        ImageData, _INTR_MAX_IMAGE_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(ImageRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        ImageData, _INTR_MAX_IMAGE_COUNT>::_compileDescriptor(p_Ref,
                                                              p_GenerateDesc,
                                                              p_Properties,
                                                              p_Document);

    p_Properties.AddMember(
        "imageType",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          (uint32_t)_descImageType(p_Ref), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "imageFormat",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          (uint32_t)_descImageFormat(p_Ref), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "flags",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          (uint32_t)_descImageFlags(p_Ref), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "dimensions",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          glm::vec3(_descDimensions(p_Ref)), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "arrayLayerCount",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          _descArrayLayerCount(p_Ref), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember(
        "mipLevelCount",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Image), "",
                          _descMipLevelCount(p_Ref), false, true),
        p_Document.GetAllocator());
    p_Properties.AddMember("fileName",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Image), "string",
                                             _descFileName(p_Ref), true, false),
                           p_Document.GetAllocator());
  }

  _INTR_INLINE static void initFromDescriptor(ImageRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        ImageData, _INTR_MAX_IMAGE_COUNT>::_initFromDescriptor(p_Ref,
                                                               p_Properties);

    if (p_Properties.HasMember("imageType"))
      _descImageType(p_Ref) = (ImageType::Enum)JsonHelper::readPropertyUint(
          p_Properties["imageType"]);
    if (p_Properties.HasMember("imageFormat"))
      _descImageFormat(p_Ref) = (Format::Enum)JsonHelper::readPropertyUint(
          p_Properties["imageFormat"]);
    if (p_Properties.HasMember("flags"))
      _descImageFlags(p_Ref) =
          JsonHelper::readPropertyUint(p_Properties["flags"]);
    if (p_Properties.HasMember("dimensions"))
      _descDimensions(p_Ref) =
          JsonHelper::readPropertyVec3(p_Properties["dimensions"]);
    if (p_Properties.HasMember("arrayLayerCount"))
      _descArrayLayerCount(p_Ref) =
          JsonHelper::readPropertyUint(p_Properties["arrayLayerCount"]);
    if (p_Properties.HasMember("mipLevelCount"))
      _descMipLevelCount(p_Ref) =
          JsonHelper::readPropertyUint(p_Properties["mipLevelCount"]);
    if (p_Properties.HasMember("fileName"))
      _descFileName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["fileName"]);
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<ImageData, _INTR_MAX_IMAGE_COUNT>::
        _saveToMultipleFiles<
            rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<ImageData, _INTR_MAX_IMAGE_COUNT>::
        _loadFromMultipleFiles(p_Path, p_Extension, initFromDescriptor,
                               resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const ImageRefArray& p_Images);

  // <-

  _INTR_INLINE static void destroyResources(const ImageRefArray& p_Images)
  {
    for (uint32_t i = 0u; i < p_Images.size(); ++i)
    {
      ImageRef ref = p_Images[i];
      VkImage& vkImage = _vkImage(ref);
      ImageViewArray& vkImageViews = _vkSubResourceImageViews(ref);
      VkImageView& vkImageView = _vkImageView(ref);

      if (!hasImageFlags(ref, ImageFlags::kExternalImage))
      {
        if (vkImage != VK_NULL_HANDLE)
        {
          RenderSystem::releaseResource(_N(VkImage), (void*)vkImage, nullptr);
        }
      }
      vkImage = VK_NULL_HANDLE;

      if (!hasImageFlags(ref, ImageFlags::kExternalView))
      {
        for (uint32_t arrayLayerIdx = 0u; arrayLayerIdx < vkImageViews.size();
             ++arrayLayerIdx)
        {
          for (uint32_t mipIdx = 0u;
               mipIdx < vkImageViews[arrayLayerIdx].size(); ++mipIdx)
          {
            VkImageView vkImageView = vkImageViews[arrayLayerIdx][mipIdx];

            RenderSystem::releaseResource(_N(VkImageView), (void*)vkImageView,
                                          nullptr);
            vkImageView = VK_NULL_HANDLE;
          }
        }

        if (vkImageView != VK_NULL_HANDLE)
        {
          RenderSystem::releaseResource(_N(VkImageView), (void*)vkImageView,
                                        nullptr);
          vkImageView = VK_NULL_HANDLE;
        }
      }
      vkImageViews.clear();
    }
  }

  // <-

  _INTR_INLINE static void
  destroyImagesAndResources(const FramebufferRefArray& p_Images)
  {
    destroyResources(p_Images);

    for (uint32_t i = 0u; i < p_Images.size(); ++i)
    {
      destroyImage(p_Images[i]);
    }
  }

  // <-

  static _INTR_INLINE void insertImageMemoryBarrier(
      ImageRef p_ImageRef, VkImageLayout p_SrcImageLayout,
      VkImageLayout p_DstImageLayout,
      VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VkPipelineStageFlags p_DstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
  {
    VkImageSubresourceRange range;
    range.aspectMask =
        _descImageFormat(p_ImageRef) != RenderSystem::_depthStencilFormatToUse
            ? VK_IMAGE_ASPECT_COLOR_BIT
            : (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    range.baseMipLevel = 0u;
    range.levelCount = _descMipLevelCount(p_ImageRef);
    range.baseArrayLayer = 0u;
    range.layerCount = _descArrayLayerCount(p_ImageRef);

    Helper::insertImageMemoryBarrier(
        RenderSystem::getPrimaryCommandBuffer(), _vkImage(p_ImageRef),
        p_SrcImageLayout, p_DstImageLayout, range, p_SrcStages, p_DstStages);
  }

  // <-

  static _INTR_INLINE void insertImageMemoryBarrierSubResource(
      ImageRef p_ImageRef, VkImageLayout p_SrcImageLayout,
      VkImageLayout p_DstImageLayout, uint32_t p_MipLevel,
      uint32_t p_ArrayLayer,
      VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VkPipelineStageFlags p_DstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
  {
    VkImageSubresourceRange range;
    range.aspectMask =
        _descImageFormat(p_ImageRef) != RenderSystem::_depthStencilFormatToUse
            ? VK_IMAGE_ASPECT_COLOR_BIT
            : (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    range.baseMipLevel = p_MipLevel;
    range.levelCount = 1u;
    range.baseArrayLayer = p_ArrayLayer;
    range.layerCount = 1u;

    Helper::insertImageMemoryBarrier(
        RenderSystem::getPrimaryCommandBuffer(), _vkImage(p_ImageRef),
        p_SrcImageLayout, p_DstImageLayout, range, p_SrcStages, p_DstStages);
  }

  // <-

  _INTR_INLINE static bool hasImageFlags(ImageRef p_Ref, uint8_t p_Flags)
  {
    return (_data.descImageFlags[p_Ref._id] & p_Flags) == p_Flags;
  }
  _INTR_INLINE static void addImageFlags(ImageRef p_Ref, uint8_t p_Flags)
  {
    _data.descImageFlags[p_Ref._id] |= p_Flags;
  }
  _INTR_INLINE static void removeImageFlags(ImageRef p_Ref, uint8_t p_Flags)
  {
    _data.descImageFlags[p_Ref._id] &= ~p_Flags;
  }

  // Member refs.
  // ->

  // Description
  _INTR_INLINE static MemoryPoolType::Enum& _descMemoryPoolType(ImageRef p_Ref)
  {
    return _data.descMemoryPoolType[p_Ref._id];
  }
  _INTR_INLINE static ImageType::Enum& _descImageType(ImageRef p_Ref)
  {
    return _data.descImageType[p_Ref._id];
  }
  _INTR_INLINE static Format::Enum& _descImageFormat(ImageRef p_Ref)
  {
    return _data.descImageFormat[p_Ref._id];
  }
  _INTR_INLINE static uint8_t& _descImageFlags(ImageRef p_Ref)
  {
    return _data.descImageFlags[p_Ref._id];
  }
  _INTR_INLINE static glm::uvec3& _descDimensions(ImageRef p_Ref)
  {
    return _data.descDimensions[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descArrayLayerCount(ImageRef p_Ref)
  {
    return _data.descArrayLayerCount[p_Ref._id];
  }
  _INTR_INLINE static uint32_t& _descMipLevelCount(ImageRef p_Ref)
  {
    return _data.descMipLevelCount[p_Ref._id];
  }
  _INTR_INLINE static _INTR_STRING& _descFileName(ImageRef p_Ref)
  {
    return _data.descFileName[p_Ref._id];
  }

  // GPU resources
  _INTR_INLINE static VkImage& _vkImage(ImageRef p_Ref)
  {
    return _data.vkImage[p_Ref._id];
  }
  _INTR_INLINE static VkImageView& _vkImageView(ImageRef p_Ref)
  {
    return _data.vkImageView[p_Ref._id];
  }
  _INTR_INLINE static VkImageView&
  _vkSubResourceImageView(ImageRef p_Ref, uint32_t p_ArrayLayerIndex,
                          uint32_t p_MipLevelIdx)
  {
    return _data
        .vkSubResourceImageViews[p_Ref._id][p_ArrayLayerIndex][p_MipLevelIdx];
  }
  _INTR_INLINE static ImageViewArray& _vkSubResourceImageViews(ImageRef p_Ref)
  {
    return _data.vkSubResourceImageViews[p_Ref._id];
  }
  _INTR_INLINE static MemoryAllocationInfo&
  _memoryAllocationInfo(ImageRef p_Ref)
  {
    return _data.memoryAllocationInfo[p_Ref._id];
  }
};
}
}
}
}
