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
#include "stdafx.h"

#define _INTR_MAX_TIMER_COUNT 64

namespace Intrinsic
{
namespace Core
{
namespace TimingHelper
{
uint64_t previousCounter[_INTR_MAX_TIMER_COUNT] = {};
uint32_t currentTimerIndex = 0u;

void timerStart()
{
  previousCounter[currentTimerIndex] = getMicroseconds();
  ++currentTimerIndex;
}

uint32_t timerEnd()
{
  --currentTimerIndex;
  return (uint32_t)(getMicroseconds() - previousCounter[currentTimerIndex]) /
         1000u;
}

void sleep(uint32_t p_Ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(p_Ms));
};

uint64_t getMicroseconds()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

uint64_t getMilliseconds() { return getMicroseconds() / 1000u; }
}
}
}
