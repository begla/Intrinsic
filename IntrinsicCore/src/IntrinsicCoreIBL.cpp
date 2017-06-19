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
#include "stdafx.h"

#define PRE_FILTERING_BLOCK_SIZE 32u

namespace Intrinsic
{
namespace Core
{
namespace IBL
{
namespace
{
enki::TaskScheduler _scheduler;

struct PreFilterParallelTaskSet : enki::ITaskSet
{
  virtual ~PreFilterParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _preFilterGGX(*_input, *_output, _faceIdx, _mipIdx, _rangeX, _rangeY,
                  _sampleCounts, _minRoughness);
  };

  const gli::texture_cube* _input;
  gli::texture_cube* _output;

  glm::uvec2 _rangeX;
  glm::uvec2 _rangeY;
  uint32_t _mipIdx;
  const uint32_t* _sampleCounts;
  uint32_t _faceIdx;
  float _minRoughness;
};

_INTR_ARRAY(PreFilterParallelTaskSet) _preFilterParallelTaskSets;
}

void initCubemapProcessing()
{
  _scheduler.Initialize();
  _preFilterParallelTaskSets = _INTR_ARRAY(PreFilterParallelTaskSet)();
}

// <-

void preFilterGGX(const gli::texture_cube& p_Input, gli::texture_cube& p_Output,
                  const uint32_t* p_SampleCounts, float p_MinRoughness)
{
  _INTR_PROFILE_AUTO("Pre Filter GGX");

  uint32_t jobCount = 0u;
  uint32_t totalSampleCount = 0u;

  // Calculate the amount of jobs to dispatch
  {
    for (uint32_t mipIdx = 0u; mipIdx <= p_Output.max_level(); ++mipIdx)
    {
      totalSampleCount += p_Output.extent(mipIdx).x *
                          p_Output.extent(mipIdx).y * 6u *
                          p_SampleCounts[mipIdx];

      const glm::uvec2 blockCount = glm::max(
          glm::uvec2(p_Output.extent(mipIdx)) / PRE_FILTERING_BLOCK_SIZE, 1u);

      for (uint32_t faceIdx = 0u; faceIdx <= p_Output.max_face(); ++faceIdx)
      {
        for (uint32_t blockY = 0u; blockY < blockCount.y; ++blockY)
        {
          for (uint32_t blockX = 0u; blockX < blockCount.x; ++blockX)
          {
            ++jobCount;
          }
        }
      }
    }
    _preFilterParallelTaskSets.resize(jobCount);
  }

  _INTR_LOG_INFO("Total amount of samples: %u", totalSampleCount);

  uint32_t jobIdx = 0u;
  for (uint32_t mipIdx = 0u; mipIdx <= p_Output.max_level(); ++mipIdx)
  {
    const glm::uvec2 blockCount = glm::max(
        glm::uvec2(p_Output.extent(mipIdx)) / PRE_FILTERING_BLOCK_SIZE, 1u);

    for (uint32_t faceIdx = 0u; faceIdx <= p_Output.max_face(); ++faceIdx)
    {
      for (uint32_t blockY = 0u; blockY < blockCount.y; ++blockY)
      {
        for (uint32_t blockX = 0u; blockX < blockCount.x; ++blockX)
        {
          const glm::uvec2 blockStart =
              glm::uvec2(blockX * PRE_FILTERING_BLOCK_SIZE,
                         blockY * PRE_FILTERING_BLOCK_SIZE);
          const glm::uvec2 blockEnd =
              glm::min(blockStart + PRE_FILTERING_BLOCK_SIZE,
                       glm::uvec2(p_Output.extent(mipIdx)));

          PreFilterParallelTaskSet& taskSet =
              _preFilterParallelTaskSets[jobIdx++];
          {
            taskSet._input = &p_Input;
            taskSet._output = &p_Output;
            taskSet._rangeX = glm::uvec2(blockStart.x, blockEnd.x);
            taskSet._rangeY = glm::uvec2(blockStart.y, blockEnd.y);
            taskSet._mipIdx = mipIdx;
            taskSet._faceIdx = faceIdx;
            taskSet._sampleCounts = p_SampleCounts;
            taskSet._minRoughness = p_MinRoughness;
          }

          _scheduler.AddTaskSetToPipe(&taskSet);
        }
      }
    }
  }

  _INTR_LOG_INFO("Queued %u jobs for pre filtering...",
                 (uint32_t)_preFilterParallelTaskSets.size());

  _scheduler.WaitforAll();
  _preFilterParallelTaskSets.clear();
}

void captureProbes(const Components::NodeRefArray& p_NodeRefs,
                        bool p_Clear, float p_Time)
{
  using namespace RV;

  static glm::uvec2 atlasIndices[6]{glm::uvec2(0u, 1u), glm::uvec2(1u, 1u),
                                    glm::uvec2(2u, 1u), glm::uvec2(3u, 1u),
                                    glm::uvec2(1u, 0u), glm::uvec2(1u, 2u)};
  static glm::quat rotationsPerAtlasIdx[6] = {
      glm::vec3(0.0f, -glm::half_pi<float>(), 0.0f), // L / +x
      glm::vec3(0.0f, 0.0f, 0.0f),                   // F / +z
      glm::vec3(0.0f, glm::half_pi<float>(), 0.0f),  // R / -x
      glm::vec3(0.0f, -glm::pi<float>(), 0.0f),      // B / -z
      glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f), // T / +y
      glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f)}; // B / -y
  static uint32_t atlasIndexToFaceIdx[6] = {0, 4, 1, 5, 2, 3};

