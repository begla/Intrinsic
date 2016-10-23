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

#include "stdafx.h"
#include "stdafx_editor.h"

QSplashScreen* splash = nullptr;

void onLoggedSplashscreen(const _INTR_STRING& p_Message,
                          Log::LogLevel::Enum p_LogLevel)
{
  splash->showMessage(p_Message.c_str(), Qt::AlignBottom | Qt::AlignLeft,
                      Qt::white);
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
