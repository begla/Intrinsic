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

// Precompiled header files
#include "stdafx_vulkan.h"
#include "stdafx.h"

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
void PipelineLayoutManager::createResources(
    const PipelineLayoutRefArray& p_PipelineLayouts)
{
  for (uint32_t pplLayoutIdx = 0u; pplLayoutIdx < p_PipelineLayouts.size();
       ++pplLayoutIdx)
  {
    PipelineLayoutRef ref = p_PipelineLayouts[pplLayoutIdx];

    VkDescriptorSetLayout& descriptorSetLayout = _vkDescriptorSetLayout(ref);
    VkPipelineLayout& pipelineLayout = _vkPipelineLayout(ref);
    VkDescriptorPool& pool = _vkDescriptorPool(ref);
    _INTR_ARRAY(BindingDescription)& bindingInfos = _descBindingDescs(ref);

    _INTR_ARRAY(VkDescriptorSetLayoutBinding) layoutBindings;
    layoutBindings.resize(bindingInfos.size());

    for (uint32_t biIdx = 0u; biIdx < bindingInfos.size(); ++biIdx)
    {
      BindingDescription& info = bindingInfos[biIdx];

      layoutBindings[biIdx].binding = info.binding;
      layoutBindings[biIdx].descriptorType =
          Helper::mapBindingTypeToVkDescriptorType(
              (BindingType::Enum)info.bindingType);
      layoutBindings[biIdx].descriptorCount = 1u;
      layoutBindings[biIdx].stageFlags =
          Helper::mapGpuProgramTypeToVkShaderStage(
              (GpuProgramType::Enum)info.shaderStage);
      layoutBindings[biIdx].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descLayout = {};
    {
      descLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      descLayout.pNext = nullptr;
      descLayout.bindingCount = (uint32_t)layoutBindings.size();
      descLayout.pBindings = layoutBindings.data();

      VkResult result = vkCreateDescriptorSetLayout(
          RenderSystem::_vkDevice, &descLayout, nullptr, &descriptorSetLayout);
      _INTR_VK_CHECK_RESULT(result);
    }

    {
      VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
      pPipelineLayoutCreateInfo.sType =
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pPipelineLayoutCreateInfo.pNext = nullptr;
      pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
      pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
      pPipelineLayoutCreateInfo.setLayoutCount = 1u;
      pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

      VkResult result = vkCreatePipelineLayout(RenderSystem::_vkDevice,
                                               &pPipelineLayoutCreateInfo,
                                               nullptr, &pipelineLayout);
      _INTR_VK_CHECK_RESULT(result);
    }

    if (!bindingInfos.empty())
    {
      _INTR_ARRAY(VkDescriptorPoolSize) poolSizes;
      poolSizes.resize(bindingInfos.size());

      uint32_t maxPoolCount = 0u;
      for (uint32_t i = 0u; i < bindingInfos.size(); ++i)
      {
        BindingDescription& info = bindingInfos[i];

        poolSizes[i].type = Helper::mapBindingTypeToVkDescriptorType(
            (BindingType::Enum)info.bindingType);
        poolSizes[i].descriptorCount = info.poolCount;
        maxPoolCount = std::max(maxPoolCount, info.poolCount);
      }

      VkDescriptorPoolCreateInfo descriptorPool = {};
      descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      descriptorPool.pNext = nullptr;
      descriptorPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
      descriptorPool.maxSets = maxPoolCount;
      descriptorPool.poolSizeCount = (uint32_t)poolSizes.size();
      descriptorPool.pPoolSizes = poolSizes.data();

      VkResult result = vkCreateDescriptorPool(RenderSystem::_vkDevice,
                                               &descriptorPool, nullptr, &pool);
      _INTR_VK_CHECK_RESULT(result);
    }
  }
}

// <-

void PipelineLayoutManager::destroyResources(
    const PipelineLayoutRefArray& p_PipelineLayouts)
{
  for (uint32_t i = 0u; i < p_PipelineLayouts.size(); ++i)
  {
    PipelineLayoutRef ref = p_PipelineLayouts[i];

    VkPipelineLayout& pipelineLayout = _vkPipelineLayout(ref);
    if (pipelineLayout != VK_NULL_HANDLE)
    {
      vkDestroyPipelineLayout(RenderSystem::_vkDevice, pipelineLayout, nullptr);
      pipelineLayout = VK_NULL_HANDLE;
    }

    VkDescriptorSetLayout& descSetLayout = _vkDescriptorSetLayout(ref);
    if (descSetLayout != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorSetLayout(RenderSystem::_vkDevice, descSetLayout,
                                   nullptr);
      descSetLayout = VK_NULL_HANDLE;
    }

    VkDescriptorPool& descPool = _vkDescriptorPool(ref);
    if (descPool != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorPool(RenderSystem::_vkDevice, descPool, nullptr);
      descPool = VK_NULL_HANDLE;

      // Invalidate descriptor sets allocated from this pool
      for (uint32_t dcIdx = 0u; dcIdx < DrawCallManager::_activeRefs.size();
           ++dcIdx)
      {
        DrawCallRef dcRef = DrawCallManager::_activeRefs[dcIdx];
        PipelineRef pRef = DrawCallManager::_descPipeline(dcRef);

        if (pRef.isValid())
        {
          PipelineLayoutRef plRef = PipelineManager::_descPipelineLayout(pRef);

          if (plRef == ref)
          {
            DrawCallManager::_vkDescriptorSet(dcRef) = nullptr;
          }
        }
      }
    }
  }
}

// <-

VkDescriptorSet PipelineLayoutManager::allocateAndWriteDescriptorSet(
    PipelineLayoutRef p_Ref, const _INTR_ARRAY(BindingInfo) & p_BindInfos)
{
  if (_vkDescriptorPool(p_Ref))
  {
    VkDescriptorSetAllocateInfo allocInfo = {};
    {
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.pNext = nullptr;
      allocInfo.descriptorPool = _vkDescriptorPool(p_Ref);
      allocInfo.descriptorSetCount = 1u;
      allocInfo.pSetLayouts = &_vkDescriptorSetLayout(p_Ref);
    }

    VkDescriptorSet descSet;
    VkResult result =
        vkAllocateDescriptorSets(RenderSystem::_vkDevice, &allocInfo, &descSet);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_ARRAY(VkWriteDescriptorSet) writes;
    _INTR_ARRAY(VkDescriptorImageInfo) imageInfos;
    _INTR_ARRAY(VkDescriptorBufferInfo) bufferInfos;

    writes.resize(p_BindInfos.size());
    imageInfos.resize(p_BindInfos.size());
    bufferInfos.resize(p_BindInfos.size());

    for (uint32_t i = 0u; i < p_BindInfos.size(); ++i)
    {
      const BindingInfo& info = p_BindInfos[i];

      writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writes[i].pNext = nullptr;
      writes[i].dstSet = descSet;
      writes[i].descriptorCount = 1u;
      writes[i].descriptorType = Helper::mapBindingTypeToVkDescriptorType(
          (BindingType::Enum)info.bindingType);

      if (info.bindingType >= BindingType::kRangeStartBuffer &&
          info.bindingType <= BindingType::kRangeEndBuffer)
      {
        VkDescriptorBufferInfo& bufferInfo = bufferInfos[i];
        bufferInfo.buffer = Resources::BufferManager::_vkBuffer(info.resource);
        bufferInfo.offset = 0u;
        bufferInfo.range = info.bufferData.rangeInBytes;

        writes[i].pBufferInfo = &bufferInfo;
      }
      else if (info.bindingType >= BindingType::kRangeStartImage &&
               info.bindingType <= BindingType::kRangeEndImage)
      {
        VkDescriptorImageInfo& imageInfo = imageInfos[i];

        if (info.bindingType == BindingType::kImageAndSamplerCombined ||
            info.bindingType == BindingType::kStorageImage ||
            info.bindingType == BindingType::kSampledImage)
        {
          if ((info.bindingFlags & BindingFlags::kAdressSubResource) == 0u)
          {
            imageInfo.imageView =
                Resources::ImageManager::_vkImageView(info.resource);
          }
          else
          {
            imageInfo.imageView =
                Resources::ImageManager::_vkSubResourceImageView(
                    info.resource, info.imageData.arrayLayerIdx,
                    info.imageData.mipLevelIdx);
          }

          imageInfo.imageLayout = info.bindingType != BindingType::kStorageImage
                                      ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                      : VK_IMAGE_LAYOUT_GENERAL;
        }

        if (info.bindingType == BindingType::kImageAndSamplerCombined ||
            info.bindingType == BindingType::kSampler)
        {
          imageInfo.sampler = Samplers::samplers[info.imageData.samplerIdx];
        }

        writes[i].pImageInfo = &imageInfo;
      }

      writes[i].dstArrayElement = 0u;
      writes[i].dstBinding = info.binding;
    }

    vkUpdateDescriptorSets(RenderSystem::_vkDevice, (uint32_t)writes.size(),
                           writes.data(), 0u, nullptr);
    return descSet;
  }

  return VK_NULL_HANDLE;
}
}
}
}
}
