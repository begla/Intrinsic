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
struct MemoryBlock
{
  uint8_t* memory;
  uint32_t memoryOffset;
};

template <uint32_t BlockCount, uint32_t BlockSizeInBytes>
struct LockFreeFixedBlockAllocator
{
  LockFreeFixedBlockAllocator() {}

  // <-

  _INTR_INLINE void init(uint8_t* p_Memory = nullptr, uint32_t p_Offset = 0u)
  {
    _memoryBlocks.resize(BlockCount);

    uint32_t currentOffset = 0u;
    for (uint32_t i = 0u; i < BlockCount; ++i)
    {
      const uint32_t actualOffset = p_Offset + currentOffset;
      _memoryBlocks[i] = {p_Memory != nullptr ? &p_Memory[actualOffset]
                                              : nullptr,
                          actualOffset};
      currentOffset += BlockSizeInBytes;
    }
  }

  // <-

  _INTR_INLINE MemoryBlock allocate()
  {
    _INTR_ASSERT(!_memoryBlocks.empty());
    return _memoryBlocks.pop_back();
  }

  // <-

  _INTR_INLINE void free(const MemoryBlock& p_Block)
  {
    _memoryBlocks.push_back(p_Block);
  }

  // <-

  _INTR_INLINE void reset() { _memoryBlocks.resize(BlockCount); }

  // <-

  _INTR_INLINE uint32_t blockSize() { return BlockSizeInBytes; }

  // <-

  _INTR_INLINE uint32_t totalBlockCount() { return BlockCount; }

  // <-

  _INTR_INLINE uint32_t availablePageCount() { return _memoryBlocks.size(); }

  // <-

  _INTR_INLINE uint32_t calcAvailableMemoryInBytes()
  {
    return _memoryBlocks.size() * BlockSizeInBytes;
  }

private:
  LockFreeStack<MemoryBlock, BlockCount> _memoryBlocks;
};
}
}
