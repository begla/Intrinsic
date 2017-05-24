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

#include "stdafx.h"
#include "stdafx_editor.h"

#if defined(_WIN32)
//#define GENERATE_CRASH_DUMPS
#include "BugSplat.h"
#endif // _WIN32

#if defined(GENERATE_CRASH_DUMPS)
#include "IntrinsicCoreCrashDumpGeneratorWin32.h"
#endif // GENERATE_CRASH_DUMPS

QSplashScreen* splash = nullptr;

void onLoggedSplashscreen(const char* p_Message, Log::LogLevel::Enum p_LogLevel)
{
  splash->showMessage(QString(_INTR_VERSION_STRING) + " | " + p_Message,
                      Qt::AlignBottom | Qt::AlignLeft, Qt::white);
}

int _main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationDomain("com");
  QCoreApplication::setOrganizationName("intrinsic-engine");
  QCoreApplication::setApplicationName("Intrinsic");
  QCoreApplication::setApplicationVersion(_INTR_VERSION_STRING);

  // Style
  {
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(37, 41, 45));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42, 46, 50));
    darkPalette.setColor(QPalette::AlternateBase, QColor(42, 46, 50));

    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(236, 112, 0));
    darkPalette.setColor(QPalette::Highlight, QColor(236, 112, 0));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setStyleSheet("QToolTip { color: #ffffff; background-color: #202428; "
                    "border: 1px solid #EC7000; } QToolBar { border: 0px }");
    a.setPalette(darkPalette);

    int id = QFontDatabase::addApplicationFont(":/Fonts/fira");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont monospace(family);
    monospace.setPointSize(10);
    a.setFont(monospace);
  }

  QPixmap splashscreen(":/Media/splashscreen");
  _INTR_ASSERT(!splashscreen.isNull());

  Intrinsic::Core::Log::Manager::addLogListener({onLoggedSplashscreen});

  splash = new QSplashScreen(splashscreen /*, Qt::WindowStaysOnTopHint*/);
  splash->show();
  a.processEvents();

  IntrinsicEd w;
  w.show();

  splash->finish(&w);

  Log::Manager::removeLogListener({onLoggedSplashscreen});

  return w.enterMainLoop();
}

#if defined(GENERATE_CRASH_DUMPS)
int main(int argc, char* argv[])
{
  __try
  {
    return _main(argc, argv);
  }
  __except (Intrinsic::Core::CrashDumpGeneratorWin32::GenerateDump(
      GetExceptionInformation()))
  {
    return -1;
  }
}
#else

#if defined(BUGSPLAT_H)
MiniDmpSender* miniDmpSender = nullptr;

bool ExceptionCallback(UINT nCode, LPVOID lpVal1, LPVOID lpVal2)
{
  wchar_t full[_MAX_PATH];
  miniDmpSender->sendAdditionalFile(
      _wfullpath(full, L"Intrinsic.log", _MAX_PATH));
  return false;
}
#endif // BUGSPLAT_H

int main(int argc, char* argv[])
{
#if defined(BUGSPLAT_H)
  miniDmpSender =
      new MiniDmpSender(L"Intrinsic", L"IntrinsicEd", _INTR_VERSION_STRING_L);
  miniDmpSender->setCallback(ExceptionCallback);
#endif // BUGSPLAT_

  return _main(argc, argv);
}
#endif // GENERATE_CRASH_DUMPS
