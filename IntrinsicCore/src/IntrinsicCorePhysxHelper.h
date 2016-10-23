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

// PhysX includes
#include "PxPhysics.h"
#include "characterkinematic/PxExtended.h"
#include "PxScene.h"

namespace Intrinsic
{
namespace Core
{
namespace PhysxHelper
{
_INTR_INLINE glm::vec3 convert(const physx::PxVec3& p_Value)
{
  return glm::vec3(p_Value.x, p_Value.y, p_Value.z);
}

// <-

_INTR_INLINE glm::vec3 convertExt(const physx::PxExtendedVec3& p_Value)
{
  return glm::vec3(p_Value.x, p_Value.y, p_Value.z);
}

// <-

_INTR_INLINE physx::PxVec3 convert(const glm::vec3& p_Value)
{
  return physx::PxVec3(p_Value.x, p_Value.y, p_Value.z);
}

// <-

_INTR_INLINE physx::PxExtendedVec3 convertExt(const glm::vec3& p_Value)
{
  return physx::PxExtendedVec3((physx::PxExtended)p_Value.x,
                               (physx::PxExtended)p_Value.y,
                               (physx::PxExtended)p_Value.z);
}

// <-

_INTR_INLINE glm::quat convert(const physx::PxQuat& p_Value)
{
  return glm::quat(p_Value.w, p_Value.x, p_Value.y, p_Value.z);
}

// <-

_INTR_INLINE physx::PxQuat convert(const glm::quat& p_Value)
{
  return physx::PxQuat(p_Value.x, p_Value.y, p_Value.z, p_Value.w);
}

// <-

_INTR_INLINE physx::PxTransform convert(const glm::vec3& p_Pos,
                                        const glm::quat& p_Orient)
{
  return physx::PxTransform(convert(p_Pos), convert(p_Orient));
}

// <-

_INTR_INLINE void convert(const physx::PxTransform& p_Transform,
                          glm::vec3& p_Pos, glm::quat& p_Orient)
{
  p_Orient = convert(p_Transform.q);
  p_Pos = convert(p_Transform.p);
}

// <-

_INTR_INLINE bool
raycast(const Math::Ray& p_Ray, physx::PxRaycastHit& p_Result,
        float p_Length = 1000.0f,
        const physx::PxQueryFlags& p_QueryFlags = physx::PxQueryFlag::eSTATIC)
{
  const physx::PxHitFlags hitsFlags = physx::PxHitFlag::eDEFAULT;
  physx::PxQueryFilterData filterData = physx::PxQueryFilterData(p_QueryFlags);

  if (Physics::System::_pxScene->raycastSingle(
          PhysxHelper::convert(p_Ray.o), PhysxHelper::convert(p_Ray.d),
          p_Length, hitsFlags, p_Result, filterData))
  {
    return true;
  }

  return false;
}

// <-
}
}
}
