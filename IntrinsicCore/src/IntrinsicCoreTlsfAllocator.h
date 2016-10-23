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

  _INTR_INLINE Allocator(uint32_t p_Size)
  {
    void* mem = malloc(p_Size);
    init(mem, p_Size);
  }

  _INTR_INLINE Allocator(void* p_Mem, uint32_t p_Size) { init(p_Mem, p_Size); }

  _INTR_INLINE void init(void* p_Mem, uint32_t p_Size)
  {
    _mem = p_Mem;
    _memoryPool = tlsf_create_with_pool(p_Mem, p_Size);
    _INTR_ASSERT(_memoryPool && "Tlsf init. failed");
  }

  _INTR_INLINE void* allocate(uint32_t p_Size)
  {
    void* mem = tlsf_malloc(_memoryPool, p_Size);
    _INTR_ASSERT(mem && "Tlsf allocation failed");
    return mem;
  }

  _INTR_INLINE void* allocateAligned(uint32_t p_Size, uint32_t p_Alignment)
  {
    return tlsf_memalign(_memoryPool, p_Alignment, p_Size);
  }

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

  static Allocator* _allocator;
};
}
}
}
