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

// Precompiled header file
#include "stdafx_vulkan.h"
#include "stdafx.h"

#define LUM_AND_BRIGHT_THREADS 8u

#define ADD_THREADS_X 64
#define ADD_THREADS_Y 2

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
struct PerInstanceDataLumBright
{
  uint32_t dim[4];
  glm::vec4 bloomThreshold;
};

struct PerInstanceDataAvgLum
{
  uint32_t dim[4];
  glm::vec4 data;
};

struct PerInstanceDataAdd
{
  uint32_t dim[4];
  uint32_t mipLevel[4];
};

namespace
{
Resources::ImageRef _lumImageRef;
Resources::ImageRef _blurPingPongImageRef;
Resources::ImageRef _blurImageRef;
Resources::ImageRef _summedImageRef;
Resources::ImageRef _brightImageRef;

Resources::PipelineRef _brightLumPipelineRef;
Resources::PipelineRef _avgLumPipelineRef;
Resources::PipelineRef _blurXPipelineRef;
Resources::PipelineRef _blurYPipelineRef;
Resources::PipelineRef _addPipelineRef;

Resources::ComputeCallRef _lumComputeCallRef;
Resources::ComputeCallRef _avgLumComputeCallRef;
Resources::ComputeCallRefArray _blurXComputeCallRefs;
Resources::ComputeCallRefArray _blurYComputeCallRefs;
Resources::ComputeCallRefArray _addComputeCallRefs;

Resources::BufferRef _lumBuffer;

bool _initAvgLum = true;

_INTR_INLINE glm::uvec2 calcBloomBaseDim()
{
  return glm::uvec2(RenderSystem::_backbufferDimensions.x / 2u,
                    RenderSystem::_backbufferDimensions.y / 2u);
}

_INTR_INLINE void dispatchLum(VkCommandBuffer p_CommandBuffer)
{
  using namespace Resources;

  const glm::uvec2 bloomBaseDim = calcBloomBaseDim();

  PerInstanceDataLumBright lumData = {};
  {
    lumData.dim[0] = bloomBaseDim.x;
    lumData.dim[1] = bloomBaseDim.y;
    lumData.bloomThreshold.x = 1.0f;
  }

  ComputeCallManager::updateUniformMemory({_lumComputeCallRef}, &lumData,
                                          sizeof(PerInstanceDataLumBright));

  RenderSystem::dispatchComputeCall(_lumComputeCallRef, p_CommandBuffer);

  ImageManager::insertImageMemoryBarrier(
      _brightImageRef, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

// <-

_INTR_INLINE void dispatchAvgLum(VkCommandBuffer p_CommandBuffer)
{
  using namespace Resources;

  const glm::uvec2 bloomBaseDim = calcBloomBaseDim();

  PerInstanceDataAvgLum avgLumData = {};
  {
    avgLumData.dim[0] = Bloom::calculateThreadGroups(bloomBaseDim.x / 2u,
                                                     LUM_AND_BRIGHT_THREADS);
    avgLumData.dim[1] = Bloom::calculateThreadGroups(bloomBaseDim.y / 2u,
                                                     LUM_AND_BRIGHT_THREADS);

    if (_initAvgLum)
    {
      avgLumData.data.y = 1.0f;
      _initAvgLum = false;
    }
    else
    {
      avgLumData.data.y = 0.0f;
    }

    avgLumData.data.x = TaskManager::_lastDeltaT;
  }

  ComputeCallManager::updateUniformMemory({_avgLumComputeCallRef}, &avgLumData,
                                          sizeof(PerInstanceDataAvgLum));

  RenderSystem::dispatchComputeCall(_avgLumComputeCallRef, p_CommandBuffer);

  BufferManager::insertBufferMemoryBarrier(
      _lumBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

// <-

_INTR_INLINE void dispatchBlur(VkCommandBuffer p_CommandBuffer,
                               uint32_t p_MipLevel0)
{
  using namespace Resources;

  PerInstanceDataBlur blurData = {};
  {
    blurData.mipLevel[0] = p_MipLevel0;
  }

  ComputeCallManager::updateUniformMemory({_blurXComputeCallRefs[p_MipLevel0]},
                                          &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_blurXComputeCallRefs[p_MipLevel0],
                                    p_CommandBuffer);

  ImageManager::insertImageMemoryBarrierSubResource(
      _blurPingPongImageRef, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
      p_MipLevel0, 0u, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  ComputeCallManager::updateUniformMemory({_blurYComputeCallRefs[p_MipLevel0]},
                                          &blurData,
                                          sizeof(PerInstanceDataBlur));

  RenderSystem::dispatchComputeCall(_blurYComputeCallRefs[p_MipLevel0],
                                    p_CommandBuffer);

  ImageManager::insertImageMemoryBarrierSubResource(
      _blurImageRef, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
      p_MipLevel0, 0u, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

// <-

_INTR_INLINE void dispatchAdd(VkCommandBuffer p_CommandBuffer,
                              uint32_t p_MipLevel0, uint32_t p_MipLevel1)
{
  using namespace Resources;

  const glm::uvec2 bloomBaseDim = calcBloomBaseDim();

  PerInstanceDataAdd addData = {};
  {
    addData.mipLevel[0] = p_MipLevel0;
    addData.mipLevel[1] = p_MipLevel1;

    addData.dim[0] = (bloomBaseDim.x * 2u) >> (p_MipLevel0 + 1);
    addData.dim[1] = (bloomBaseDim.y * 2u) >> (p_MipLevel0 + 1);
  }

  ComputeCallManager::updateUniformMemory({_addComputeCallRefs[p_MipLevel0]},
                                          &addData, sizeof(PerInstanceDataAdd));

  RenderSystem::dispatchComputeCall(_addComputeCallRefs[p_MipLevel0],
                                    p_CommandBuffer);

  ImageManager::insertImageMemoryBarrierSubResource(
      _summedImageRef, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
      p_MipLevel0, 0u, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
}

// <-

void Bloom::init()
{
  using namespace Resources;

  PipelineRefArray pipelinesToCreate;
  BufferRefArray buffersToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;

  // Buffers
  {
    _lumBuffer = BufferManager::createBuffer(_N(BloomLumBuffer));
    {
      BufferManager::resetToDefault(_lumBuffer);
      BufferManager::addResourceFlags(
          _lumBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

      BufferManager::_descBufferType(_lumBuffer) = BufferType::kStorage;
      BufferManager::_descSizeInBytes(_lumBuffer) = sizeof(float);
    }
    buffersToCreate.push_back(_lumBuffer);
  }

  BufferManager::createResources(buffersToCreate);

  // Pipeline layouts
  PipelineLayoutRef pipelineLayoutLumBright;
  {
    pipelineLayoutLumBright =
        PipelineLayoutManager::createPipelineLayout(_N(BloomLumBright));
    PipelineLayoutManager::resetToDefault(pipelineLayoutLumBright);

    GpuProgramManager::reflectPipelineLayout(
        32u,
        {Resources::GpuProgramManager::getResourceByName(
            "post_bloom_bright_lum.comp")},
        pipelineLayoutLumBright);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayoutLumBright);

  PipelineLayoutRef pipelineLayoutAvgLum;
  {
    pipelineLayoutAvgLum =
        PipelineLayoutManager::createPipelineLayout(_N(BloomAvgLum));
    PipelineLayoutManager::resetToDefault(pipelineLayoutAvgLum);

    GpuProgramManager::reflectPipelineLayout(
        32u,
        {Resources::GpuProgramManager::getResourceByName(
            "post_bloom_avg_lum.comp")},
        pipelineLayoutAvgLum);
  }

  pipelineLayoutsToCreate.push_back(pipelineLayoutAvgLum);

  PipelineLayoutRef pipelineLayoutAdd;
  {
    pipelineLayoutAdd =
        PipelineLayoutManager::createPipelineLayout(_N(BloomAdd));
    PipelineLayoutManager::resetToDefault(pipelineLayoutAdd);

    GpuProgramManager::reflectPipelineLayout(
        32u,
        {Resources::GpuProgramManager::getResourceByName(
            "post_bloom_add.comp")},
        pipelineLayoutAdd);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayoutAdd);

  PipelineLayoutRef pipelineLayoutBlur;
  {
    pipelineLayoutBlur =
        PipelineLayoutManager::createPipelineLayout(_N(BloomBlur));
    PipelineLayoutManager::resetToDefault(pipelineLayoutBlur);

    GpuProgramManager::reflectPipelineLayout(
        128u,
        {Resources::GpuProgramManager::getResourceByName(
            "post_bloom_blur_x.comp")},
        pipelineLayoutBlur);
  }
  pipelineLayoutsToCreate.push_back(pipelineLayoutBlur);

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);

  // Pipelines
  {
    _brightLumPipelineRef = PipelineManager::createPipeline(_N(BloomBrightLum));
    PipelineManager::resetToDefault(_brightLumPipelineRef);

    PipelineManager::_descComputeProgram(_brightLumPipelineRef) =
        GpuProgramManager::getResourceByName("post_bloom_bright_lum.comp");
    PipelineManager::_descPipelineLayout(_brightLumPipelineRef) =
        pipelineLayoutLumBright;
  }
  pipelinesToCreate.push_back(_brightLumPipelineRef);

  {
    _avgLumPipelineRef = PipelineManager::createPipeline(_N(BloomAvgLum));
    PipelineManager::resetToDefault(_avgLumPipelineRef);

    PipelineManager::_descComputeProgram(_avgLumPipelineRef) =
        GpuProgramManager::getResourceByName("post_bloom_avg_lum.comp");
    PipelineManager::_descPipelineLayout(_avgLumPipelineRef) =
        pipelineLayoutAvgLum;
  }
  pipelinesToCreate.push_back(_avgLumPipelineRef);

  {
    _addPipelineRef = PipelineManager::createPipeline(_N(BloomAdd));
    PipelineManager::resetToDefault(_addPipelineRef);

    PipelineManager::_descComputeProgram(_addPipelineRef) =
        GpuProgramManager::getResourceByName("post_bloom_add.comp");
    PipelineManager::_descPipelineLayout(_addPipelineRef) = pipelineLayoutAdd;
  }
  pipelinesToCreate.push_back(_addPipelineRef);

  {
    _blurXPipelineRef = PipelineManager::createPipeline(_N(BloomBlurX));
    PipelineManager::resetToDefault(_blurXPipelineRef);

    PipelineManager::_descComputeProgram(_blurXPipelineRef) =
        GpuProgramManager::getResourceByName("post_bloom_blur_x.comp");
    PipelineManager::_descPipelineLayout(_blurXPipelineRef) =
        pipelineLayoutBlur;
  }
  pipelinesToCreate.push_back(_blurXPipelineRef);

  {
    _blurYPipelineRef = PipelineManager::createPipeline(_N(BloomBlurY));
    PipelineManager::resetToDefault(_blurYPipelineRef);

    PipelineManager::_descComputeProgram(_blurYPipelineRef) =
        GpuProgramManager::getResourceByName("post_bloom_blur_y.comp");
    PipelineManager::_descPipelineLayout(_blurYPipelineRef) =
        pipelineLayoutBlur;
  }
  pipelinesToCreate.push_back(_blurYPipelineRef);

  PipelineManager::createResources(pipelinesToCreate);
}

// <-

void Bloom::onReinitRendering()
{
  using namespace Resources;

  ImageRefArray imgsToDestroy;
  ImageRefArray imgsToCreate;
  ComputeCallRefArray computeCallsToDestroy;
  ComputeCallRefArray computeCallsToCreate;

  // Cleanup old resources
  {
    if (_lumImageRef.isValid())
      imgsToDestroy.push_back(_lumImageRef);
    if (_blurImageRef.isValid())
      imgsToDestroy.push_back(_blurImageRef);
    if (_blurPingPongImageRef.isValid())
      imgsToDestroy.push_back(_blurPingPongImageRef);
    if (_summedImageRef.isValid())
      imgsToDestroy.push_back(_summedImageRef);
    if (_brightImageRef.isValid())
      imgsToDestroy.push_back(_brightImageRef);

    ImageManager::destroyImagesAndResources(imgsToDestroy);
  }

  // Compute Calls
  if (_lumComputeCallRef.isValid())
  {
    computeCallsToDestroy.push_back(_lumComputeCallRef);
  }
  if (!_addComputeCallRefs.empty())
  {
    for (uint32_t i = 0u; i < _addComputeCallRefs.size(); ++i)
    {
      computeCallsToDestroy.push_back(_addComputeCallRefs[i]);
    }
    _addComputeCallRefs.clear();
  }
  if (_avgLumComputeCallRef.isValid())
  {
    computeCallsToDestroy.push_back(_avgLumComputeCallRef);
  }
  if (!_blurXComputeCallRefs.empty())
  {
    for (uint32_t i = 0u; i < _blurXComputeCallRefs.size(); ++i)
    {
      computeCallsToDestroy.push_back(_blurXComputeCallRefs[i]);
    }
    _blurXComputeCallRefs.clear();
  }
  if (!_blurYComputeCallRefs.empty())
  {
    for (uint32_t i = 0u; i < _blurYComputeCallRefs.size(); ++i)
    {
      computeCallsToDestroy.push_back(_blurYComputeCallRefs[i]);
    }
    _blurYComputeCallRefs.clear();
  }

  ComputeCallManager::destroyResources(computeCallsToDestroy);

  for (uint32_t i = 0u; i < (uint32_t)computeCallsToDestroy.size(); ++i)
  {
    ComputeCallManager::destroyComputeCall(computeCallsToDestroy[i]);
  }

  // Images
  glm::uvec3 dim = glm::uvec3(calcBloomBaseDim(), 1u);

  _lumImageRef = ImageManager::createImage(_N(BloomLum));
  {
    ImageManager::resetToDefault(_lumImageRef);
    ImageManager::addResourceFlags(
        _lumImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_lumImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_lumImageRef) = glm::uvec3(
        calculateThreadGroups(dim.x / 2u, LUM_AND_BRIGHT_THREADS),
        calculateThreadGroups(dim.y / 2u, LUM_AND_BRIGHT_THREADS), 1u);
    ImageManager::_descImageFormat(_lumImageRef) = Format::kR32SFloat;
    ImageManager::_descImageType(_lumImageRef) = ImageType::kTexture;

    ImageManager::_descImageFlags(_lumImageRef) &=
        ~ImageFlags::kUsageAttachment;
    ImageManager::_descImageFlags(_lumImageRef) |= ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_lumImageRef);

  _blurImageRef = ImageManager::createImage(_N(BloomBlur));
  {
    ImageManager::resetToDefault(_blurImageRef);
    ImageManager::addResourceFlags(
        _blurImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_blurImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_blurImageRef) = dim;
    ImageManager::_descImageFormat(_blurImageRef) = Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_blurImageRef) = ImageType::kTexture;
    ImageManager::_descMipLevelCount(_blurImageRef) = 4u;

    ImageManager::_descImageFlags(_blurImageRef) &=
        ~ImageFlags::kUsageAttachment;
    ImageManager::_descImageFlags(_blurImageRef) |= ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_blurImageRef);

  _blurPingPongImageRef = ImageManager::createImage(_N(BloomBlurPingPong));
  {
    ImageManager::resetToDefault(_blurPingPongImageRef);
    ImageManager::addResourceFlags(
        _blurPingPongImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_blurPingPongImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_blurPingPongImageRef) = dim;
    ImageManager::_descImageFormat(_blurPingPongImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_blurPingPongImageRef) = ImageType::kTexture;
    ImageManager::_descMipLevelCount(_blurPingPongImageRef) = 4u;

    ImageManager::_descImageFlags(_blurPingPongImageRef) &=
        ~ImageFlags::kUsageAttachment;
    ImageManager::_descImageFlags(_blurPingPongImageRef) |=
        ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_blurPingPongImageRef);

  _summedImageRef = ImageManager::createImage(_N(BloomSummed));
  {
    ImageManager::resetToDefault(_summedImageRef);
    ImageManager::addResourceFlags(
        _summedImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_summedImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_summedImageRef) = dim;
    ImageManager::_descImageFormat(_summedImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_summedImageRef) = ImageType::kTexture;
    ImageManager::_descMipLevelCount(_summedImageRef) = 4u;

    ImageManager::_descImageFlags(_summedImageRef) &=
        ~ImageFlags::kUsageAttachment;
    ImageManager::_descImageFlags(_summedImageRef) |= ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_summedImageRef);

  _brightImageRef = ImageManager::createImage(_N(BloomBright));
  {
    ImageManager::resetToDefault(_brightImageRef);
    ImageManager::addResourceFlags(
        _brightImageRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    ImageManager::_descMemoryPoolType(_brightImageRef) =
        MemoryPoolType::kResolutionDependentImages;

    ImageManager::_descDimensions(_brightImageRef) = dim;
    ImageManager::_descImageFormat(_brightImageRef) =
        Format::kR16G16B16A16Float;
    ImageManager::_descImageType(_brightImageRef) = ImageType::kTexture;
    ImageManager::_descMipLevelCount(_brightImageRef) = 4u;

    ImageManager::_descImageFlags(_brightImageRef) &=
        ~ImageFlags::kUsageAttachment;
    ImageManager::_descImageFlags(_brightImageRef) |= ImageFlags::kUsageStorage;
  }
  imgsToCreate.push_back(_brightImageRef);

  ImageManager::createResources(imgsToCreate);

  // Compute calls
  _lumComputeCallRef =
      ComputeCallManager::createComputeCall(_N(BloomBrightLum));
  {
    Vulkan::Resources::ComputeCallManager::resetToDefault(_lumComputeCallRef);
    Vulkan::Resources::ComputeCallManager::addResourceFlags(
        _lumComputeCallRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    Vulkan::Resources::ComputeCallManager::_descPipeline(_lumComputeCallRef) =
        _brightLumPipelineRef;
    Vulkan::Resources::ComputeCallManager::_descDimensions(_lumComputeCallRef) =
        glm::vec3(calculateThreadGroups(dim.x / 2u, LUM_AND_BRIGHT_THREADS),
                  calculateThreadGroups(dim.y / 2u, LUM_AND_BRIGHT_THREADS), 1);

    ComputeCallManager::bindBuffer(
        _lumComputeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataLumBright));
    ComputeCallManager::bindImage(_lumComputeCallRef, _N(output0Tex),
                                  GpuProgramType::kCompute, _brightImageRef,
                                  Samplers::kLinearRepeat,
                                  BindingFlags::kAdressSubResource, 0u, 0u);
    ComputeCallManager::bindImage(_lumComputeCallRef, _N(output1Tex),
                                  GpuProgramType::kCompute, _brightImageRef,
                                  Samplers::kLinearRepeat,
                                  BindingFlags::kAdressSubResource, 0u, 1u);
    ComputeCallManager::bindImage(_lumComputeCallRef, _N(output2Tex),
                                  GpuProgramType::kCompute, _brightImageRef,
                                  Samplers::kLinearRepeat,
                                  BindingFlags::kAdressSubResource, 0u, 2u);
    ComputeCallManager::bindImage(_lumComputeCallRef, _N(output3Tex),
                                  GpuProgramType::kCompute, _brightImageRef,
                                  Samplers::kLinearRepeat,
                                  BindingFlags::kAdressSubResource, 0u, 3u);
    ComputeCallManager::bindImage(_lumComputeCallRef, _N(outputLumTex),
                                  GpuProgramType::kCompute, _lumImageRef,
                                  Samplers::kLinearRepeat);
    ComputeCallManager::bindImage(
        _lumComputeCallRef, _N(input0Tex), GpuProgramType::kCompute,
        ImageManager::getResourceByName(_N(Scene)), Samplers::kLinearRepeat);

    computeCallsToCreate.push_back(_lumComputeCallRef);
  }

  _avgLumComputeCallRef =
      ComputeCallManager::createComputeCall(_N(BloomAvgLum));
  {
    Vulkan::Resources::ComputeCallManager::resetToDefault(
        _avgLumComputeCallRef);
    Vulkan::Resources::ComputeCallManager::addResourceFlags(
        _avgLumComputeCallRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);
    Vulkan::Resources::ComputeCallManager::_descPipeline(
        _avgLumComputeCallRef) = _avgLumPipelineRef;

    ComputeCallManager::bindBuffer(
        _avgLumComputeCallRef, _N(PerInstance), GpuProgramType::kCompute,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceCompute,
        sizeof(PerInstanceDataAvgLum));
    ComputeCallManager::bindImage(_avgLumComputeCallRef, _N(inOutLumTex),
                                  GpuProgramType::kCompute, _lumImageRef,
                                  Samplers::kInvalidSampler);
    ComputeCallManager::bindBuffer(_avgLumComputeCallRef, _N(AvgLum),
                                   GpuProgramType::kCompute, _lumBuffer,
                                   UboType::kInvalidUbo, sizeof(float));

    computeCallsToCreate.push_back(_avgLumComputeCallRef);
  }

  for (uint32_t i = 0; i < 4u; ++i)
  {
    ComputeCallRef computeCallBlurXRef =
        ComputeCallManager::createComputeCall(_N(BloomBlurX));
    {
      Vulkan::Resources::ComputeCallManager::resetToDefault(
          computeCallBlurXRef);
      Vulkan::Resources::ComputeCallManager::addResourceFlags(
          computeCallBlurXRef,
          Dod::Resources::ResourceFlags::kResourceVolatile);
      Vulkan::Resources::ComputeCallManager::_descPipeline(
          computeCallBlurXRef) = _blurXPipelineRef;

      Vulkan::Resources::ComputeCallManager::_descDimensions(
          computeCallBlurXRef) =
          glm::vec3(
              calculateThreadGroups((dim.x * 2u) >> (i + 1), BLUR_X_THREADS_X),
              calculateThreadGroups((dim.y * 2u) >> (i + 1), BLUR_X_THREADS_Y),
              1);

      ComputeCallManager::bindBuffer(
          computeCallBlurXRef, _N(PerInstance), GpuProgramType::kCompute,
          UniformManager::_perInstanceUniformBuffer,
          UboType::kPerInstanceCompute, sizeof(PerInstanceDataBlur));
      ComputeCallManager::bindImage(
          computeCallBlurXRef, _N(outTex), GpuProgramType::kCompute,
          _blurPingPongImageRef, Samplers::kInvalidSampler,
          BindingFlags::kAdressSubResource, 0u, i);
      ComputeCallManager::bindImage(computeCallBlurXRef, _N(inTex),
                                    GpuProgramType::kCompute, _brightImageRef,
                                    Samplers::kLinearClamp);

      computeCallsToCreate.push_back(computeCallBlurXRef);
      _blurXComputeCallRefs.push_back(computeCallBlurXRef);

      ComputeCallRef computeCallBlurYRef =
          ComputeCallManager::createComputeCall(_N(BloomBlurY));
      {
        Vulkan::Resources::ComputeCallManager::resetToDefault(
            computeCallBlurYRef);
        Vulkan::Resources::ComputeCallManager::addResourceFlags(
            computeCallBlurYRef,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        Vulkan::Resources::ComputeCallManager::_descPipeline(
            computeCallBlurYRef) = _blurYPipelineRef;

        Vulkan::Resources::ComputeCallManager::_descDimensions(
            computeCallBlurYRef) =
            glm::vec3(calculateThreadGroups((dim.x * 2u) >> (i + 1),
                                            BLUR_Y_THREADS_X),
                      calculateThreadGroups((dim.y * 2u) >> (i + 1),
                                            BLUR_Y_THREADS_Y),
                      1);

        ComputeCallManager::bindBuffer(
            computeCallBlurYRef, _N(PerInstance), GpuProgramType::kCompute,
            UniformManager::_perInstanceUniformBuffer,
            UboType::kPerInstanceCompute, sizeof(PerInstanceDataBlur));
        ComputeCallManager::bindImage(computeCallBlurYRef, _N(outTex),
                                      GpuProgramType::kCompute, _blurImageRef,
                                      Samplers::kInvalidSampler,
                                      BindingFlags::kAdressSubResource, 0u, i);
        ComputeCallManager::bindImage(
            computeCallBlurYRef, _N(inTex), GpuProgramType::kCompute,
            _blurPingPongImageRef, Samplers::kLinearClamp);

        computeCallsToCreate.push_back(computeCallBlurYRef);
        _blurYComputeCallRefs.push_back(computeCallBlurYRef);
      }

      for (uint32_t i = 0u; i < 3u; ++i)
      {
        ComputeCallRef computeCallAddRef =
            ComputeCallManager::createComputeCall(_N(BloomAdd));
        {
          Vulkan::Resources::ComputeCallManager::resetToDefault(
              computeCallAddRef);
          Vulkan::Resources::ComputeCallManager::addResourceFlags(
              computeCallAddRef,
              Dod::Resources::ResourceFlags::kResourceVolatile);
          Vulkan::Resources::ComputeCallManager::_descPipeline(
              computeCallAddRef) = _addPipelineRef;

          Vulkan::Resources::ComputeCallManager::_descDimensions(
              computeCallAddRef) =
              glm::uvec3(
                  calculateThreadGroups((dim.x * 2u) >> (i + 1), ADD_THREADS_X),
                  calculateThreadGroups((dim.y * 2u) >> (i + 1), ADD_THREADS_Y),
                  1u);

          ComputeCallManager::bindBuffer(
              computeCallAddRef, _N(PerInstance), GpuProgramType::kCompute,
              UniformManager::_perInstanceUniformBuffer,
              UboType::kPerInstanceCompute, sizeof(PerInstanceDataAdd));
          ComputeCallManager::bindImage(
              computeCallAddRef, _N(input0Tex), GpuProgramType::kCompute,
              _brightImageRef, Samplers::kLinearClamp);
          ComputeCallManager::bindImage(computeCallAddRef, _N(input1Tex),
                                        GpuProgramType::kCompute, _blurImageRef,
                                        Samplers::kLinearClamp);
          ComputeCallManager::bindImage(
              computeCallAddRef, _N(output0Tex), GpuProgramType::kCompute,
              _summedImageRef, Samplers::kInvalidSampler,
              BindingFlags::kAdressSubResource, 0u, i);

          computeCallsToCreate.push_back(computeCallAddRef);
          _addComputeCallRefs.push_back(computeCallAddRef);
        }
      }
    }
  }

  ComputeCallManager::createResources(computeCallsToCreate);
}

// <-

void Bloom::destroy() {}

// <-

void Bloom::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Bloom");
  _INTR_PROFILE_GPU("Render Bloom");

  VkCommandBuffer primaryCmdBuffer = RenderSystem::getPrimaryCommandBuffer();

  ImageManager::insertImageMemoryBarrier(
      _brightImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  ImageManager::insertImageMemoryBarrier(
      _lumImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  ImageManager::insertImageMemoryBarrier(
      _summedImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  ImageManager::insertImageMemoryBarrier(
      _blurImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  ImageManager::insertImageMemoryBarrier(
      _blurPingPongImageRef, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  dispatchLum(primaryCmdBuffer);

  dispatchAvgLum(primaryCmdBuffer);

  dispatchBlur(primaryCmdBuffer, 3u);

  dispatchAdd(primaryCmdBuffer, 2u, 3u);
  dispatchBlur(primaryCmdBuffer, 2u);

  dispatchAdd(primaryCmdBuffer, 1u, 2u);
  dispatchBlur(primaryCmdBuffer, 1u);

  dispatchAdd(primaryCmdBuffer, 0u, 1u);
  dispatchBlur(primaryCmdBuffer, 0u);

  ImageManager::insertImageMemoryBarrier(
      _summedImageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  ImageManager::insertImageMemoryBarrier(
      _lumImageRef, VK_IMAGE_LAYOUT_GENERAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
}
}
}
}
