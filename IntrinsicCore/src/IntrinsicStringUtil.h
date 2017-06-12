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
namespace StringUtil
{
template <class T> _INTR_INLINE _INTR_STRING toString(T p_Value)
{
  _INTR_STRING_STREAM ss;
  ss << p_Value;
  return ss.str();
}

template <class T> _INTR_INLINE T fromString(const _INTR_STRING& p_String)
{
  T value;
  _INTR_STRING_STREAM ss;
  ss << p_String;
  ss >> value;
  return value;
}

_INTR_INLINE void split(const _INTR_STRING& p_String, const char* p_Delim,
                        _INTR_ARRAY(_INTR_STRING) & p_Tokens)
{
  const uint32_t bytesToCopy =
      sizeof(char) * ((uint32_t)p_String.length() + 1u);

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

_INTR_INLINE void extractFileNameAndExtension(const _INTR_STRING& p_String,
                                              _INTR_STRING& p_FileName,
                                              _INTR_STRING& p_Extension)
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

_INTR_INLINE void replace(_INTR_STRING& p_String,
                          const _INTR_STRING& p_StringToReplace,
                          const _INTR_STRING& p_Replacement)
{
  size_t start_pos = p_String.find(p_StringToReplace);
  if (start_pos == std::string::npos)
    return;
  p_String.replace(start_pos, p_StringToReplace.length(), p_Replacement);
}
}
}
}
