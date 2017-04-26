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
namespace MemoryPoolType
{
enum Enum
{
  kStaticImages,
  kStaticBuffers,
  kStaticStagingBuffers,

  kResolutionDependentImages,
  kResolutionDependentBuffers,
  kResolutionDependentStagingBuffers,

  kVolatileStagingBuffers,

  kCount,

  kRangeStartStatic = kStaticImages,
  kRangeEndStatic = kStaticStagingBuffers,
  kRangeStartResolutionDependent = kResolutionDependentImages,
  kRangeEndResolutionDependent = kResolutionDependentStagingBuffers,
  kRangeStartVolatile = kVolatileStagingBuffers,
  kRangeEndVolatile = kVolatileStagingBuffers
};
}

struct MemoryAllocationInfo
{
  VkDeviceMemory vkDeviceMemory;
  uint32_t sizeInBytes;
  uint32_t alignmentInBytes;
  uint32_t offsetInBytes;
  uint8_t memoryPoolType;
};

namespace RenderSize
{
enum Enum
{
  kCustom,

  kFull,
  kHalf,
  kQuarter
};
}

struct ResourceReleaseEntry
{
  Intrinsic::Core::Name typeName;

  void* userData0;
  void* userData1;

  uint8_t age;
};

namespace ImageType
{
enum Enum
{
  kExternal,
  kTextureFromFile,
  kTexture,
};
}

namespace ImageFlags
{
enum Flags
{
  kExternalImage = 0x01u,
  kExternalView = 0x02u,
  kExternalDeviceMemory = 0x04u,

  kUsageAttachment = 0x08u,
  kUsageSampled = 0x10u,
  kUsageStorage = 0x20u
};
}

namespace GpuProgramType
{
enum Enum
{
  kVertex,
  kFragment,
  kGeometry,
  kCompute
};
}

namespace MemoryLocation
{
enum Enum
{
  kDeviceLocal,
  kHostVisible
};
}

namespace BufferType
{
enum Enum
{
  kVertex,
  kIndex16,
  kIndex32,
  kUniform,
  kStorage
};
}

namespace BindingType
{
enum Enum
{
  kUniformBuffer,
  kUniformBufferDynamic,
  kStorageBuffer,

  kImageAndSamplerCombined,
  kSampledImage,
  kStorageImage,

  kSampler,

  kCount,

  kRangeStartBuffer = kUniformBuffer,
  kRangeEndBuffer = kStorageBuffer,
  kRangeStartImage = kImageAndSamplerCombined,
  kRangeEndImage = kStorageImage
};
}

namespace Format
{
enum Enum
{
  // Floating point
  kR32G32B32SFloat,
  kR32G32B32A32SFloat,
  kR32G32SFloat,
  kR16G16B16A16Float,
  kR16G16B16Float,
  kR16G16Float,

  // Simple
  kB8G8R8A8UNorm,

  // BC formats
  kBC1RGBUNorm,
  kBC1RGBSrgb,
  kBC2UNorm,
  kBC2Srgb,
  kBC5UNorm,
  kBC5SNorm,

  // Depth/Stencil
  kD24UnormS8UInt,

  // TODO: Clean this up
  kB8G8R8A8Srgb,
  kBC6UFloat,
  kBC3UNorm,
  kBC3Srgb,
  kR32SFloat,
  kR32UInt,
  kD16UnormS8UInt,
  kD32SFloatS8UInt,
  kD32SFloat,
  kD16UNorm,

  kCount
};
}

namespace UboType
{
enum Enum
{
  kPerInstanceFragment,
  kPerInstanceVertex,
  kPerInstanceCompute,

  kPerMaterialFragment,
  kPerMaterialVertex,

  kInvalidUbo,

  kCount,

  kRangeStartPerInstance = kPerInstanceFragment,
  kRangeEndPerInstance = kPerInstanceCompute,
  kRangeStartPerMaterial = kPerMaterialFragment,
  kRangeEndPerMaterial = kPerMaterialVertex
};
}

struct VertexBinding
{
  uint32_t stride;
  uint8_t binding;
};

struct VertexAttribute
{
  uint8_t format;
  uint8_t location;
  uint8_t binding;
  uint32_t offset;
};

namespace AttachmentFlags
{
enum Flags
{
  kClearOnLoad = 0x01u,
  kClearStencilOnLoad = 0x02u
};
}

struct AttachmentDescription
{
  uint8_t format;
  uint8_t flags;
};

struct AttachmentInfo
{
  AttachmentInfo(Core::Dod::Ref p_ImageRef, uint32_t p_ArrayLayerIdx = 0u)
      : imageRef(p_ImageRef), arrayLayerIdx(p_ArrayLayerIdx)
  {
  }

  Core::Dod::Ref imageRef;
  uint32_t arrayLayerIdx;
};

struct BindingDescription
{
  uint8_t binding;
  uint8_t bindingType;
  uint8_t shaderStage;

  uint32_t poolCount;

  Name name;
};

struct BindingFlags
{
  enum Flags
  {
    kAdressSubResource = 0x01
  };
};

struct BufferData
{
  uint32_t offsetInBytes;
  uint32_t rangeInBytes;

  uint8_t uboType;
};

struct ImageData
{
  uint8_t mipLevelIdx;
  uint8_t arrayLayerIdx;
  uint8_t samplerIdx;
};

struct BindingInfo
{
  uint8_t binding;
  uint8_t bindingType;
  uint8_t bindingFlags;

  Core::Dod::Ref resource;

  union
  {
    BufferData bufferData;
    ImageData imageData;
  };
};
}
}
}
