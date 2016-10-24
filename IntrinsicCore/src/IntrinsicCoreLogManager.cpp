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

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Log
{
// Static members
// Intrinsic

uint32_t Manager::_currentIndent = 0u;
_INTR_STRING Manager::_lastLoggedLine = "";
_INTR_ARRAY(LogListenerEntry) Manager::_logListeners;

// <-

void Manager::log(LogLevel::Enum p_LogLevel, const char* p_Message, ...)
{
  if (!p_Message || *p_Message == '\0')
  {
    return;
  }

  _INTR_STRING message = p_Message;

  // Strip new line
  if (message[message.size() - 1u] == '\n')
  {
    message.resize(message.size() - 1u);
  }

  static const char* logLevelToLogLevelNameMapping[] = {
      "Verbose", "Info", "Warning", "Error", "Debug"};
  static const uint32_t logLevelToColorMapping[] = {
      rlutil::WHITE, rlutil::GREEN, rlutil::YELLOW, rlutil::RED, rlutil::CYAN};

  time_t t;
  time(&t);
  tm* localT;
  localT = localtime(&t);

  char timeStr[64];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %X", localT);

  rlutil::setColor(rlutil::DARKGREY);

  printf("%s (%s): ", logLevelToLogLevelNameMapping[p_LogLevel], timeStr);

  // Indent
  for (uint32_t i = 0u; i < _currentIndent; ++i)
  {
    printf(" ");
  }

  rlutil::setColor(logLevelToColorMapping[p_LogLevel]);

  static const uint32_t messageBufferSizeInByte = 4096u;
  static char _messageBuffer[messageBufferSizeInByte];

  va_list args;
  va_start(args, p_Message);
  vsprintf(_messageBuffer, message.c_str(), args);
  va_end(args);

  printf("%s", _messageBuffer);

  rlutil::setColor(rlutil::DARKGREY);

  printf("\n");

  if (p_LogLevel == Intrinsic::Core::Log::LogLevel::kError)
  {
    _INTR_ERROR_DIALOG(_messageBuffer);
  }

  _lastLoggedLine = message;

  // Call listeners
  for (uint32_t i = 0u; i < _logListeners.size(); ++i)
  {
    _logListeners[i].callbackFunction(_INTR_STRING(_messageBuffer), p_LogLevel);
  }
}
}
}
}
