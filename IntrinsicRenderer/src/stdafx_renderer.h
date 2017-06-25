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

// ->::Renderer related includes
#include "IntrinsicRendererPrerequisites.h"

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
#include "IntrinsicRendererEnumsStructs.h"
#include "IntrinsicRendererRenderStates.h"
#include "IntrinsicRendererSamplers.h"
#include "IntrinsicRendererGpuMemoryManager.h"
#include "IntrinsicRendererRenderSystem.h"
#include "IntrinsicRendererRenderProcessUniformManager.h"
#include "IntrinsicRendererRenderProcess.h"
#include "IntrinsicRendererDebugging.h"
#include "IntrinsicRendererResourcesGpuProgram.h"
#include "IntrinsicRendererResourcesRenderPass.h"
#include "IntrinsicRendererResourcesFramebuffer.h"
#include "IntrinsicRendererResourcesVertexLayout.h"
#include "IntrinsicRendererHelper.h"
#include "IntrinsicRendererResourcesImage.h"
#include "IntrinsicRendererResourcesBuffer.h"
#include "IntrinsicRendererResourcesPipelineLayout.h"
#include "IntrinsicRendererResourcesPipeline.h"
#include "IntrinsicRendererResourcesMaterial.h"
#include "IntrinsicRendererUniformManager.h"
#include "IntrinsicRendererMaterialBuffer.h"
#include "IntrinsicRendererRenderPassBase.h"
#include "IntrinsicRendererResourcesDrawCall.h"
#include "IntrinsicRendererResourcesComputeCall.h"
#include "IntrinsicRendererRenderPassShadow.h"
#include "IntrinsicRendererRenderPassDebug.h"
#include "IntrinsicRendererRenderPassGenericFullscreen.h"
#include "IntrinsicRendererRenderPassGenericMesh.h"
#include "IntrinsicRendererRenderPassGenericBlur.h"
#include "IntrinsicRendererRenderPassVolumetricLighting.h"
#include "IntrinsicRendererRenderPassClustering.h"
#include "IntrinsicRendererRenderPassBloom.h"
#include "IntrinsicRendererRenderPassPerPixelPicking.h"
#include "IntrinsicRendererDrawCallDispatcher.h"
