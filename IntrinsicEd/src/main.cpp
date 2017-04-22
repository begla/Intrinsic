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

QSplashScreen* splash = nullptr;

void onLoggedSplashscreen(const char* p_Message, Log::LogLevel::Enum p_LogLevel)
{
  splash->showMessage(p_Message, Qt::AlignBottom | Qt::AlignLeft, Qt::white);
}

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationDomain("net");
  QCoreApplication::setOrganizationName("movingblocks");
  QCoreApplication::setApplicationName("Intrinsic");
  QCoreApplication::setApplicationVersion("1.0.0");

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
