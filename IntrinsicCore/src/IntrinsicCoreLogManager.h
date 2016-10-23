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

typedef void (*LogCallbackFunction)(const _INTR_STRING&, LogLevel::Enum);

struct LogListenerEntry
{
  LogCallbackFunction callbackFunction;
};

struct Manager
{
  static void log(LogLevel::Enum p_LogLevel, const char* p_Message, ...);

  _INTR_INLINE static void addLogListener(const LogListenerEntry& p_Entry)
  {
    _logListeners.push_back(p_Entry);
  }

  _INTR_INLINE static void removeLogListener(const LogListenerEntry& p_Entry)
  {
    for (auto it = _logListeners.begin(); it != _logListeners.end();)
    {
      if (it->callbackFunction == p_Entry.callbackFunction)
      {
        it = _logListeners.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  static uint32_t _currentIndent;
  static _INTR_STRING _lastLoggedLine;
  static _INTR_ARRAY(LogListenerEntry) _logListeners;
};
}
}
}
