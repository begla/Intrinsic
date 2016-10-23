// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
