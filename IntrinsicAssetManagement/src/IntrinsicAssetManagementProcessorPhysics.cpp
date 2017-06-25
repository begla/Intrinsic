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

// Precompiled header file
#include "stdafx_assets.h"

// PhysX includes
#include "PxPhysics.h"
#include "PxScene.h"
#include "cooking/PxCooking.h"
#include "common/PxTolerancesScale.h"
#include "extensions/PxDefaultStreams.h"
#include "geometry/PxTriangleMesh.h"
#include "geometry/PxConvexMesh.h"

using namespace RResources;
using namespace CResources;

namespace Intrinsic
{
namespace AssetManagement
{
namespace Processors
{
void Physics::createPhysicsTriangleMeshes(
    const CResources::MeshRefArray& p_MeshRefs)
{
  for (MeshRef meshRef : p_MeshRefs)
  {
    // Don't even try to create empty triangle meshes
    if (MeshManager::_descPositionsPerSubMesh(meshRef).empty() ||
        MeshManager::_descIndicesPerSubMesh(meshRef).empty())
      continue;

    const _INTR_STRING meshFilePath =
        "media/physics_meshes/" +
        CResources::MeshManager::_name(meshRef).getString() + ".pm";

    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.points.count =
        (uint32_t)MeshManager::_descPositionsPerSubMesh(meshRef)[0].size();
    meshDesc.points.stride = sizeof(glm::vec3);
    meshDesc.points.data =
        MeshManager::_descPositionsPerSubMesh(meshRef)[0].data();

    meshDesc.triangles.count =
        (uint32_t)MeshManager::_descIndicesPerSubMesh(meshRef)[0].size() / 3u;
    meshDesc.triangles.stride = 3u * sizeof(uint32_t);
    meshDesc.triangles.data =
        MeshManager::_descIndicesPerSubMesh(meshRef)[0].data();

    bool result = false;
    {
      physx::PxDefaultFileOutputStream outStream =
          physx::PxDefaultFileOutputStream(meshFilePath.c_str());
      result = Core::Physics::System::_pxCooking->cookTriangleMesh(meshDesc,
                                                                   outStream);
    }

    if (!result)
    {
      _INTR_LOG_WARNING(
          "Failed to cook physics triangle mesh for mesh \"%s\"!",
          CResources::MeshManager::_name(meshRef).getString().c_str());
      std::remove(meshFilePath.c_str());
    }
  }
}
void Physics::createPhysicsConvexMeshes(
    const CResources::MeshRefArray& p_MeshRefs)
{
  for (MeshRef meshRef : p_MeshRefs)
  {
    // Don't even try to create empty convex meshes
    if (MeshManager::_descPositionsPerSubMesh(meshRef).empty() ||
        MeshManager::_descIndicesPerSubMesh(meshRef).empty())
      continue;

    const _INTR_STRING convexMeshFilePath =
        "media/physics_meshes/" +
        CResources::MeshManager::_name(meshRef).getString() + ".pcm";

    physx::PxConvexMeshDesc convexMeshDesc;
    convexMeshDesc.flags |= physx::PxConvexFlag::eCOMPUTE_CONVEX;
    convexMeshDesc.flags |= physx::PxConvexFlag::eINFLATE_CONVEX;

    convexMeshDesc.points.count =
        (uint32_t)MeshManager::_descPositionsPerSubMesh(meshRef)[0].size();
    convexMeshDesc.points.stride = sizeof(glm::vec3);
    convexMeshDesc.points.data =
        MeshManager::_descPositionsPerSubMesh(meshRef)[0].data();

    bool result = false;
    {
      physx::PxDefaultFileOutputStream outStream =
          physx::PxDefaultFileOutputStream(convexMeshFilePath.c_str());
      result = Core::Physics::System::_pxCooking->cookConvexMesh(convexMeshDesc,
                                                                 outStream);
    }

    if (!result)
    {
      _INTR_LOG_WARNING(
          "Failed to cook physics convex mesh for mesh \"%s\"! Trying to "
          "generate a convex hull from the AABB...",
          CResources::MeshManager::_name(meshRef).getString().c_str());
      std::remove(convexMeshFilePath.c_str());

      CResources::AABBPerSubMeshArray& aabbs =
          CResources::MeshManager::_aabbPerSubMesh(meshRef);

      Math::AABB ajdustedAABB = aabbs[0];
      ajdustedAABB.max += 0.01f;
      ajdustedAABB.min -= 0.01f;

      glm::vec3 aabbCorners[8];
      Math::calcAABBCorners(ajdustedAABB, aabbCorners);

      convexMeshDesc.points.count = 8u;
      convexMeshDesc.points.stride = sizeof(glm::vec3);
      convexMeshDesc.points.data = aabbCorners;

      convexMeshDesc.flags &= ~physx::PxConvexFlag::eINFLATE_CONVEX;

      {
        physx::PxDefaultFileOutputStream outStream =
            physx::PxDefaultFileOutputStream(convexMeshFilePath.c_str());
        result = Core::Physics::System::_pxCooking->cookConvexMesh(
            convexMeshDesc, outStream);
      }

      if (!result)
      {
        _INTR_LOG_WARNING(
            "Failed to cook physics convex mesh from AABB for mesh \"%s\"!",
            CResources::MeshManager::_name(meshRef).getString().c_str());
        std::remove(convexMeshFilePath.c_str());
      }
    }
  }
}
}
}
}
