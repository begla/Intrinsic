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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

#define MAX_GLOBAL_DESCRIPTORS 4096u

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
namespace
{
VkDescriptorSetLayout _globalDescriptorSetLayout;
VkDescriptorPool _globalDescriptorPool;
VkDescriptorSet _globalDescriptorSet;
uint32_t _globalTextureId = 0u;

_INTR_INLINE void updateGlobalDescriptorSetForSingleImage(ImageRef p_ImageRef)
{
  VkDescriptorImageInfo imageInfo;
  {
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ImageManager::_vkImageView(p_ImageRef);
    imageInfo.sampler = Samplers::samplers[Samplers::kLinearRepeat];
  }

  uint32_t textureId = ImageManager::getTextureId(p_ImageRef);
  if (textureId == (uint32_t)-1)
  {
    textureId = _globalTextureId++;
    ImageManager::_globalTextureIdMapping[p_ImageRef] = textureId;
  }

  VkWriteDescriptorSet write;
  {
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = _globalDescriptorSet;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1u;
    write.pImageInfo = &imageInfo;
    write.dstBinding = 0u;
    write.dstArrayElement = textureId;
  }
  vkUpdateDescriptorSets(RenderSystem::_vkDevice, 1u, &write, 0u, nullptr);
}
}

// Static members
_INTR_HASH_MAP(Dod::Ref, uint32_t) ImageManager::_globalTextureIdMapping;

