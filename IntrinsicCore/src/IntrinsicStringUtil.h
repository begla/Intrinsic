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
    namespace StringUtil
    {
      template<class T>
      _INTR_INLINE _INTR_STRING toString(T p_Value)
      {
        _INTR_STRING_STREAM ss;
        ss << p_Value;
        return ss.str();
      }

      _INTR_INLINE void split(const _INTR_STRING& p_String, const char* p_Delim, _INTR_ARRAY(_INTR_STRING)& p_Tokens)
      {
        const uint32_t bytesToCopy = sizeof(char) * ((uint32_t)p_String.length() + 1u);

        char* tempStringBuffer = (char*)Tlsf::MainAllocator::allocate(bytesToCopy);
        memcpy(tempStringBuffer, p_String.c_str(), bytesToCopy);

        char* splitString = tempStringBuffer;

        char* nextToken = nullptr;
        char* token = _INTR_STRING_TOK(splitString, p_Delim, &nextToken);

        while (token != nullptr)
        {
          p_Tokens.push_back(token);
          token = _INTR_STRING_TOK(nullptr, p_Delim, &nextToken);
        }

        Tlsf::MainAllocator::free(tempStringBuffer);
      }

      _INTR_INLINE void extractFileNameAndExtension(const _INTR_STRING& p_String, _INTR_STRING& p_FileName, _INTR_STRING& p_Extension)
      {
        _INTR_STRING path = p_String;

        size_t sep = path.find_last_of("\\/");
        if (sep != std::string::npos)
          path = path.substr(sep + 1, p_String.size() - sep - 1);

        size_t dot = path.find_first_of(".");
        if (dot != std::string::npos)
        {
          p_FileName = path.substr(0, dot);
          p_Extension = path.substr(dot, path.size() - dot);
        }
        else
        {
          p_FileName = path;
          p_Extension = "";
        }
      }

      _INTR_INLINE _INTR_STRING stripNumberSuffix(const _INTR_STRING& p_String)
      {
        size_t last_index = p_String.find_last_not_of("0123456789");
        return p_String.substr(0u, last_index + 1u);
      }
    }
  }
}
