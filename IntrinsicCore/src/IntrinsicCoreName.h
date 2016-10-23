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
struct Name
{
  _INTR_INLINE Name() : _hash(0u) {}
  _INTR_INLINE Name(const _INTR_STRING& p_String) { setName(p_String.c_str()); }
  _INTR_INLINE Name(const char* p_String) { setName(p_String); }
  _INTR_INLINE Name(uint32_t p_Hash) : _hash(p_Hash) {}

  _INTR_INLINE void setName(const char* p_String)
  {
    _string = p_String;
    _hash = Math::hash(_string.c_str(), _string.size());
  }

  _INTR_INLINE bool isValid() const { return _hash != 0u; }

  _INTR_INLINE bool operator==(const Name& p_Rhs) const
  {
    return _hash == p_Rhs._hash;
  }

  _INTR_INLINE bool operator!=(const Name& p_Rhs) const
  {
    return !(*this == p_Rhs);
  }

  _INTR_STRING _string;
  uint32_t _hash;
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
