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

// PhysX includes
#include "PxPhysics.h"
#include "characterkinematic/PxExtended.h"
#include "PxScene.h"

namespace Intrinsic
{
namespace Core
{
namespace PhysicsHelper
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
        const physx::PxQueryFlags& p_QueryFlags = physx::PxQueryFlag::eSTATIC |
                                                  physx::PxQueryFlag::eANY_HIT)
{
  const physx::PxHitFlags hitsFlags = physx::PxHitFlag::eDEFAULT;
  physx::PxQueryFilterData filterData = physx::PxQueryFilterData(p_QueryFlags);

  physx::PxRaycastBuffer hit;
  if (Physics::System::_pxScene->raycast(PhysicsHelper::convert(p_Ray.o),
                                         PhysicsHelper::convert(p_Ray.d),
                                         p_Length, hit, hitsFlags, filterData))
  {
    _INTR_ASSERT(hit.hasBlock);
    p_Result = hit.block;
    return true;
  }

  return false;
}

// <-
}
}
}
