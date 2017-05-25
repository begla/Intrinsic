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
struct Name
{
  _INTR_INLINE Name() : _hash(0u) {}
  _INTR_INLINE Name(const _INTR_STRING& p_String) { setName(p_String.c_str()); }
  _INTR_INLINE Name(const char* p_String) { setName(p_String); }
  _INTR_INLINE Name(uint32_t p_Hash) : _hash(p_Hash) {}

  _INTR_INLINE void setName(const char* p_String)
  {
    _hash = Math::hash(p_String, strlen(p_String));
    if (_stringMap.find(_hash) == _stringMap.end())
      _stringMap[_hash] = p_String;
  }

  _INTR_INLINE bool isValid() const { return _hash != 0u; }

  _INTR_INLINE _INTR_STRING getString() const { return _stringMap[_hash]; }

  _INTR_INLINE bool operator==(const Name& p_Rhs) const
  {
    return _hash == p_Rhs._hash;
  }

  _INTR_INLINE bool operator!=(const Name& p_Rhs) const
  {
    return !(*this == p_Rhs);
  }

  uint32_t _hash;
  static _INTR_HASH_MAP(uint32_t, _INTR_STRING) _stringMap;
};
}
}

namespace std
{
template <> class hash<Intrinsic::Core::Name>
{
public:
  size_t operator()(const Intrinsic::Core::Name& p_Name) const
  {
    return p_Name._hash;
  }
};
};
