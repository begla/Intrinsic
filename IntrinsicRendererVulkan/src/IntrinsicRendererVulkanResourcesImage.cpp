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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

// Lib. includes
#include "gli.hpp"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
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
    managerEntry.loadFromSingleFileFunction =
        Resources::ImageManager::loadFromSingleFile;
    managerEntry.saveToSingleFileFunction =
        Resources::ImageManager::saveToSingleFile;
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
}

// <-

void createTexture(ImageRef p_Ref)
{
  VkImage& vkImage = ImageManager::_vkImage(p_Ref);
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));
  VkDeviceMemory& vkDeviceMemory = ImageManager::_vkDeviceMemory(p_Ref);
  glm::uvec3& dimensions = ImageManager::_descDimensions(p_Ref);
  ImageType::Enum& imageType = ImageManager::_descImageType(p_Ref);
  uint8_t& imageFlags = ImageManager::_descImageFlags(p_Ref);
  uint32_t& mipLevelCount = ImageManager::_descMipLevelCount(p_Ref);
  const uint32_t arrayLayerCount = ImageManager::_descArrayLayerCount(p_Ref);

  _INTR_ASSERT(dimensions.x >= 1.0f && dimensions.y >= 1.0f &&
               dimensions.z >= 1.0f);

  bool isDepthTarget = vkFormat == VK_FORMAT_D24_UNORM_S8_UINT;
  bool isStencilTarget = vkFormat == VK_FORMAT_D24_UNORM_S8_UINT;

  VkImageType vkImageType = VK_IMAGE_TYPE_1D;
  VkImageViewType vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_1D;
  VkImageViewType vkImageViewType = arrayLayerCount == 1u
                                        ? VK_IMAGE_VIEW_TYPE_1D
                                        : VK_IMAGE_VIEW_TYPE_1D_ARRAY;

  if (dimensions.y >= 2.0f && dimensions.z == 1.0f)
  {
    vkImageType = VK_IMAGE_TYPE_2D;
    vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewType = arrayLayerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D
                                            : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  }
  else if (dimensions.y >= 2.0f && dimensions.z >= 2.0f)
  {
    _INTR_ASSERT(arrayLayerCount == 1u);
    vkImageType = VK_IMAGE_TYPE_3D;
    vkImageViewTypeSubResource = VK_IMAGE_VIEW_TYPE_3D;
    vkImageViewType = VK_IMAGE_VIEW_TYPE_3D;
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
        _INTR_ASSERT(false);
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
        _INTR_ASSERT(false);
      }
    }
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

  imageCreateInfo.flags = 0u;

  VkMemoryRequirements memReqs;
  VkResult result = vkCreateImage(RenderSystem::_vkDevice, &imageCreateInfo,
                                  nullptr, &vkImage);
  _INTR_VK_CHECK_RESULT(result);

  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  VkMemoryAllocateInfo memAllocInfo = {};
  {
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex =
        Helper::computeGpuMemoryTypeIdx(memReqs.memoryTypeBits, 0);
  }

  result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                            &vkDeviceMemory);
  _INTR_VK_CHECK_RESULT(result);

  result =
      vkBindImageMemory(RenderSystem::_vkDevice, vkImage, vkDeviceMemory, 0u);
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
  _INTR_VK_CHECK_RESULT(result);
}

// <-

void createTextureFromFileCubemap(ImageRef p_Ref, gli::texture& p_Texture)
{
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));

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

  VkMemoryAllocateInfo memAllocInfo = {};
  {
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
  }
  VkMemoryRequirements memReqs = {};

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;

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
  vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                &memReqs);

  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = Helper::computeGpuMemoryTypeIdx(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                            &stagingMemory);
  _INTR_VK_CHECK_RESULT(result);

  result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                              stagingMemory, 0);
  _INTR_VK_CHECK_RESULT(result);

  uint8_t* data;
  result = vkMapMemory(RenderSystem::_vkDevice, stagingMemory, 0, memReqs.size,
                       0, (void**)&data);
  _INTR_VK_CHECK_RESULT(result);
  memcpy(data, texCube.data(), texCube.size());
  vkUnmapMemory(RenderSystem::_vkDevice, stagingMemory);

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

  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = Helper::computeGpuMemoryTypeIdx(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkDeviceMemory& vkDeviceMemory = ImageManager::_vkDeviceMemory(p_Ref);
  result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                            &vkDeviceMemory);
  _INTR_VK_CHECK_RESULT(result);

  result =
      vkBindImageMemory(RenderSystem::_vkDevice, vkImage, vkDeviceMemory, 0);
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

  vkFreeMemory(RenderSystem::_vkDevice, stagingMemory, nullptr);
  vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffer, nullptr);

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
}

// <-

void createTextureFromFile2D(ImageRef p_Ref, gli::texture& p_Texture)
{
  VkFormat vkFormat =
      Helper::mapFormatToVkFormat(ImageManager::_descImageFormat(p_Ref));

  gli::texture2d tex2D = gli::texture2d(p_Texture);
  _INTR_ASSERT(!tex2D.empty());

  uint32_t width = static_cast<uint32_t>(tex2D[0].extent().x);
  uint32_t height = static_cast<uint32_t>(tex2D[0].extent().y);
  uint32_t mipLevels = static_cast<uint32_t>(tex2D.levels());

  ImageManager::_descDimensions(p_Ref) = glm::uvec3(width, height, 1u);
  ImageManager::_descMipLevelCount(p_Ref) = mipLevels;
  ImageManager::_descArrayLayerCount(p_Ref) = 1u;
  ImageManager::_descImageFlags(p_Ref) = ImageFlags::kUsageSampled;

  VkMemoryAllocateInfo memAllocInfo = {};
  {
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
  }
  VkMemoryRequirements memReqs = {};

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;

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
  vkGetBufferMemoryRequirements(RenderSystem::_vkDevice, stagingBuffer,
                                &memReqs);

  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = Helper::computeGpuMemoryTypeIdx(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                            &stagingMemory);
  _INTR_VK_CHECK_RESULT(result);

  result = vkBindBufferMemory(RenderSystem::_vkDevice, stagingBuffer,
                              stagingMemory, 0);
  _INTR_VK_CHECK_RESULT(result);

  uint8_t* data;
  result = vkMapMemory(RenderSystem::_vkDevice, stagingMemory, 0, memReqs.size,
                       0, (void**)&data);
  _INTR_VK_CHECK_RESULT(result);
  memcpy(data, tex2D.data(), tex2D.size());
  vkUnmapMemory(RenderSystem::_vkDevice, stagingMemory);

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

  vkGetImageMemoryRequirements(RenderSystem::_vkDevice, vkImage, &memReqs);

  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = Helper::computeGpuMemoryTypeIdx(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VkDeviceMemory& vkDeviceMemory = ImageManager::_vkDeviceMemory(p_Ref);
  result = vkAllocateMemory(RenderSystem::_vkDevice, &memAllocInfo, nullptr,
                            &vkDeviceMemory);
  _INTR_VK_CHECK_RESULT(result);

  result =
      vkBindImageMemory(RenderSystem::_vkDevice, vkImage, vkDeviceMemory, 0);
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

  vkFreeMemory(RenderSystem::_vkDevice, stagingMemory, nullptr);
  vkDestroyBuffer(RenderSystem::_vkDevice, stagingBuffer, nullptr);

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
}

// <-

void createTextureFromFile(ImageRef p_Ref)
{
  gli::texture tex = gli::load(
      ("media/textures/" + ImageManager::_descFileName(p_Ref)).c_str());

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
    _INTR_ASSERT(false);
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
}
}
}
}