void ImageManager::init()
{
  _INTR_LOG_INFO("Inititializing Image Manager...");

  Dod::Resources::ResourceManagerBase<
      ImageData, _INTR_MAX_IMAGE_COUNT>::_initResourceManager();

  {
    Dod::Resources::ResourceManagerEntry managerEntry;
    managerEntry.createFunction = Resources::ImageManager::createImage;
    managerEntry.destroyFunction = Resources::ImageManager::destroyImage;
    managerEntry.createResourcesFunction =
        Resources::ImageManager::createResources;
    managerEntry.destroyResourcesFunction =
        Resources::ImageManager::destroyResources;
    managerEntry.getActiveResourceAtIndexFunction =
        Resources::ImageManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        Resources::ImageManager::getActiveResourceCount;
    managerEntry.loadFromMultipleFilesFunction =
        Resources::ImageManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction =
        Resources::ImageManager::saveToMultipleFiles;
    managerEntry.getResourceFlagsFunction = ImageManager::_resourceFlags;
    Application::_resourceManagerMapping[_N(Image)] = managerEntry;
  }

  {
    Dod::PropertyCompilerEntry compilerEntry;
    compilerEntry.compileFunction = Resources::ImageManager::compileDescriptor;
    compilerEntry.initFunction = Resources::ImageManager::initFromDescriptor;
    compilerEntry.ref = Dod::Ref();
    Application::_resourcePropertyCompilerMapping[_N(Image)] = compilerEntry;
  }

  _defaultResourceName = _N(checkerboard);

  // Initializes global descriptor sets
  {
    VkDescriptorSetLayoutBinding globalImageBinding = {};
    {
      globalImageBinding.binding = 0u;
      globalImageBinding.descriptorType =
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      globalImageBinding.descriptorCount = MAX_GLOBAL_DESCRIPTORS;
      globalImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      globalImageBinding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descGlobalLayout = {};
    {
      descGlobalLayout.sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      descGlobalLayout.pNext = nullptr;
      descGlobalLayout.bindingCount = 1u;
      descGlobalLayout.pBindings = &globalImageBinding;

      VkResult result = vkCreateDescriptorSetLayout(
          RenderSystem::_vkDevice, &descGlobalLayout, nullptr,
          &_globalDescriptorSetLayout);
      _INTR_VK_CHECK_RESULT(result);
    }

    VkDescriptorPoolSize poolSize;
    {
      poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      poolSize.descriptorCount = MAX_GLOBAL_DESCRIPTORS;
    }

    VkDescriptorPoolCreateInfo descriptorPool = {};
    descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPool.pNext = nullptr;
    descriptorPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPool.maxSets = 1u;
    descriptorPool.poolSizeCount = 1u;
    descriptorPool.pPoolSizes = &poolSize;

    VkResult result =
        vkCreateDescriptorPool(RenderSystem::_vkDevice, &descriptorPool,
                               nullptr, &_globalDescriptorPool);
    _INTR_VK_CHECK_RESULT(result);

    VkDescriptorSetAllocateInfo allocInfo = {};
    {
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.pNext = nullptr;
      allocInfo.descriptorPool = _globalDescriptorPool;
      allocInfo.descriptorSetCount = 1u;
      allocInfo.pSetLayouts = &_globalDescriptorSetLayout;

      VkResult result = vkAllocateDescriptorSets(
          RenderSystem::_vkDevice, &allocInfo, &_globalDescriptorSet);
      _INTR_VK_CHECK_RESULT(result);
    }
  }
}

// <-

namespace
{
_INTR_INLINE void allocateMemory(GpuMemoryAllocationInfo& p_MemAllocInfo,
                                 MemoryPoolType::Enum p_PoolType,
                                 const VkMemoryRequirements& p_MemReqs)
{
  // Try to keep memory for static images
  bool needsAlloc = true;
  if (p_PoolType >= MemoryPoolType::kRangeStartStatic &&
      p_PoolType <= MemoryPoolType::kRangeEndStatic)
  {
    if (p_MemAllocInfo._sizeInBytes <= p_MemReqs.size &&
        p_MemAllocInfo._alignmentInBytes == p_MemReqs.alignment &&
        p_MemAllocInfo._memoryPoolType == p_PoolType)
    {
      needsAlloc = false;
    }
  }

  if (needsAlloc)
  {
    p_MemAllocInfo = GpuMemoryManager::allocateOffset(
        p_PoolType, (uint32_t)p_MemReqs.size, (uint32_t)p_MemReqs.alignment,
        p_MemReqs.memoryTypeBits);
  }
}
void createTexture(ImageRef p_Ref)
{
  VkImage& vkImage = ImageManager::_vkImage(p_Ref);
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));
  const bool isSrgbFormat = vkFormat == VK_FORMAT_B8G8R8A8_SRGB ||
                            vkFormat == VK_FORMAT_B8G8R8A8_UNORM;

  GpuMemoryAllocationInfo& memoryAllocationInfo =
      ImageManager::_memoryAllocationInfo(p_Ref);
  MemoryPoolType::Enum memoryPoolType =
      ImageManager::_descMemoryPoolType(p_Ref);
  glm::uvec3& dimensions = ImageManager::_descDimensions(p_Ref);
  ImageType::Enum& imageType = ImageManager::_descImageType(p_Ref);
  uint8_t& imageFlags = ImageManager::_descImageFlags(p_Ref);
  uint32_t& mipLevelCount = ImageManager::_descMipLevelCount(p_Ref);
  const uint32_t arrayLayerCount = ImageManager::_descArrayLayerCount(p_Ref);

  _INTR_ASSERT(dimensions.x >= 1.0f && dimensions.y >= 1.0f &&
               dimensions.z >= 1.0f);

  bool isDepthTarget =
      Helper::isFormatDepthFormat(ImageManager::_descImageFormat(p_Ref));
  bool isStencilTarget =
      Helper::isFormatDepthStencilFormat(ImageManager::_descImageFormat(p_Ref));

  VkImageType vkImageType = VK_IMAGE_TYPE_1D;
  VkImageViewType vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_1D;
  VkImageViewType vkImageViewType = arrayLayerCount == 1u
                                        ? VK_IMAGE_VIEW_TYPE_1D
                                        : VK_IMAGE_VIEW_TYPE_1D_ARRAY;
  ImageManager::_imageTextureType(p_Ref) = arrayLayerCount == 1u
                                               ? ImageTextureType::k1D
                                               : ImageTextureType::k1DArray;

  if (dimensions.y >= 2.0f && dimensions.z == 1.0f)
  {
    vkImageType = VK_IMAGE_TYPE_2D;
    vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewType = arrayLayerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D
                                            : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ImageManager::_imageTextureType(p_Ref) = arrayLayerCount == 1u
                                                 ? ImageTextureType::k2D
                                                 : ImageTextureType::k2DArray;
  }
  else if (dimensions.y >= 2.0f && dimensions.z >= 2.0f)
  {
    _INTR_ASSERT(arrayLayerCount == 1u);
    vkImageType = VK_IMAGE_TYPE_3D;
    vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_3D;
    vkImageViewType = VK_IMAGE_VIEW_TYPE_3D;
    ImageManager::_imageTextureType(p_Ref) = ImageTextureType::k3D;
  }

  VkImageCreateInfo imageCreateInfo = {};

  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(RenderSystem::_vkPhysicalDevice, vkFormat,
                                      &props);

  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

  if ((imageFlags & ImageFlags::kUsageAttachment) > 0u)
  {
    if (isDepthTarget || isStencilTarget)
    {
      if (props.linearTilingFeatures &
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      {
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
      }
      else if (props.optimalTilingFeatures &
               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      {
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      }
      else
      {
        _INTR_ASSERT(false && "Depth/stencil format not supported");
      }
    }
    else
    {
      if (props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
      {
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
      }
      else if (props.optimalTilingFeatures &
               VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
      {
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      }
      else
      {
        _INTR_ASSERT(false && "Color format not supported");
      }
    }
  }

  if ((imageFlags & ImageFlags::kUsageSampled) > 0u)
  {
    _INTR_ASSERT((props.optimalTilingFeatures &
                  VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) > 0u &&
                 "Image format does not support sampling");
  }
  if ((imageFlags & ImageFlags::kUsageStorage) > 0u)
  {
    _INTR_ASSERT((props.optimalTilingFeatures &
                  VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) > 0u &&
                 "Image format does not support storing");
  }

  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.pNext = nullptr;
  imageCreateInfo.imageType = vkImageType;
  imageCreateInfo.format = vkFormat;
  imageCreateInfo.extent.width = (uint32_t)dimensions.x;
  imageCreateInfo.extent.height = (uint32_t)dimensions.y;
  imageCreateInfo.extent.depth = (uint32_t)dimensions.z;
  imageCreateInfo.mipLevels = mipLevelCount;
  imageCreateInfo.arrayLayers = arrayLayerCount;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.queueFamilyIndexCount = 0u;
  imageCreateInfo.pQueueFamilyIndices = nullptr;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  // Setup usage
  imageCreateInfo.usage =
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if ((imageFlags & ImageFlags::kUsageAttachment) > 0u)
  {
    imageCreateInfo.usage |= (isDepthTarget || isStencilTarget)
                                 ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                 : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if ((imageFlags & ImageFlags::kUsageSampled) > 0u)
  {
    imageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if ((imageFlags & ImageFlags::kUsageStorage) > 0u)
  {
    imageCreateInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  imageCreateInfo.flags =
      isSrgbFormat ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0u;

  VkResult result = vkCreateImage(RenderSystem::_vkDevice, &imageCreateInfo,
                                  nullptr, &vkImage);
  _INTR_VK_CHECK_RESULT(result);

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  allocateMemory(memoryAllocationInfo, memoryPoolType, memReqs);

  result = vkBindImageMemory(RenderSystem::_vkDevice, vkImage,
                             memoryAllocationInfo._vkDeviceMemory,
                             memoryAllocationInfo._offset);
  _INTR_VK_CHECK_RESULT(result);

  VkImageViewCreateInfo imageViewCreateInfo = {};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.pNext = nullptr;
  imageViewCreateInfo.image = VK_NULL_HANDLE;
  imageViewCreateInfo.format = vkFormat;
  imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
  imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
  imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
  imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

  if (isDepthTarget && isStencilTarget)
  {
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
  else if (!isDepthTarget && !isStencilTarget)
  {
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  else
  {
    _INTR_ASSERT(false);
  }

  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.image = vkImage;

  ImageViewArray& vkSubResourceImageViews =
      ImageManager::_vkSubResourceImageViews(p_Ref);
  vkSubResourceImageViews.resize(arrayLayerCount);
  for (uint32_t arrayLayerIdx = 0u; arrayLayerIdx < arrayLayerCount;
       ++arrayLayerIdx)
  {
    vkSubResourceImageViews[arrayLayerIdx].resize(mipLevelCount);
  }

  // Image views for each sub resource
  for (uint32_t arrayLayerIdx = 0u; arrayLayerIdx < arrayLayerCount;
       ++arrayLayerIdx)
  {
    for (uint32_t mipLevelIdx = 0u; mipLevelIdx < mipLevelCount; ++mipLevelIdx)
    {
      imageViewCreateInfo.viewType = vkImageViewTypeSubResource;
      imageViewCreateInfo.subresourceRange.baseMipLevel = mipLevelIdx;
      imageViewCreateInfo.subresourceRange.levelCount = 1u;
      imageViewCreateInfo.subresourceRange.baseArrayLayer = arrayLayerIdx;
      imageViewCreateInfo.subresourceRange.layerCount = 1u;

      result = vkCreateImageView(
          RenderSystem::_vkDevice, &imageViewCreateInfo, nullptr,
          &vkSubResourceImageViews[arrayLayerIdx][mipLevelIdx]);
      _INTR_VK_CHECK_RESULT(result);
    }
  }

  // General image view
  imageViewCreateInfo.viewType = vkImageViewType;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = mipLevelCount;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
  imageViewCreateInfo.subresourceRange.layerCount = arrayLayerCount;

  result = vkCreateImageView(RenderSystem::_vkDevice, &imageViewCreateInfo,
                             nullptr, &ImageManager::_vkImageView(p_Ref));

  if (isSrgbFormat)
  {
    imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
    result =
        vkCreateImageView(RenderSystem::_vkDevice, &imageViewCreateInfo,
                          nullptr, &ImageManager::_vkImageViewGamma(p_Ref));
    imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    result =
        vkCreateImageView(RenderSystem::_vkDevice, &imageViewCreateInfo,
                          nullptr, &ImageManager::_vkImageViewLinear(p_Ref));
  }

  _INTR_VK_CHECK_RESULT(result);
}

// <-

void createTextureFromFileCubemap(ImageRef p_Ref, gli::texture& p_Texture)
{
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));

  GpuMemoryAllocationInfo& memoryAllocationInfo =
      ImageManager::_memoryAllocationInfo(p_Ref);
  MemoryPoolType::Enum memoryPoolType =
      ImageManager::_descMemoryPoolType(p_Ref);

  gli::texture_cube texCube = gli::texture_cube(p_Texture);
  _INTR_ASSERT(!texCube.empty());

  uint32_t width = static_cast<uint32_t>(texCube[0].extent(0u).x);
  uint32_t height = static_cast<uint32_t>(texCube[0].extent(0u).y);
  uint32_t faces = static_cast<uint32_t>(texCube.faces());
  uint32_t mipLevels = static_cast<uint32_t>(texCube.levels());

  ImageManager::_descDimensions(p_Ref) = glm::uvec3(width, height, 1u);
  ImageManager::_descMipLevelCount(p_Ref) = mipLevels;
  ImageManager::_descArrayLayerCount(p_Ref) = 1u;
  ImageManager::_descImageFlags(p_Ref) = ImageFlags::kUsageSampled;

  VkBuffer stagingBuffer;

  VkBufferCreateInfo bufferCreateInfo = {};
  {
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.size = texCube.size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                   nullptr, &stagingBuffer);
  _INTR_VK_CHECK_RESULT(result);
  VkMemoryRequirements stagingMemReqs;
  vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                &stagingMemReqs);

  GpuMemoryAllocationInfo stagingGpuAllocInfo =
      GpuMemoryManager::allocateOffset(MemoryPoolType::kVolatileStagingBuffers,
                                       (uint32_t)stagingMemReqs.size,
                                       (uint32_t)stagingMemReqs.alignment,
                                       stagingMemReqs.memoryTypeBits);

  result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                              stagingGpuAllocInfo._vkDeviceMemory,
                              stagingGpuAllocInfo._offset);
  _INTR_VK_CHECK_RESULT(result);

  uint8_t* data = stagingGpuAllocInfo._mappedMemory;
  {
    memcpy(data, texCube.data(), texCube.size());
  }

  _INTR_ARRAY(VkBufferImageCopy) bufferCopyRegions;
  uint32_t offset = 0;

  for (uint32_t face = 0; face < faces; face++)
  {
    for (uint32_t mipLevel = 0; mipLevel < mipLevels; mipLevel++)
    {
      VkBufferImageCopy bufferCopyRegion = {};
      bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      bufferCopyRegion.imageSubresource.mipLevel = mipLevel;
      bufferCopyRegion.imageSubresource.baseArrayLayer = face;
      bufferCopyRegion.imageSubresource.layerCount = 1u;
      bufferCopyRegion.imageExtent.width =
          static_cast<uint32_t>(texCube[face].extent(mipLevel).x);
      bufferCopyRegion.imageExtent.height =
          static_cast<uint32_t>(texCube[face].extent(mipLevel).y);
      bufferCopyRegion.imageExtent.depth = 1u;
      bufferCopyRegion.bufferOffset = offset;

      bufferCopyRegions.push_back(bufferCopyRegion);

      offset += static_cast<uint32_t>(texCube[face].size(mipLevel));
    }
  }

  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(RenderSystem::_vkPhysicalDevice, vkFormat,
                                      &props);

  _INTR_ASSERT((props.optimalTilingFeatures &
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) > 0u &&
               "Format does not support optimal tiling");

  VkImageCreateInfo imageCreateInfo = {};
  {
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = vkFormat;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = faces;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {width, height, 1u};
    imageCreateInfo.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  VkImage& vkImage = ImageManager::_vkImage(p_Ref);
  result = vkCreateImage(RenderSystem::_vkDevice, &imageCreateInfo, nullptr,
                         &vkImage);
  _INTR_VK_CHECK_RESULT(result);

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  allocateMemory(memoryAllocationInfo, memoryPoolType, memReqs);

  result = vkBindImageMemory(RenderSystem::_vkDevice, vkImage,
                             memoryAllocationInfo._vkDeviceMemory,
                             memoryAllocationInfo._offset);
  _INTR_VK_CHECK_RESULT(result);

  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = mipLevels;
  subresourceRange.layerCount = faces;

  Helper::insertImageMemoryBarrier(copyCmd, vkImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   subresourceRange);

  vkCmdCopyBufferToImage(copyCmd, stagingBuffer, vkImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(bufferCopyRegions.size()),
                         bufferCopyRegions.data());

  Helper::insertImageMemoryBarrier(
      copyCmd, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

  RenderSystem::flushTemporaryCommandBuffer();

  vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffer, nullptr);
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);

  VkImageViewCreateInfo view = {};
  {
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = nullptr;
    view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    view.format = vkFormat;
    view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                       VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0u;
    view.subresourceRange.baseArrayLayer = 0u;
    view.subresourceRange.layerCount = faces;
    view.subresourceRange.levelCount = mipLevels;
    view.image = vkImage;
  }

  VkImageView vkImageView;
  result =
      vkCreateImageView(RenderSystem::_vkDevice, &view, nullptr, &vkImageView);
  _INTR_VK_CHECK_RESULT(result);

  ImageManager::_vkImageView(p_Ref) = vkImageView;
  ImageManager::_imageTextureType(p_Ref) = ImageTextureType::kCube;
}

// <-

void createTextureFromFile2D(ImageRef p_Ref, gli::texture& p_Texture)
{
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));

  GpuMemoryAllocationInfo& memoryAllocationInfo =
      ImageManager::_memoryAllocationInfo(p_Ref);
  MemoryPoolType::Enum memoryPoolType =
      ImageManager::_descMemoryPoolType(p_Ref);

  gli::texture2d tex2D = gli::texture2d(p_Texture);
  _INTR_ASSERT(!tex2D.empty());

  uint32_t width = static_cast<uint32_t>(tex2D[0].extent().x);
  uint32_t height = static_cast<uint32_t>(tex2D[0].extent().y);
  uint32_t mipLevels = static_cast<uint32_t>(tex2D.levels());

  ImageManager::_descDimensions(p_Ref) = glm::uvec3(width, height, 1u);
  ImageManager::_descMipLevelCount(p_Ref) = mipLevels;
  ImageManager::_descArrayLayerCount(p_Ref) = 1u;
  ImageManager::_descImageFlags(p_Ref) = ImageFlags::kUsageSampled;

  VkBuffer stagingBuffer;

  VkBufferCreateInfo bufferCreateInfo = {};
  {
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.size = tex2D.size();
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  VkResult result = vkCreateBuffer(RenderSystem::_vkDevice, &bufferCreateInfo,
                                   nullptr, &stagingBuffer);
  _INTR_VK_CHECK_RESULT(result);

  VkMemoryRequirements stagingMemReqs;
  vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                &stagingMemReqs);

  GpuMemoryAllocationInfo stagingGpuAllocInfo =
      GpuMemoryManager::allocateOffset(MemoryPoolType::kVolatileStagingBuffers,
                                       (uint32_t)stagingMemReqs.size,
                                       (uint32_t)stagingMemReqs.alignment,
                                       stagingMemReqs.memoryTypeBits);

  result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                              stagingGpuAllocInfo._vkDeviceMemory,
                              stagingGpuAllocInfo._offset);
  _INTR_VK_CHECK_RESULT(result);

  uint8_t* data = stagingGpuAllocInfo._mappedMemory;
  {
    memcpy(data, tex2D.data(), tex2D.size());
  }

  _INTR_ARRAY(VkBufferImageCopy) bufferCopyRegions;
  uint32_t offset = 0;

  for (uint32_t i = 0; i < mipLevels; i++)
  {
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = i;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0u;
    bufferCopyRegion.imageSubresource.layerCount = 1u;
    bufferCopyRegion.imageExtent.width =
        static_cast<uint32_t>(tex2D[i].extent().x);
    bufferCopyRegion.imageExtent.height =
        static_cast<uint32_t>(tex2D[i].extent().y);
    bufferCopyRegion.imageExtent.depth = 1u;
    bufferCopyRegion.bufferOffset = offset;

    bufferCopyRegions.push_back(bufferCopyRegion);

    offset += static_cast<uint32_t>(tex2D[i].size());
  }

  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(RenderSystem::_vkPhysicalDevice, vkFormat,
                                      &props);
  _INTR_ASSERT((props.optimalTilingFeatures &
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) > 0u &&
               "Format does not support optimal tiling");

  VkImageCreateInfo imageCreateInfo = {};
  {
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = vkFormat;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent = {width, height, 1u};
    imageCreateInfo.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  VkImage& vkImage = ImageManager::_vkImage(p_Ref);
  result = vkCreateImage(RenderSystem::_vkDevice, &imageCreateInfo, nullptr,
                         &vkImage);
  _INTR_VK_CHECK_RESULT(result);

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  allocateMemory(memoryAllocationInfo, memoryPoolType, memReqs);

  result = vkBindImageMemory(RenderSystem::_vkDevice, vkImage,
                             memoryAllocationInfo._vkDeviceMemory,
                             memoryAllocationInfo._offset);
  _INTR_VK_CHECK_RESULT(result);

  VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = mipLevels;
  subresourceRange.layerCount = 1;

  Helper::insertImageMemoryBarrier(copyCmd, vkImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   subresourceRange);

  vkCmdCopyBufferToImage(copyCmd, stagingBuffer, vkImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(bufferCopyRegions.size()),
                         bufferCopyRegions.data());

  Helper::insertImageMemoryBarrier(
      copyCmd, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

  RenderSystem::flushTemporaryCommandBuffer();

  vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffer, nullptr);
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);

  VkImageViewCreateInfo view = {};
  {
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = nullptr;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = vkFormat;
    view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                       VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0u;
    view.subresourceRange.baseArrayLayer = 0u;
    view.subresourceRange.layerCount = 1u;
    view.subresourceRange.levelCount = mipLevels;
    view.image = vkImage;
  }

  VkImageView vkImageView;
  result =
      vkCreateImageView(RenderSystem::_vkDevice, &view, nullptr, &vkImageView);
  _INTR_VK_CHECK_RESULT(result);

  ImageManager::_vkImageView(p_Ref) = vkImageView;
  ImageManager::_imageTextureType(p_Ref) = ImageTextureType::k2D;

  updateGlobalDescriptorSetForSingleImage(p_Ref);
}

// <-

void createTextureFromFile(ImageRef p_Ref)
{
  _INTR_STRING texturePath =
      "media/textures/" + ImageManager::_descFileName(p_Ref);

  // Check if the texture exists - if not, use the checkerboard texture as a
  // fallback instead
  if (!Util::fileExists(texturePath.c_str()))
  {
    _INTR_LOG_WARNING(
        "Texture '%s' not found, using checkerboard texture instead...",
        texturePath.c_str());
    texturePath = "media/textures/checkerboard.dds";
  }

  gli::texture tex = gli::load(texturePath.c_str());
  if (tex.target() == gli::target::TARGET_2D)
  {
    createTextureFromFile2D(p_Ref, tex);
  }
  else if (tex.target() == gli::target::TARGET_CUBE)
  {
    createTextureFromFileCubemap(p_Ref, tex);
  }
  else
  {
    _INTR_ASSERT(false && "Unsupported texture type");
  }
}
}

// <-

void ImageManager::createResources(const ImageRefArray& p_Images)
{
  for (uint32_t i = 0u; i < p_Images.size(); ++i)
  {
    ImageRef ref = p_Images[i];

    if (_descImageType(ref) == ImageType::kExternal)
    {
      continue;
    }

    if (_descImageType(ref) == ImageType::kTexture)
    {
      createTexture(ref);
    }
    else if (_descImageType(ref) == ImageType::kTextureFromFile)
    {
      createTextureFromFile(ref);
    }
  }
}

void ImageManager::updateGlobalDescriptorSet()
{
  // Write to global descriptor set
  _INTR_ARRAY(VkDescriptorImageInfo) imageInfos;
  imageInfos.resize(MAX_GLOBAL_DESCRIPTORS);

  VkDescriptorImageInfo defaultImageInfo;
  {
    defaultImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    defaultImageInfo.imageView = ImageManager::_vkImageView(
        ImageManager::getResourceByName(_N(checkerboard)));
    defaultImageInfo.sampler = Samplers::samplers[Samplers::kLinearRepeat];
  }
  std::fill(imageInfos.begin(), imageInfos.end(), defaultImageInfo);

  _globalTextureIdMapping.clear();
  _globalTextureId = 0u;
  for (uint32_t i = 0u; i < _activeRefs.size(); ++i)
  {
    ImageRef imgRef = _activeRefs[i];

    if (_imageTextureType(imgRef) != ImageTextureType::k2D ||
        _descImageType(imgRef) != ImageType::kTextureFromFile)
    {
      continue;
    }

    // Update mapping
    const uint32_t id = _globalTextureId++;
    _globalTextureIdMapping[imgRef] = id;

    VkDescriptorImageInfo& imageInfo = imageInfos[id];
    {
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = ImageManager::_vkImageView(imgRef);
      imageInfo.sampler = Samplers::samplers[Samplers::kLinearRepeat];
    }
  }

  VkWriteDescriptorSet write;
  {
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = _globalDescriptorSet;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = (uint32_t)imageInfos.size();
    write.pImageInfo = imageInfos.data();
    write.dstBinding = 0u;
    write.dstArrayElement = 0u;
  }
  vkUpdateDescriptorSets(RenderSystem::_vkDevice, 1u, &write, 0u, nullptr);
}

VkDescriptorSetLayout ImageManager::getGlobalDescriptorSetLayout()
{
  return _globalDescriptorSetLayout;
}

VkDescriptorSet ImageManager::getGlobalDescriptorSet()
{
  return _globalDescriptorSet;
}
}
}
}
}
