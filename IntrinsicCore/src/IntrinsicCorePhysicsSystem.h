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

// Forward decls.
namespace physx
{
class PxPhysics;
class PxFoundation;
class PxCooking;
class PxScene;
class PxCpuDispatcher;
}

namespace Intrinsic
{
namespace Core
{
namespace Physics
{
namespace DebugRenderingFlags
{
enum Flags
{
  kEnabled = 0x01
};
}

struct System
{
  static void init();
  static void dispatchSimulation(float p_DeltaT);
  static void syncSimulation();
  static void updatePvdCamera();
  static void renderLineDebugGeometry();

  static void setDebugRenderingFlags(uint32_t p_DebugRenderingFlags);

  static physx::PxPhysics* _pxPhysics;
  static physx::PxFoundation* _pxFoundation;
  static physx::PxCooking* _pxCooking;
  static physx::PxCpuDispatcher* _pxCpuDispatcher;
  static physx::PxScene* _pxScene;
};
}
}
}
