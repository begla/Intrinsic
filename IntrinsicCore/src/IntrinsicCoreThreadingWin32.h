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

namespace Intrinsic
{
namespace Core
{
namespace Threading
{
typedef volatile int64_t Atomic;

_INTR_INLINE Atomic interlockedAdd(Atomic& p_Value, Atomic p_Add)
{
  return InterlockedAddNoFence64(&p_Value, p_Add) - p_Add;
}

_INTR_INLINE Atomic interlockedSub(Atomic& p_Value, Atomic p_Sub)
{
  return InterlockedAddNoFence64(&p_Value, -p_Sub) + p_Sub;
}
}
}
}
