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
template <class T, uint64_t Capacity> struct LockFreeStack
{
  LockFreeStack()
  {
    _data = (T*)Tlsf::MainAllocator::allocate(Capacity * sizeof(T));
    _capacity = Capacity;
    _size = 0u;
  }

  // <-

  ~LockFreeStack()
  {
    Tlsf::MainAllocator::free(_data);
    _data = nullptr;
  }

  // <-

  _INTR_INLINE void push_back(const T& p_Element)
  {
    const Threading::Atomic oldSize = Threading::interlockedAdd(_size, 1);
    _INTR_ASSERT(oldSize + 1u <= Capacity && "Stack overflow");
    _data[oldSize] = p_Element;
  }

  // <-

  _INTR_INLINE T pop_back()
  {
    const uint64_t oldSize = Threading::interlockedSub(_size, 1);
    _INTR_ASSERT(oldSize - 1u <= Capacity && "Stack underflow");
    return _data[oldSize - 1u];
  }

  // <-

  _INTR_INLINE void resize(uint64_t p_Size) { _size = p_Size; }

  // <-

  _INTR_INLINE void clear() { resize(0u); }

  // <-

  _INTR_INLINE bool empty() { return _size == 0u; }

  // <-

  _INTR_INLINE T& back()
  {
    _INTR_ASSERT(!empty());
    return _data[_size - 1u];
  }

  // <-

  _INTR_INLINE uint64_t capacity() { return _capacity; }

  // <-

  _INTR_INLINE uint64_t size() { return _size; }

  // <-

  _INTR_INLINE T& operator[](uint64_t p_Idx) { return _data[p_Idx]; }

  // <-

  _INTR_INLINE void insert(const _INTR_ARRAY(T) & p_Array)
  {
    uint64_t oldSize = Threading::interlockedAdd(_size, p_Array.size());

    for (uint64_t i = 0u; i < p_Array.size(); ++i)
    {
      _data[oldSize + i] = p_Array[i];
    }
  }

  // <-

  _INTR_INLINE void copy(_INTR_ARRAY(T) & p_Array)
  {
    for (uint64_t i = 0u; i < _size; ++i)
    {
      p_Array.push_back(_data[i]);
    }
  }

  // <-

  T* _data;

private:
  uint64_t _capacity;
  Threading::Atomic _size;
};
}
}
