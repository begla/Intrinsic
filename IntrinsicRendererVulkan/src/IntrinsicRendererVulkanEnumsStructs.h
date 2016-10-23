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

namespace MaterialPass
{
enum Enum
{
  kSurface,
  kShadow,

  kSky,

  kDebugGizmo,
  kDebugGrid,

  kFoliage,
  kShadowFoliage,

  kSurfaceWater,
  kSurfaceLayered,

  kPerPixelPicking,

  kCount,
  kNone
};
}

namespace MaterialPassFlags
{
enum Flags
{
  kSurface = (1u << MaterialPass::kSurface),
  kShadow = (1u << MaterialPass::kShadow),

  kSky = (1u << MaterialPass::kSky),

  kDebugGizmo = (1u << MaterialPass::kDebugGizmo),
  kDebugGrid = (1u << MaterialPass::kDebugGrid),

  kFoliage = (1u << MaterialPass::kFoliage),
  kShadowFoliage = (1u << MaterialPass::kShadowFoliage),

  kSurfaceWater = (1u << MaterialPass::kSurfaceWater),
  kSurfaceLayered = (1u << MaterialPass::kSurfaceLayered),

  kPerPixelPicking = (1u << MaterialPass::kPerPixelPicking),

  kShadowedSurface = kSurface | kShadow,
  kShadowedFoliage = kFoliage | kShadowFoliage,
  kShadowedSurfaceLayered = kSurfaceLayered | kShadow,

  kShadowedSurfaceWithPicking = kSurface | kShadow | kPerPixelPicking,
  kShadowedFoliageWithPicking = kFoliage | kShadowFoliage | kPerPixelPicking,
  kShadowedSurfaceLayeredWithPicking =
      kSurfaceLayered | kShadow | kPerPixelPicking
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

namespace MemoryUsage
{
enum Enum
{
  kOptimal,
  kHostVisibleAndCoherent
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
  kR32UInt
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
