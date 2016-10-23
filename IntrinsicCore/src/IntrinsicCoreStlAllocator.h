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
template <class T> class StlAllocator
{
public:
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U> struct rebind
  {
    typedef StlAllocator<U> other;
  };

  pointer address(reference value) const { return &value; }
  const_pointer address(const_reference value) const { return &value; }

  StlAllocator() throw() {}
  StlAllocator(const StlAllocator&) throw() {}
  template <class U> StlAllocator(const StlAllocator<U>&) throw() {}
  ~StlAllocator() throw() {}

  size_type max_size() const throw()
  {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = 0)
  {
    return (T*)Tlsf::MainAllocator::allocate((uint32_t)num * sizeof(T));
  }

  void construct(pointer p, const T& value) { new ((void*)p) T(value); }

  void destroy(pointer p) { p->~T(); }

  void deallocate(pointer p, size_type num)
  {
    return Tlsf::MainAllocator::free(p);
  }
};

template <class T1, class T2>
bool operator==(const StlAllocator<T1>&, const StlAllocator<T2>&) throw()
{
  return true;
}
template <class T1, class T2>
bool operator!=(const StlAllocator<T1>&, const StlAllocator<T2>&) throw()
{
  return false;
}
}
}
