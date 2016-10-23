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
struct Application
{
  static _INTR_HASH_MAP(Name,
                        Intrinsic::Core::Dod::Components::ComponentManagerEntry)
      _componentManagerMapping;
  static _INTR_HASH_MAP(Name,
                        Intrinsic::Core::Dod::Resources::ResourceManagerEntry)
      _resourceManagerMapping;

  static _INTR_HASH_MAP(Name, Intrinsic::Core::Dod::PropertyCompilerEntry)
      _componentPropertyCompilerMapping;
  static _INTR_HASH_MAP(Name, Intrinsic::Core::Dod::PropertyCompilerEntry)
      _resourcePropertyCompilerMapping;

  static _INTR_ARRAY(Intrinsic::Core::Dod::Components::ComponentManagerEntry)
      _orderedComponentManagers;

  static void init(void* p_PlatformHandle, void* p_PlatformWindow);
  static void initEventSystem();
  static void shutdown();

  static enki::TaskScheduler _scheduler;
  static bool _running;

private:
  static void initManagers();
};
}
}
