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