  const glm::uvec2 cubeMapRes =
      RV::RenderSystem::getAbsoluteRenderSize(RV::RenderSize::kCubemap);

  const uint32_t faceSizeInBytes =
      cubeMapRes.x * cubeMapRes.y * 2u * sizeof(uint32_t);

  // Setup camera
  Entity::EntityRef entityRef =
      Entity::EntityManager::createEntity(_N(ProbeCamera));
  Components::NodeRef camNodeRef =
      Components::NodeManager::createNode(entityRef);
  Components::NodeManager::attachChild(World::_rootNode, camNodeRef);
  Components::CameraRef camRef =
      Components::CameraManager::createCamera(entityRef);
  Components::CameraManager::resetToDefault(camRef);
  Components::CameraManager::_descFov(camRef) = glm::radians(90.0f);
  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  Components::CameraRef prevCamera = World::_activeCamera;
  World::setActiveCamera(camRef);

  // Setup buffer for readback
  BufferRef readBackBufferRef =
      BufferManager::createBuffer(_N(IrradianceProbeReadBack));
  {
    BufferManager::resetToDefault(readBackBufferRef);
    BufferManager::addResourceFlags(
        readBackBufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    BufferManager::_descMemoryPoolType(readBackBufferRef) =
        RV::MemoryPoolType::kVolatileStagingBuffers;

    BufferManager::_descBufferType(readBackBufferRef) =
        RV::BufferType::kStorage;
    BufferManager::_descSizeInBytes(readBackBufferRef) = faceSizeInBytes;

    BufferManager::createResources({readBackBufferRef});
  }

  uint32_t prevDebugStageFlags = RenderPass::Debug::_activeDebugStageFlags;
  RenderPass::Debug::_activeDebugStageFlags = 0u;
  RenderPass::Clustering::_globalAmbientFactor = 0.0f;
  RenderPass::VolumetricLighting::_globalScatteringFactor = 0.0f;
  float prevTime = World::_currentTime;
  World::_currentTime = p_Time;
  float prevMaxFps = Settings::Manager::_targetFrameRate;
  Settings::Manager::_targetFrameRate = 0.0f;

  for (uint32_t i = 0u; i < p_NodeRefs.size(); ++i)
  {
    Components::NodeRef nodeRef = p_NodeRefs[i];

    Entity::EntityRef currentEntity = Components::NodeManager::_entity(nodeRef);
    Components::NodeRef irradProbeRef =
        Components::IrradianceProbeManager::getComponentForEntity(
            currentEntity);

    if (p_Clear)
    {
      if (irradProbeRef.isValid())
        Components::IrradianceProbeManager::_descSHs(irradProbeRef).clear();
    }

    Components::NodeManager::_position(camNodeRef) =
        Components::NodeManager::_worldPosition(nodeRef);
    const Components::NodeRefArray nodesToUpdate = {camNodeRef};
    Components::NodeManager::updateTransforms(nodesToUpdate);

    // Render a couple of frames so everything is correctly faded in/out
    for (uint32_t f = 0u; f < 10u; ++f)
    {
      World::updateDayNightCycle(0.0f);
      Components::PostEffectVolumeManager::blendPostEffects(
          Components::PostEffectVolumeManager::_activeRefs);
      RenderProcess::Default::renderFrame(0.0f);
      ++TaskManager::_frameCounter;
    }

    {
#if defined(STORE_ATLAS_DDS)
      gli::texture2d tex = gli::texture2d(
          gli::FORMAT_RGBA16_SFLOAT_PACK16,
          gli::texture2d::extent_type(cubeMapRes.x * 4u, cubeMapRes.y * 3u),
          1u);
#endif // STORE_ATLAS_DDS

      gli::texture_cube texCube =
          gli::texture_cube(gli::FORMAT_RGBA16_SFLOAT_PACK16, cubeMapRes, 1u);

      for (uint32_t atlasIdx = 0u; atlasIdx < 6; ++atlasIdx)
      {
        Components::NodeManager::_orientation(camNodeRef) =
            rotationsPerAtlasIdx[atlasIdx];
        Components::NodeManager::updateTransforms({camNodeRef});

        // Render face
        Components::PostEffectVolumeManager::blendPostEffects(
            Components::PostEffectVolumeManager::_activeRefs);
        RenderProcess::Default::renderFrame(0.0f);

        // Wait for the rendering to finish
        RenderSystem::waitForAllFrames();

        // Copy image to host visible memory
        VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

        ImageRef sceneImageRef = ImageManager::getResourceByName(_N(Scene));

        ImageManager::insertImageMemoryBarrier(
            copyCmd, sceneImageRef, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkBufferImageCopy bufferImageCopy = {};
        {
          bufferImageCopy.bufferOffset = 0u;
          bufferImageCopy.imageOffset = {};
          bufferImageCopy.bufferRowLength = cubeMapRes.x;
          bufferImageCopy.bufferImageHeight = cubeMapRes.y;
          bufferImageCopy.imageExtent.width = cubeMapRes.x;
          bufferImageCopy.imageExtent.height = cubeMapRes.y;
          bufferImageCopy.imageExtent.depth = 1u;
          bufferImageCopy.imageSubresource.aspectMask =
              VK_IMAGE_ASPECT_COLOR_BIT;
          bufferImageCopy.imageSubresource.baseArrayLayer = 0u;
          bufferImageCopy.imageSubresource.layerCount = 1u;
          bufferImageCopy.imageSubresource.mipLevel = 0u;
        }

        // Read back face
        vkCmdCopyImageToBuffer(copyCmd, ImageManager::_vkImage(sceneImageRef),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               BufferManager::_vkBuffer(readBackBufferRef), 1u,
                               &bufferImageCopy);

        RenderSystem::flushTemporaryCommandBuffer();

        const uint8_t* sceneMemory =
            BufferManager::getGpuMemory(readBackBufferRef);

        memcpy(texCube.data(0u, atlasIndexToFaceIdx[atlasIdx], 0u), sceneMemory,
               faceSizeInBytes);

#if defined(STORE_ATLAS_DDS)
        const glm::uvec2 atlasIndex = atlasIndices[atlasIdx];
        for (uint32_t scanLineIdx = 0u; scanLineIdx < cubeMapRes.y;
             ++scanLineIdx)
        {
          const uint32_t memOffsetInBytes =
              ((atlasIndex.y * cubeMapRes.y * (cubeMapRes.x * 4u)) +
               scanLineIdx * (cubeMapRes.x * 4u) +
               atlasIndex.x * cubeMapRes.x) *
              sizeof(uint32_t) * 2u;

          const uint32_t scanLineSizeInBytes =
              sizeof(uint32_t) * 2u * cubeMapRes.x;
          memcpy((uint8_t*)tex.data() + memOffsetInBytes,
                 sceneMemory + scanLineIdx * scanLineSizeInBytes,
                 scanLineSizeInBytes);
        }
#endif // STORE_ATLAS_DDS
      }

      // TODO: WIP specular probes
      if (/*specularProbeRef.isValid()*/ false)
      {
        // Output texture
        gli::texture_cube filteredTexCube =
            gli::texture_cube(gli::FORMAT_RGBA16_SFLOAT_PACK16, cubeMapRes,
                              gli::levels(cubeMapRes));

        const _INTR_ARRAY(uint32_t) sampleCounts = {
            16u, 512u, 1024u, 1024u, 1024u, 1024u, 1024u, 1024u, 1024u};
        _INTR_ASSERT(sampleCounts.size() == filteredTexCube.levels());

        IBL::preFilterGGX(texCube, filteredTexCube, sampleCounts.data());

        _INTR_STRING timeString = StringUtil::toString(p_Time);
        StringUtil::replace(timeString, ".", "-");

        const _INTR_STRING filePath =
            "media/irradiance_probes/" +
            Entity::EntityManager::_name(currentEntity).getString() +
            "_cube_filtered_" + timeString + ".dds";
        gli::save_dds(filteredTexCube, filePath.c_str());
      }

      if (irradProbeRef.isValid())
      {
#if defined(STORE_ATLAS_DDS)
        {
          const _INTR_STRING filePath =
              "media/irradiance_probes/" +
              Entity::EntityManager::_name(currentEntity).getString() +
              "_cube_atlas_" + timeString + ".dds";
          gli::save_dds(tex, filePath.c_str());
        }

#endif // STORE_ATLAS_DDS

#if defined(STORE_CUBE_DDS)
        {
          _INTR_STRING timeString = StringUtil::toString(p_Time);
          StringUtil::replace(timeString, ".", "-");

          const _INTR_STRING filePath =
              "media/irradiance_probes/" +
              Entity::EntityManager::_name(currentEntity).getString() +
              "_cube_" + timeString + ".dds";
          gli::save_dds(texCube, filePath.c_str());
        }
#endif // STORE_CUBE_DDS

        // Generate and store SH irrad.
        Components::IrradianceProbeManager::_descSHs(irradProbeRef)
            .push_back(IBL::project(texCube));
      }
    }
  }

  // Cleanup and restore
  BufferManager::destroyResources({readBackBufferRef});
  BufferManager::destroyBuffer(readBackBufferRef);
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);

  Settings::Manager::_targetFrameRate = prevMaxFps;
  World::_currentTime = prevTime;
  RenderPass::Clustering::_globalAmbientFactor = 1.0f;
  RenderPass::VolumetricLighting::_globalScatteringFactor = 1.0f;
  RenderPass::Debug::_activeDebugStageFlags = prevDebugStageFlags;
  World::setActiveCamera(prevCamera);
  World::destroyNodeFull(camNodeRef);
}
}
}
}
