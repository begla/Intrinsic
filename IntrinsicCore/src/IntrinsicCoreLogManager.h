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
namespace Log
{
namespace LogLevel
{
enum Enum
{
  kVerbose = 0u,
  kInfo,
  kWarning,
  kError,
  kDebug
};
}

typedef void (*LogCallbackFunction)(const char*, LogLevel::Enum);

struct LogListenerEntry
{
  LogCallbackFunction callbackFunction;
};

struct Manager
{
  static void log(LogLevel::Enum p_LogLevel, const char* p_Message, ...);
  static void addLogListener(const LogListenerEntry& p_Entry);
  static void removeLogListener(const LogListenerEntry& p_Entry);

  static void indent();
  static void unindent();
};
}
}
}
