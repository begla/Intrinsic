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
namespace Simd
{
_INTR_INLINE __m128 simdSet(float x, float y, float z, float w)
{
  return _mm_set_ps(w, z, y, x);
}

// <-

_INTR_INLINE __m128 simdSplatX(__m128 v)
{
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
}
_INTR_INLINE __m128 simdSplatY(__m128 v)
{
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
}
_INTR_INLINE __m128 simdSplatZ(__m128 v)
{
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
}
_INTR_INLINE __m128 simdSplatW(__m128 v)
{
  return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
}

// <-

_INTR_INLINE __m128 simdMadd(__m128 a, __m128 b, __m128 c)
{
  return _mm_add_ps(_mm_mul_ps(a, b), c);
}
}
}
}
