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

#define _INTR_TLSF_SIZE_IN_MB 256u
#define _INTR_TLSF_INIT_ON_DEMAND()                                            \
  {                                                                            \
    if (_allocator == nullptr)                                                 \
      _allocator = new Allocator(_INTR_TLSF_SIZE_IN_MB * 1024u * 1024u);       \
  }

namespace Intrinsic
{
namespace Core
{
namespace Tlsf
{
struct Allocator
{
  _INTR_INLINE Allocator() {}

  // <-

  _INTR_INLINE Allocator(uint32_t p_Size)
  {
    void* mem = malloc(p_Size);
    init(mem, p_Size);
  }

  // <-

  _INTR_INLINE Allocator(void* p_Mem, uint32_t p_Size) { init(p_Mem, p_Size); }

  // <-

  _INTR_INLINE void init(void* p_Mem, uint32_t p_Size)
  {
    _mem = p_Mem;
    _memoryPool = tlsf_create_with_pool(p_Mem, p_Size);
    _INTR_ASSERT(_memoryPool && "Tlsf init. failed");
  }

  // <-

  _INTR_INLINE void* allocate(uint32_t p_Size)
  {
    void* mem = tlsf_malloc(_memoryPool, p_Size);
    _INTR_ASSERT(mem && "Tlsf allocation failed");
    return mem;
  }

  // <-

  _INTR_INLINE void* allocateAligned(uint32_t p_Size, uint32_t p_Alignment)
  {
    return tlsf_memalign(_memoryPool, p_Alignment, p_Size);
  }

  // <-

  _INTR_INLINE void free(void* p_Mem)
  {
    _INTR_ASSERT(p_Mem && "Tried to free nullptr");
    tlsf_free(_memoryPool, p_Mem);
  }

  tlsf_t _memoryPool;
  void* _mem;
};

struct MainAllocator
{
  static void* allocate(uint32_t p_Size)
  {
    _INTR_TLSF_INIT_ON_DEMAND();

    return _allocator->allocate(p_Size);
  }

  static void free(void* p_Mem) { _allocator->free(p_Mem); }

private:
  static Allocator* _allocator;
};
}
}
}
