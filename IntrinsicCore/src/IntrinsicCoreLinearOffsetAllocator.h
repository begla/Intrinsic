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
struct LinearOffsetAllocator
{
  LinearOffsetAllocator()
      : _sizeInBytes(0u), _currentOffsetInBytes(0u), _initialOffset(0u)
  {
  }

  // <-

  _INTR_INLINE void init(uint32_t p_Size, uint32_t p_InitialOffset = 0u)
  {
    _initialOffset = p_InitialOffset;
    _currentOffsetInBytes = _initialOffset;
    _sizeInBytes = p_Size;
  }

  // <-

  _INTR_INLINE uint32_t allocate(uint32_t p_Size, uint32_t p_Alignment)
  {
    // Align current offset
    const uint32_t newOffset =
        (_currentOffsetInBytes + p_Alignment) & ~(p_Alignment - 1u);
    _currentOffsetInBytes = newOffset + p_Size;
    _INTR_ASSERT((_currentOffsetInBytes - _initialOffset) <= _sizeInBytes &&
                 "Out of memory");
    return newOffset;
  }

  // <-

  _INTR_INLINE void reset() { _currentOffsetInBytes = _initialOffset; }

  // <-

  _INTR_INLINE uint32_t size() const { return _sizeInBytes; }

  // <-

  _INTR_INLINE uint32_t currentOffset() const { return _currentOffsetInBytes; }

  // <-

  _INTR_INLINE uint32_t calcAvailableMemoryInBytes() const
  {
    return _sizeInBytes - (_currentOffsetInBytes - _initialOffset);
  }

private:
  uint32_t _initialOffset;
  uint32_t _sizeInBytes;
  uint32_t _currentOffsetInBytes;
};
}
}
