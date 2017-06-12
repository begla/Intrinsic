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

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Log
{
namespace
{
_INTR_ARRAY(LogListenerEntry) _logListeners;
uint32_t _currentIndent = 0u;
}

// <-

void Manager::log(LogLevel::Enum p_LogLevel, const char* p_Message, ...)
{
  static _INTR_OFSTREAM logFile =
      _INTR_OFSTREAM("Intrinsic.log", std::ios_base::out);
  static const uint32_t messageBufferSizeInByte = 4096u;
  static char messageBuffer[messageBufferSizeInByte];

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
    printf(" ");

  rlutil::setColor(logLevelToColorMapping[p_LogLevel]);

  va_list args;
  va_start(args, p_Message);
  vsprintf(messageBuffer, message.c_str(), args);
  va_end(args);

  printf("%s\n", messageBuffer);
  rlutil::setColor(rlutil::DARKGREY);

  // Log to file
  {
    logFile << logLevelToLogLevelNameMapping[p_LogLevel] << " (" << timeStr
            << "): ";
    for (uint32_t i = 0u; i < _currentIndent; ++i)
      logFile << " ";
    logFile << messageBuffer << "\n";
    logFile.flush();
  }

  if (p_LogLevel == Log::LogLevel::kError)
  {
    _INTR_ERROR_DIALOG(messageBuffer);
  }

  // Call listeners
  for (uint32_t i = 0u; i < _logListeners.size(); ++i)
  {
    _logListeners[i].callbackFunction(messageBuffer, p_LogLevel);
  }
}

// <-

void Manager::addLogListener(const LogListenerEntry& p_Entry)
{
  _logListeners.push_back(p_Entry);
}

// <-

void Manager::removeLogListener(const LogListenerEntry& p_Entry)
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

// <-

void Manager::indent() { ++_currentIndent; }

// <-

void Manager::unindent()
{
  _INTR_ASSERT(_currentIndent > 0u);
  --_currentIndent;
}
}
}
}
