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

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{
struct Benchmark
{
  struct Path
  {
    _INTR_STRING name;
    _INTR_ARRAY(glm::vec3) nodePositions;
    float camSpeed;
  };

  struct Data
  {
    Data() { meanFps = 0.0f; }

    _INTR_INLINE uint32_t calcScore() { return (uint32_t)(meanFps * 1337.0f); }
    float meanFps;
  };

  static void init();
  static void activate();
  static void deativate();

  static void parseBenchmark(rapidjson::Document& p_BenchmarkDesc);
  static void assembleBenchmarkPaths(const rapidjson::Document& p_BenchmarkDesc,
                                     _INTR_ARRAY(Path) & p_Paths);
  static void update(float p_DeltaT);
};
}
}
}
