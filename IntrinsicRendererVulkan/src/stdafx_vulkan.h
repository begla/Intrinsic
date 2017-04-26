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

// ->::Renderer related includes
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
#include "IntrinsicRendererVulkanGpuMemoryManager.h"
#include "IntrinsicRendererVulkanRenderSystem.h"
#include "IntrinsicRendererVulkanRenderProcessUniformManager.h"
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
#include "IntrinsicRendererVulkanRenderPassBase.h"
#include "IntrinsicRendererVulkanResourcesDrawCall.h"
#include "IntrinsicRendererVulkanResourcesComputeCall.h"
#include "IntrinsicRendererVulkanRenderPassGBuffer.h"
#include "IntrinsicRendererVulkanRenderPassGBufferTransparents.h"
#include "IntrinsicRendererVulkanRenderPassShadow.h"
#include "IntrinsicRendererVulkanRenderPassDebug.h"
#include "IntrinsicRendererVulkanRenderPassGenericFullscreen.h"
#include "IntrinsicRendererVulkanRenderPassVolumetricLighting.h"
#include "IntrinsicRendererVulkanRenderPassLighting.h"
#include "IntrinsicRendererVulkanRenderPassBloom.h"
#include "IntrinsicRendererVulkanRenderPassSky.h"
#include "IntrinsicRendererVulkanRenderPassPerPixelPicking.h"
#include "IntrinsicRendererVulkanRenderPassFoliage.h"
#include "IntrinsicRendererVulkanRenderPassLensFlare.h"
#include "IntrinsicRendererVulkanDrawCallDispatcher.h"
