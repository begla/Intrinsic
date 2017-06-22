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

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h>

namespace Intrinsic
{
namespace Core
{
namespace CrashDumpGeneratorWin32
{
int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
  BOOL bMiniDumpSuccessful;
  char* szPath = "crash_dumps";
  char szFileName[MAX_PATH];
  DWORD dwBufferSize = MAX_PATH;
  HANDLE hDumpFile;
  SYSTEMTIME stLocalTime;
  MINIDUMP_EXCEPTION_INFORMATION ExpParam;

  GetLocalTime(&stLocalTime);
  StringCchPrintf(szFileName, MAX_PATH, "%s", szPath);
  CreateDirectory(szFileName, NULL);

  StringCchPrintf(szFileName, MAX_PATH,
                  "%s\\%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", szPath,
                  stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
                  stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
                  GetCurrentProcessId(), GetCurrentThreadId());
  hDumpFile =
      CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
                 FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

  ExpParam.ThreadId = GetCurrentThreadId();
  ExpParam.ExceptionPointers = pExceptionPointers;
  ExpParam.ClientPointers = TRUE;

  bMiniDumpSuccessful =
      MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
                        MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

  _INTR_ERROR_DIALOG_SIMPLE("Intrinsic(Ed) crashed. A minidump has been "
                            "generated in 'app/crash_dumps'...");

  return EXCEPTION_EXECUTE_HANDLER;
}
}
}
}
