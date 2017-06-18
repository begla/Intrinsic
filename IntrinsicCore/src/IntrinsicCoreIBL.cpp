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
}
}
}
