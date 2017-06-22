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

using namespace RVResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
void GenericBlur::init(const rapidjson::Value& p_RenderPassDesc)
{
  using namespace Resources;

  Base::init(p_RenderPassDesc);

  ComputeCallRefArray computeCallsToCreate;

  const glm::uvec3 dim =
      ImageManager::_descDimensions(ImageManager::getResourceByName(
          p_RenderPassDesc["sourceImage"].GetString()));

  // Compute calls
  _computeCallX = ComputeCallManager::createComputeCall(_name);
  {
    ComputeCallManager::resetToDefault(_computeCallX);
    ComputeCallManager::addResourceFlags(
        _computeCallX, Dod::Resources::ResourceFlags::kResourceVolatile);
    ComputeCallManager::_descPipeline(_computeCallX) =
        PipelineManager::getResourceByName(_N(BloomBlurX));

    ComputeCallManager::_descDimensions(_computeCallX) =
        glm::uvec3(Bloom::calculateThreadGroups(
                       dim.x, BLUR_THREADS - 2u * BLUR_HALF_BLUR_WIDTH),
                   dim.y, 1u);

    ComputeCallManager::bindBuffer(
        _computeCallX, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataBlur));
    ComputeCallManager::bindImage(
        _computeCallX, _N(outTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(
            p_RenderPassDesc["pingPongImage"].GetString()),
        Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(
        _computeCallX, _N(inTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(
            p_RenderPassDesc["sourceImage"].GetString()),
        Samplers::kLinearClamp);

    computeCallsToCreate.push_back(_computeCallX);
  }

  _computeCallY = ComputeCallManager::createComputeCall(_name);
  {
    ComputeCallManager::resetToDefault(_computeCallY);
    ComputeCallManager::addResourceFlags(
        _computeCallY, Dod::Resources::ResourceFlags::kResourceVolatile);
    ComputeCallManager::_descPipeline(_computeCallY) =
        PipelineManager::getResourceByName(_N(BloomBlurY));

    ComputeCallManager::_descDimensions(_computeCallY) =
        glm::uvec3(Bloom::calculateThreadGroups(
                       dim.y, BLUR_THREADS - 2u * BLUR_HALF_BLUR_WIDTH),
                   dim.x, 1u);

    ComputeCallManager::bindBuffer(
        _computeCallY, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataBlur));
    ComputeCallManager::bindImage(
        _computeCallY, _N(outTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(
            p_RenderPassDesc["targetImage"].GetString()),
        Samplers::kInvalidSampler);
    ComputeCallManager::bindImage(
        _computeCallY, _N(inTex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(
            p_RenderPassDesc["pingPongImage"].GetString()),
        Samplers::kLinearClamp);

    computeCallsToCreate.push_back(_computeCallY);
  }

  ComputeCallManager::createResources(computeCallsToCreate);
}

// <-

void GenericBlur::destroy()
{
  Base::destroy();

  ComputeCallRefArray computeCallsToDestroy;

  if (_computeCallX.isValid())
    computeCallsToDestroy.push_back(_computeCallX);
  _computeCallX = ComputeCallRef();
  if (_computeCallY.isValid())
    computeCallsToDestroy.push_back(_computeCallX);
  _computeCallY = ComputeCallRef();

  ComputeCallManager::destroyComputeCallsAndResources(computeCallsToDestroy);
}

// <-

void GenericBlur::dispatchBlur(VkCommandBuffer p_CommandBuffer)
{
  PerInstanceDataBlur blurData = {};
  {
    blurData.mipLevel[0] = 0u;
  }

  ComputeCallManager::updateUniformMemory({_computeCallX}, &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_computeCallX, p_CommandBuffer);

  ComputeCallManager::updateUniformMemory({_computeCallY}, &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_computeCallY, p_CommandBuffer);
}

// <-

void GenericBlur::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU_DEFINE(GenericBlurCPU, "Render Pass", _name.c_str());
  _INTR_PROFILE_CPU_CUSTOM(GenericBlurCPU);
  _INTR_PROFILE_GPU_DEFINE(GenericBlurGPU, _name.c_str());
  _INTR_PROFILE_GPU_CUSTOM(GenericBlurGPU, _name.c_str());

  dispatchBlur(RenderSystem::getPrimaryCommandBuffer());
}
}
}
}
}
