// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"
#include "IntrinsicRendererVulkanHelper.h"

namespace Intrinsic
{
	namespace Renderer
	{
		namespace Vulkan
		{
			namespace Helper
			{
				bool GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, Format::Enum& depthFormat)
				{
					// Since all depth formats may be optional, we need to find a suitable depth format to use
					// Start with the highest precision packed format
					std::vector<Format::Enum> depthFormats =
					{
						Format::Enum::kD32sFLOATs8UINT,
						Format::Enum::kD32sFLOAT,
						Format::Enum::kD24UnormS8UInt,
						Format::Enum::kD16UnormS8UInt,
						Format::Enum::kD16UNORM
					};

					for (auto& format : depthFormats)
					{
						VkFormatProperties formatProps;

						//Get correct format
						VkFormat convertedFormat = mapFormatToVkFormat(format);
						vkGetPhysicalDeviceFormatProperties(physicalDevice, convertedFormat, &formatProps);

						// Format must support depth stencil attachment for optimal tiling
						if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
						{
							depthFormat = format;
							return true;
						}
					}

					return false;
				}
			}
		}
	}
}