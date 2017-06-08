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
namespace Helper
{
_INTR_INLINE void initResource(TBuiltInResource& p_Resource)
{
  p_Resource.maxLights = 32;
  p_Resource.maxClipPlanes = 6;
  p_Resource.maxTextureUnits = 32;
  p_Resource.maxTextureCoords = 32;
  p_Resource.maxVertexAttribs = 64;
  p_Resource.maxVertexUniformComponents = 4096;
  p_Resource.maxVaryingFloats = 64;
  p_Resource.maxVertexTextureImageUnits = 32;
  p_Resource.maxCombinedTextureImageUnits = 4096;
  p_Resource.maxTextureImageUnits = 32;
  p_Resource.maxFragmentUniformComponents = 4096;
  p_Resource.maxDrawBuffers = 32;
  p_Resource.maxVertexUniformVectors = 128;
  p_Resource.maxVaryingVectors = 8;
  p_Resource.maxFragmentUniformVectors = 16;
  p_Resource.maxVertexOutputVectors = 16;
  p_Resource.maxFragmentInputVectors = 15;
  p_Resource.minProgramTexelOffset = -8;
  p_Resource.maxProgramTexelOffset = 7;
  p_Resource.maxClipDistances = 8;
  p_Resource.maxComputeWorkGroupCountX = 65535;
  p_Resource.maxComputeWorkGroupCountY = 65535;
  p_Resource.maxComputeWorkGroupCountZ = 65535;
  p_Resource.maxComputeWorkGroupSizeX = 1024;
  p_Resource.maxComputeWorkGroupSizeY = 1024;
  p_Resource.maxComputeWorkGroupSizeZ = 64;
  p_Resource.maxComputeUniformComponents = 1024;
  p_Resource.maxComputeTextureImageUnits = 16;
  p_Resource.maxComputeImageUniforms = 8;
  p_Resource.maxComputeAtomicCounters = 8;
  p_Resource.maxComputeAtomicCounterBuffers = 1;
  p_Resource.maxVaryingComponents = 60;
  p_Resource.maxVertexOutputComponents = 64;
  p_Resource.maxGeometryInputComponents = 64;
  p_Resource.maxGeometryOutputComponents = 128;
  p_Resource.maxFragmentInputComponents = 128;
  p_Resource.maxImageUnits = 8;
  p_Resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
  p_Resource.maxCombinedShaderOutputResources = 8;
  p_Resource.maxImageSamples = 0;
  p_Resource.maxVertexImageUniforms = 0;
  p_Resource.maxTessControlImageUniforms = 0;
  p_Resource.maxTessEvaluationImageUniforms = 0;
  p_Resource.maxGeometryImageUniforms = 0;
  p_Resource.maxFragmentImageUniforms = 8;
  p_Resource.maxCombinedImageUniforms = 8;
  p_Resource.maxGeometryTextureImageUnits = 16;
  p_Resource.maxGeometryOutputVertices = 256;
  p_Resource.maxGeometryTotalOutputComponents = 1024;
  p_Resource.maxGeometryUniformComponents = 1024;
  p_Resource.maxGeometryVaryingComponents = 64;
  p_Resource.maxTessControlInputComponents = 128;
  p_Resource.maxTessControlOutputComponents = 128;
  p_Resource.maxTessControlTextureImageUnits = 16;
  p_Resource.maxTessControlUniformComponents = 1024;
  p_Resource.maxTessControlTotalOutputComponents = 4096;
  p_Resource.maxTessEvaluationInputComponents = 128;
  p_Resource.maxTessEvaluationOutputComponents = 128;
  p_Resource.maxTessEvaluationTextureImageUnits = 16;
  p_Resource.maxTessEvaluationUniformComponents = 1024;
  p_Resource.maxTessPatchComponents = 120;
  p_Resource.maxPatchVertices = 32;
  p_Resource.maxTessGenLevel = 64;
  p_Resource.maxViewports = 16;
  p_Resource.maxVertexAtomicCounters = 0;
  p_Resource.maxTessControlAtomicCounters = 0;
  p_Resource.maxTessEvaluationAtomicCounters = 0;
  p_Resource.maxGeometryAtomicCounters = 0;
  p_Resource.maxFragmentAtomicCounters = 8;
  p_Resource.maxCombinedAtomicCounters = 8;
  p_Resource.maxAtomicCounterBindings = 1;
  p_Resource.maxVertexAtomicCounterBuffers = 0;
  p_Resource.maxTessControlAtomicCounterBuffers = 0;
  p_Resource.maxTessEvaluationAtomicCounterBuffers = 0;
  p_Resource.maxGeometryAtomicCounterBuffers = 0;
  p_Resource.maxFragmentAtomicCounterBuffers = 1;
  p_Resource.maxCombinedAtomicCounterBuffers = 1;
  p_Resource.maxAtomicCounterBufferSize = 16384;
  p_Resource.maxTransformFeedbackBuffers = 4;
  p_Resource.maxTransformFeedbackInterleavedComponents = 64;
  p_Resource.maxCullDistances = 8;
  p_Resource.maxCombinedClipAndCullDistances = 8;
  p_Resource.maxSamples = 4;
  p_Resource.limits.nonInductiveForLoops = 1;
  p_Resource.limits.whileLoops = 1;
  p_Resource.limits.doWhileLoops = 1;
  p_Resource.limits.generalUniformIndexing = 1;
  p_Resource.limits.generalAttributeMatrixVectorIndexing = 1;
  p_Resource.limits.generalVaryingIndexing = 1;
  p_Resource.limits.generalSamplerIndexing = 1;
  p_Resource.limits.generalVariableIndexing = 1;
  p_Resource.limits.generalConstantMatrixVectorIndexing = 1;
}

// <-

_INTR_INLINE VkShaderStageFlagBits
mapGpuProgramTypeToVkShaderStage(GpuProgramType::Enum p_Type)
{
  switch (p_Type)
  {
  case GpuProgramType::kVertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case GpuProgramType::kFragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case GpuProgramType::kGeometry:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case GpuProgramType::kCompute:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  }

  _INTR_ASSERT(false && "Failed to map GPU program type");
  return VK_SHADER_STAGE_VERTEX_BIT;
}

// <-

_INTR_INLINE EShLanguage mapGpuProgramTypeToEshLang(GpuProgramType::Enum p_Type)
{
  switch (p_Type)
  {
  case GpuProgramType::kVertex:
    return EShLangVertex;
  case GpuProgramType::kFragment:
    return EShLangFragment;
  case GpuProgramType::kGeometry:
    return EShLangGeometry;
  case GpuProgramType::kCompute:
    return EShLangCompute;
  }

  _INTR_ASSERT(false && "Failed to map GPU program type");
  return EShLangVertex;
}

// <-

_INTR_INLINE VkBufferUsageFlagBits
mapBufferTypeToVkUsageFlagBits(BufferType::Enum p_BufferType)
{
  switch (p_BufferType)
  {
  case BufferType::kVertex:
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  case BufferType::kIndex32:
  case BufferType::kIndex16:
    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  case BufferType::kUniform:
    return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  case BufferType::kStorage:
    return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }

  _INTR_ASSERT(false && "Failed to map buffer type");
  return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
}

// <-

_INTR_INLINE VkDescriptorType
mapBindingTypeToVkDescriptorType(BindingType::Enum p_BindingType)
{
  switch (p_BindingType)
  {
  case BindingType::kUniformBuffer:
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  case BindingType::kUniformBufferDynamic:
    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  case BindingType::kImageAndSamplerCombined:
    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  case BindingType::kSampledImage:
    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  case BindingType::kSampler:
    return VK_DESCRIPTOR_TYPE_SAMPLER;
  case BindingType::kStorageBuffer:
    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  case BindingType::kStorageImage:
    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  }

  _INTR_ASSERT(false && "Failed to map binding type");
  return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

// <-

_INTR_INLINE VkFormat mapFormatToVkFormat(Format::Enum p_Format)
{
  switch (p_Format)
  {
  case Format::kR32G32B32SFloat:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case Format::kR32G32B32A32SFloat:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case Format::kR32G32SFloat:
    return VK_FORMAT_R32G32_SFLOAT;
  case Format::kR16G16Float:
    return VK_FORMAT_R16G16_SFLOAT;
  case Format::kR16G16B16Float:
    return VK_FORMAT_R16G16B16_SFLOAT;
  case Format::kR16G16B16A16Float:
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  case Format::kR32SFloat:
    return VK_FORMAT_R32_SFLOAT;
  case Format::kR32UInt:
    return VK_FORMAT_R32_UINT;
  case Format::kB10G11R11UFloat:
    return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

  case Format::kD24UnormS8UInt:
    return VK_FORMAT_D24_UNORM_S8_UINT;
  case Format::kD32SFloatS8UInt:
    return VK_FORMAT_D32_SFLOAT_S8_UINT;
  case Format::kD32SFloat:
    return VK_FORMAT_D32_SFLOAT;
  case Format::kD16UNorm:
    return VK_FORMAT_D16_UNORM;
  case Format::kD16UnormS8UInt:
    return VK_FORMAT_D16_UNORM_S8_UINT;

  case Format::kB8G8R8A8UNorm:
    return VK_FORMAT_B8G8R8A8_UNORM;
  case Format::kB8G8R8A8Srgb:
    return VK_FORMAT_B8G8R8A8_SRGB;

  case Format::kBC1RGBUNorm:
    return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
  case Format::kBC1RGBSrgb:
    return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
  case Format::kBC2UNorm:
    return VK_FORMAT_BC2_UNORM_BLOCK;
  case Format::kBC2Srgb:
    return VK_FORMAT_BC2_SRGB_BLOCK;
  case Format::kBC3UNorm:
    return VK_FORMAT_BC3_UNORM_BLOCK;
  case Format::kBC3Srgb:
    return VK_FORMAT_BC3_SRGB_BLOCK;
  case Format::kBC5UNorm:
    return VK_FORMAT_BC5_UNORM_BLOCK;
  case Format::kBC5SNorm:
    return VK_FORMAT_BC5_SNORM_BLOCK;
  case Format::kBC6UFloat:
    return VK_FORMAT_BC6H_UFLOAT_BLOCK;

  case Format::kR8UNorm:
    return VK_FORMAT_R8_UNORM;
  }

  _INTR_ASSERT(false && "Failed to map format");
  return VK_FORMAT_R32G32B32_SFLOAT;
};

// <-

_INTR_INLINE bool isFormatDepthStencilFormat(Format::Enum p_Format)
{
  switch (p_Format)
  {
  case Format::kD24UnormS8UInt:
    return true;
  case Format::kD32SFloatS8UInt:
    return true;
  case Format::kD16UnormS8UInt:
    return true;
  }

  return false;
};

_INTR_INLINE bool isFormatDepthFormat(Format::Enum p_Format)
{
  switch (p_Format)
  {
  case Format::kD24UnormS8UInt:
    return true;
  case Format::kD32SFloatS8UInt:
    return true;
  case Format::kD16UnormS8UInt:
    return true;
  case Format::kD32SFloat:
    return true;
  case Format::kD16UNorm:
    return true;
  }

  return false;
}

// <-

_INTR_INLINE void createDefaultMeshVertexLayout(Dod::Ref& p_VertexLayoutToInit)
{
  // Position
  VertexBinding bindPos = {};
  {
    bindPos.binding = 0u;
    bindPos.stride = 6u;
  }

  VertexAttribute attrPos = {};
  {
    attrPos.binding = 0u;
    attrPos.format = Format::kR16G16B16A16Float;
    attrPos.location = 0u;
    attrPos.offset = 0u;
  }

  // UV0
  VertexBinding bindUv0 = {};
  {
    bindUv0.binding = 1u;
    bindUv0.stride = 4u;
  }

  VertexAttribute attrUv0 = {};
  {
    attrUv0.binding = 1u;
    attrUv0.format = Format::kR16G16Float;
    attrUv0.location = 1u;
    attrUv0.offset = 0u;
  }

  // Normal
  VertexBinding bindNormal = {};
  {
    bindNormal.binding = 2u;
    bindNormal.stride = 6u;
  }

  VertexAttribute attrNormal = {};
  {
    attrNormal.binding = 2u;
    attrNormal.format = Format::kR16G16B16A16Float;
    attrNormal.location = 2u;
    attrNormal.offset = 0u;
  }

  // Tangent
  VertexBinding bindTangent = {};
  {
    bindTangent.binding = 3u;
    bindTangent.stride = 6u;
  }

  VertexAttribute attrTangent = {};
  {
    attrTangent.binding = 3u;
    attrTangent.format = Format::kR16G16B16A16Float;
    attrTangent.location = 3u;
    attrTangent.offset = 0u;
  }

  // Binormal
  VertexBinding bindBinormal = {};
  {
    bindBinormal.binding = 4u;
    bindBinormal.stride = 6u;
  }

  VertexAttribute attrBinormal = {};
  {
    attrBinormal.binding = 4u;
    attrBinormal.format = Format::kR16G16B16A16Float;
    attrBinormal.location = 4u;
    attrBinormal.offset = 0u;
  }

  // VertexColor
  VertexBinding bindVtxColor = {};
  {
    bindVtxColor.binding = 5u;
    bindVtxColor.stride = 4u;
  }

  VertexAttribute attrVtxColor = {};
  {
    attrVtxColor.binding = 5u;
    attrVtxColor.format = Format::kB8G8R8A8UNorm;
    attrVtxColor.location = 5u;
    attrVtxColor.offset = 0u;
  }

  _INTR_ARRAY(VertexBinding)& vertexBindings =
      Resources::VertexLayoutManager::_descVertexBindings(p_VertexLayoutToInit);
  _INTR_ARRAY(VertexAttribute)& vertexAttributes =
      Resources::VertexLayoutManager::_descVertexAttributes(
          p_VertexLayoutToInit);

  vertexBindings.push_back(bindPos);
  vertexBindings.push_back(bindUv0);
  vertexBindings.push_back(bindNormal);
  vertexBindings.push_back(bindTangent);
  vertexBindings.push_back(bindBinormal);
  vertexBindings.push_back(bindVtxColor);

  vertexAttributes.push_back(attrPos);
  vertexAttributes.push_back(attrUv0);
  vertexAttributes.push_back(attrNormal);
  vertexAttributes.push_back(attrBinormal);
  vertexAttributes.push_back(attrTangent);
  vertexAttributes.push_back(attrVtxColor);
}

// <-

_INTR_INLINE void createDebugLineVertexLayout(Dod::Ref& p_VertexLayoutToInit)
{
  VertexBinding bind = {};
  {
    bind.binding = 0u;
    bind.stride = 16u;
  }

  // Position
  VertexAttribute attrPos = {};
  {
    attrPos.binding = 0u;
    attrPos.format = Format::kR32G32B32SFloat;
    attrPos.location = 0u;
    attrPos.offset = 0u;
  }

  // VertexColor
  VertexAttribute attrVtxColor = {};
  {
    attrVtxColor.binding = 0u;
    attrVtxColor.format = Format::kB8G8R8A8UNorm;
    attrVtxColor.location = 1u;
    attrVtxColor.offset = 12u;
  }

  Resources::VertexLayoutManager::_descVertexBindings(p_VertexLayoutToInit)
      .push_back(bind);

  Resources::VertexLayoutManager::_descVertexAttributes(p_VertexLayoutToInit)
      .push_back(attrPos);
  Resources::VertexLayoutManager::_descVertexAttributes(p_VertexLayoutToInit)
      .push_back(attrVtxColor);
}

// <-

_INTR_INLINE static void updateAccessMask(VkAccessFlags& p_AccessFlags,
                                          VkImageLayout p_ImageLayout)
{
  if (p_ImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
    p_AccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (p_ImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    p_AccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }

  else if (p_ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    p_AccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if (p_ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
    p_AccessFlags = VK_ACCESS_TRANSFER_READ_BIT;
  }

  else if (p_ImageLayout == VK_IMAGE_LAYOUT_GENERAL)
  {
    p_AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
  }
  else if (p_ImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    p_AccessFlags = VK_ACCESS_SHADER_READ_BIT;
  }
}

// <-

_INTR_INLINE static void insertImageMemoryBarrier(
    VkCommandBuffer p_CommandBuffer, VkImage p_Image,
    VkImageLayout p_OldImageLayout, VkImageLayout p_NewImageLayout,
    VkImageSubresourceRange p_SubresourceRange,
    VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VkPipelineStageFlags p_DestStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
{
  _INTR_ASSERT(p_CommandBuffer != nullptr);

  VkImageMemoryBarrier imageMemoryBarrier = {};
  {
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
    updateAccessMask(imageMemoryBarrier.srcAccessMask, p_OldImageLayout);
    updateAccessMask(imageMemoryBarrier.dstAccessMask, p_NewImageLayout);
    imageMemoryBarrier.oldLayout = p_OldImageLayout;
    imageMemoryBarrier.newLayout = p_NewImageLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = p_Image;
    imageMemoryBarrier.subresourceRange = p_SubresourceRange;
  }

  vkCmdPipelineBarrier(p_CommandBuffer, p_SrcStages, p_DestStages, 0u, 0u,
                       nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);
}

// <-

_INTR_INLINE static void insertImageMemoryBarrier(
    VkCommandBuffer p_CommandBuffer, VkImage p_Image,
    VkImageAspectFlags p_AspectMask, VkImageLayout p_OldImageLayout,
    VkImageLayout p_NewImageLayout,
    VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VkPipelineStageFlags p_DstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
{
  VkImageSubresourceRange range;
  range.aspectMask = p_AspectMask;
  range.baseMipLevel = 0u;
  range.levelCount = 1u;
  range.baseArrayLayer = 0u;
  range.layerCount = 1u;

  insertImageMemoryBarrier(p_CommandBuffer, p_Image, p_OldImageLayout,
                           p_NewImageLayout, range, p_SrcStages, p_DstStages);
}

_INTR_INLINE static void insertBufferMemoryBarrier(
    VkCommandBuffer p_CommandBuffer, VkBuffer p_Buffer, uint32_t p_SizeInBytes,
    uint32_t p_OffsetInBytes, VkAccessFlags p_SrcAccessMask,
    VkAccessFlags p_DstAccessMask,
    VkPipelineStageFlags p_SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VkPipelineStageFlags p_DstStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
{
  VkBufferMemoryBarrier bufferMemoryBarrier = {};
  {
    bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferMemoryBarrier.pNext = nullptr;
    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.buffer = p_Buffer;
    bufferMemoryBarrier.offset = p_OffsetInBytes;
    bufferMemoryBarrier.size = p_SizeInBytes;
    bufferMemoryBarrier.srcAccessMask = p_SrcAccessMask;
    bufferMemoryBarrier.dstAccessMask = p_DstAccessMask;
  }

  vkCmdPipelineBarrier(p_CommandBuffer, p_SrcStages, p_DstStages, 0u, 0u,
                       nullptr, 1u, &bufferMemoryBarrier, 0u, nullptr);
}

// <-

_INTR_INLINE static uint32_t computeGpuMemoryTypeIdx(uint32_t p_TypeBits,
                                                     VkFlags p_RequirementsMask)
{
  uint32_t memoryTypeIndex = (uint32_t)-1;

  for (uint32_t i = 0;
       i < RenderSystem::_vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
  {
    const uint32_t typeIndexMask = 1u << i;

    if ((p_TypeBits & typeIndexMask) == typeIndexMask)
    {
      if ((RenderSystem::_vkPhysicalDeviceMemoryProperties.memoryTypes[i]
               .propertyFlags &
           p_RequirementsMask) == p_RequirementsMask)
      {
        return i;
      }
    }
  }

  _INTR_ASSERT(false);
  return (uint32_t)-1;
}

// <-

_INTR_INLINE static Format::Enum mapFormat(const _INTR_STRING& p_Format)
{
  static _INTR_HASH_MAP(Name, Format::Enum)
      formats = {{"R32G32B32SFloat", Format::kR32G32B32SFloat},
                 {"R32G32B32A32SFloat", Format::kR32G32B32A32SFloat},
                 {"R32G32SFloat", Format::kR32G32SFloat},
                 {"R16G16B16Float", Format::kR16G16B16Float},
                 {"R16G16Float", Format::kR16G16Float},
                 {"B8G8R8A8UNorm", Format::kB8G8R8A8UNorm},
                 {"BC1RGBUNorm", Format::kBC1RGBUNorm},
                 {"BC1RGBSrgb", Format::kBC1RGBSrgb},
                 {"BC2UNorm", Format::kBC2UNorm},
                 {"BC2Srgb", Format::kBC2Srgb},
                 {"BC5UNorm", Format::kBC5UNorm},
                 {"BC5SNorm", Format::kBC5SNorm},
                 {"D24UnormS8UInt", Format::kD24UnormS8UInt},
                 {"B8G8R8A8Srgb", Format::kB8G8R8A8Srgb},
                 {"BC6UFloat", Format::kBC6UFloat},
                 {"BC3UNorm", Format::kBC3UNorm},
                 {"BC3Srgb", Format::kBC3Srgb},
                 {"R32SFloat", Format::kR32SFloat},
                 {"R32UInt", Format::kR32UInt},
                 {"D16UnormS8UInt", Format::kD16UnormS8UInt},
                 {"D32SFloatS8UInt", Format::kD32SFloatS8UInt},
                 {"D32SFloat", Format::kD32SFloat},
                 {"D16UNorm", Format::kD16UNorm},
                 {"R16G16B16A16Float", Format::kR16G16B16A16Float},
                 {"B10G11R11UFloat", Format::kB10G11R11UFloat}};

  auto format = formats.find(p_Format);
  if (format != formats.end())
  {
    return format->second;
  }

  _INTR_ASSERT(false && "Format not supported/found");
  return Format::kB8G8R8A8UNorm;
}

// <-

_INTR_INLINE static RenderSize::Enum
mapRenderSize(const _INTR_STRING& p_RenderSize)
{
  static _INTR_HASH_MAP(Name, RenderSize::Enum)
      renderSizes = {{"Full", RenderSize::kFull},
                     {"Half", RenderSize::kHalf},
                     {"Quarter", RenderSize::kQuarter},
                     {"Cubemap", RenderSize::kCubemap}};

  auto renderSize = renderSizes.find(p_RenderSize);
  if (renderSize != renderSizes.end())
  {
    return renderSize->second;
  }

  _INTR_ASSERT(false && "Render size not supported/found");
  return RenderSize::kFull;
}

// <-

_INTR_INLINE static GpuProgramType::Enum
mapGpuProgramType(const _INTR_STRING& p_GpuProgramType)
{
  static _INTR_HASH_MAP(Name, GpuProgramType::Enum)
      gpuProgramTypes = {{"Fragment", GpuProgramType::kFragment},
                         {"Vertex", GpuProgramType::kVertex},
                         {"Compute", GpuProgramType::kCompute}};

  auto gpuProgramType = gpuProgramTypes.find(p_GpuProgramType);
  if (gpuProgramType != gpuProgramTypes.end())
  {
    return gpuProgramType->second;
  }

  _INTR_ASSERT(false && "GPU program type not supported/found");
  return GpuProgramType::kVertex;
}

// <-

_INTR_INLINE static Samplers::Enum mapSampler(const _INTR_STRING& p_Sampler)
{
  static _INTR_HASH_MAP(Name, Samplers::Enum)
      samplers = {{"LinearClamp", Samplers::kLinearClamp},
                  {"LinearRepeat", Samplers::kLinearRepeat},
                  {"NearestClamp", Samplers::kNearestClamp},
                  {"NearestRepeat", Samplers::kNearestRepeat},
                  {"Shadow", Samplers::kShadow}};

  auto sampler = samplers.find(p_Sampler);
  if (sampler != samplers.end())
  {
    return sampler->second;
  }

  _INTR_ASSERT(false && "GPU program type not supported/found");
  return Samplers::kLinearClamp;
}

// <-

_INTR_INLINE static VkImageLayout
mapImageLayout(const _INTR_STRING& p_ImageLayout)
{
  static _INTR_HASH_MAP(Name, VkImageLayout) imageLayouts = {
      {"Undefined", VK_IMAGE_LAYOUT_UNDEFINED},
      {"General", VK_IMAGE_LAYOUT_GENERAL},
      {"ColorAttachment", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
      {"DepthStencilAttachment",
       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
      {"DepthStencilReadOnly", VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
      {"ShaderReadOnly", VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
      {"TransferSrc", VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
      {"TransferDst", VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
      {"Preinitialized", VK_IMAGE_LAYOUT_PREINITIALIZED}};

  auto imageLayout = imageLayouts.find(p_ImageLayout);
  if (imageLayout != imageLayouts.end())
  {
    return imageLayout->second;
  }

  _INTR_ASSERT(false && "Image layout not supported/found");
  return VK_IMAGE_LAYOUT_GENERAL;
}

// <-

_INTR_INLINE static DepthStencilStates::Enum
mapDepthStencilState(const _INTR_STRING& p_DepthStencilState)
{
  static _INTR_HASH_MAP(Name, DepthStencilStates::Enum) depthStencilStates = {
      {"Default", DepthStencilStates::kDefault},
      {"DefaultNoDepthTestAndWrite",
       DepthStencilStates::kDefaultNoDepthTestAndWrite},
      {"DefaultNoWrite", DepthStencilStates::kDefaultNoWrite},
      {"DefaultNoDepthTest", DepthStencilStates::kDefaultNoDepthTest}};

  auto depthStencilState = depthStencilStates.find(p_DepthStencilState);
  if (depthStencilState != depthStencilStates.end())
  {
    return depthStencilState->second;
  }

  _INTR_ASSERT(false && "Depth stencil state not supported/found");
  return DepthStencilStates::kDefault;
}

// <-

_INTR_INLINE static BlendStates::Enum
mapBlendState(const _INTR_STRING& p_BlendSate)
{
  static _INTR_HASH_MAP(Name, BlendStates::Enum)
      blendStates = {{"Default", BlendStates::kDefault},
                     {"AlphaBlend", BlendStates::kAlphaBlend}};

  auto blendState = blendStates.find(p_BlendSate);
  if (blendState != blendStates.end())
  {
    return blendState->second;
  }

  _INTR_ASSERT(false && "Blend state not supported/found");
  return BlendStates::kDefault;
}
}
}
}
}
