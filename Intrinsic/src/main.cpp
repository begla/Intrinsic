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
