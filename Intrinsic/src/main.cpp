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

// No console window
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif // _MSC_VER

int main(int argc, char* argv[])
{
  // Loading settings file
  Settings::Manager::loadSettings();

  // Init. SDL and window
  int sdlResult =
      SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
  _INTR_ASSERT(sdlResult == 0u);

  SDL_DisplayMode displayMode;
  SDL_GetCurrentDisplayMode(0, &displayMode);

  uint32_t windowFlags =
      SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_GRABBED;
  if (Settings::Manager::_windowMode == Settings::WindowMode::kFullscreen)
  {
    windowFlags |= SDL_WINDOW_FULLSCREEN;
  }
  else if (Settings::Manager::_windowMode ==
           Settings::WindowMode::kFullscreenDesktop)
  {
    Settings::Manager::_screenResolutionWidth = displayMode.w;
    Settings::Manager::_screenResolutionHeight = displayMode.h;

    windowFlags |= SDL_WINDOW_FULLSCREEN;
  }
  else if (Settings::Manager::_windowMode ==
           Settings::WindowMode::kBorderlessWindow)
  {
    Settings::Manager::_screenResolutionWidth = displayMode.w;
    Settings::Manager::_screenResolutionHeight = displayMode.h;

    windowFlags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED;
  }

  SDL_Window* sdlWindow = SDL_CreateWindow(
      "Intrinsic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      Settings::Manager::_screenResolutionWidth,
      Settings::Manager::_screenResolutionHeight, windowFlags);
  _INTR_ASSERT(sdlWindow);

  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(sdlWindow, &wmInfo);

  // Init. event system
  Application::initEventSystem();

// Init. Intrinsic
#if (_WIN32)
  Application::init(GetModuleHandle(NULL), (void*)wmInfo.info.win.window);
#else
  Application::init((void*)wmInfo.info.x11.display,
                    (void*)wmInfo.info.x11.window);
#endif // _WIN32

  // Activate main game state
  GameStates::Manager::activateGameState(GameStates::GameState::kMain);

  SDL_ShowWindow(sdlWindow);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  while (Application::_running)
  {
    TaskManager::executeTasks();
  }

  return 0;
}
