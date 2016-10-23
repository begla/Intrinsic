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

// Intrinsic::Renderer related includes
#include "IntrinsicRendererVulkanPrerequisites.h"

// Vulkan related includes
#include "vulkan/vulkan.h"
#undef snprintf
#undef max
#undef min
#undef Bool
#undef Success

// GLSLang related includes
#include "glslang/SPIRV/GlslangToSpv.h"

// SPIRVCross related includes
#include "spirv_glsl.hpp"

// Core related includes
#include "stdafx.h"

// Renderer related includes
#include "IntrinsicRendererVulkanEnumsStructs.h"
#include "IntrinsicRendererVulkanRenderStates.h"
#include "IntrinsicRendererVulkanSamplers.h"
#include "IntrinsicRendererVulkanRenderSystem.h"
#include "IntrinsicRendererVulkanRenderProcess.h"
#include "IntrinsicRendererVulkanDebugging.h"
#include "IntrinsicRendererVulkanResourcesGpuProgram.h"
#include "IntrinsicRendererVulkanResourcesRenderPass.h"
#include "IntrinsicRendererVulkanResourcesFramebuffer.h"
#include "IntrinsicRendererVulkanResourcesVertexLayout.h"
#include "IntrinsicRendererVulkanHelper.h"
#include "IntrinsicRendererVulkanResourcesImage.h"
#include "IntrinsicRendererVulkanResourcesBuffer.h"
#include "IntrinsicRendererVulkanResourcesPipelineLayout.h"
#include "IntrinsicRendererVulkanResourcesPipeline.h"
#include "IntrinsicRendererVulkanResourcesMaterial.h"
#include "IntrinsicRendererVulkanUniformManager.h"
#include "IntrinsicRendererVulkanMaterialBuffer.h"
#include "IntrinsicRendererVulkanResourcesDrawCall.h"
#include "IntrinsicRendererVulkanResourcesComputeCall.h"
#include "IntrinsicRendererVulkanRenderPassGBuffer.h"
#include "IntrinsicRendererVulkanRenderPassGBufferTransparents.h"
#include "IntrinsicRendererVulkanRenderPassShadow.h"
#include "IntrinsicRendererVulkanRenderPassDebug.h"
#include "IntrinsicRendererVulkanRenderPassPreCombine.h"
#include "IntrinsicRendererVulkanRenderPassPostCombine.h"
#include "IntrinsicRendererVulkanRenderPassVolumetricLighting.h"
#include "IntrinsicRendererVulkanRenderPassLighting.h"
#include "IntrinsicRendererVulkanRenderPassBloom.h"
#include "IntrinsicRendererVulkanRenderPassSky.h"
#include "IntrinsicRendererVulkanRenderPassPerPixelPicking.h"
#include "IntrinsicRendererVulkanRenderPassFoliage.h"
#include "IntrinsicRendererVulkanRenderPassLensFlare.h"
#include "IntrinsicRendererVulkanDrawCallDispatcher.h"
