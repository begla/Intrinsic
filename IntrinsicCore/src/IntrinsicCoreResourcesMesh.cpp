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

// Precompiled header file
#include "stdafx.h"

// PhysX includes
#include "PxPhysics.h"
#include "PxScene.h"
#include "cooking/PxCooking.h"
#include "common/PxTolerancesScale.h"

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
namespace
{
_INTR_INLINE void createPxTriangleMesh(MeshRef p_MeshRef)
{
  physx::PxCookingParams params(
      Physics::System::_pxPhysics->getTolerancesScale());
  params.meshPreprocessParams |=
      physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
  params.meshPreprocessParams |=
      physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
  params.meshCookingHint = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;

  Physics::System::_pxCooking->setParams(params);

  physx::PxTriangleMeshDesc meshDesc;
  meshDesc.points.count =
      (uint32_t)MeshManager::_descPositionsPerSubMesh(p_MeshRef)[0].size();
  meshDesc.points.stride = sizeof(glm::vec3);
  meshDesc.points.data =
      MeshManager::_descPositionsPerSubMesh(p_MeshRef)[0].data();

  meshDesc.triangles.count =
      (uint32_t)MeshManager::_descIndicesPerSubMesh(p_MeshRef)[0].size() / 3u;
  meshDesc.triangles.stride = 3u * sizeof(uint32_t);
  meshDesc.triangles.data =
      MeshManager::_descIndicesPerSubMesh(p_MeshRef)[0].data();

  //#ifdef _DEBUG
  //        const bool res =
  //        Physics::System::_pxCooking->validateTriangleMesh(meshDesc);
  //        _INTR_ASSERT(res);
  //#endif // _DEBUG

  MeshManager::_pxTriangleMesh(p_MeshRef) =
      Physics::System::_pxCooking->createTriangleMesh(
          meshDesc, Physics::System::_pxPhysics->getPhysicsInsertionCallback());
  _INTR_ASSERT(MeshManager::_pxTriangleMesh(p_MeshRef));
}
}

void MeshManager::init()
{
  _INTR_LOG_INFO("Inititializing Mesh Manager...");

  Dod::Resources::ResourceManagerBase<
      MeshData, _INTR_MAX_MESH_COUNT>::_initResourceManager();

  Dod::Resources::ResourceManagerEntry managerEntry;
  {
    managerEntry.createFunction = Resources::MeshManager::createMesh;
    managerEntry.destroyFunction = Resources::MeshManager::destroyMesh;
    managerEntry.createResourcesFunction =
        Resources::MeshManager::createResources;
    managerEntry.destroyResourcesFunction =
        Resources::MeshManager::destroyResources;
    managerEntry.getActiveResourceAtIndexFunction =
        Resources::MeshManager::getActiveResourceAtIndex;
    managerEntry.getActiveResourceCountFunction =
        Resources::MeshManager::getActiveResourceCount;
    managerEntry.loadFromMultipleFilesFunction =
        Resources::MeshManager::loadFromMultipleFiles;
    managerEntry.saveToMultipleFilesFunction =
        Resources::MeshManager::saveToMultipleFiles;

    Application::_resourceManagerMapping[_N(Mesh)] = managerEntry;
  }

  Dod::PropertyCompilerEntry propertyEntry;
  {
    propertyEntry.compileFunction = Resources::MeshManager::compileDescriptor;
    propertyEntry.initFunction = Resources::MeshManager::initFromDescriptor;
    propertyEntry.ref = Dod::Ref();

    Application::_resourcePropertyCompilerMapping[_N(Mesh)] = propertyEntry;
  }

  _defaultResourceName = _N(cube);
}

// <-

void MeshManager::createResources(const MeshRefArray& p_Meshes)
{
  // Create vertex/index buffers - we're using a separate buffer for each vertex
  // attribute
  Renderer::Vulkan::Resources::BufferRefArray buffersToCreate;
  _INTR_ARRAY(void*) tempBuffersToRelease;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshRef = p_Meshes[meshIdx];
    const PositionsPerSubMeshArray& positions =
        _descPositionsPerSubMesh(meshRef);
    const UVsPerSubMeshArray& uv0s = _descUV0sPerSubMesh(meshRef);
    const IndicesPerSubMeshArray& indices = _descIndicesPerSubMesh(meshRef);
    const NormalsPerSubMeshArray& normals = _descNormalsPerSubMesh(meshRef);
    const TangentsPerSubMeshArray& tangents = _descTangentsPerSubMesh(meshRef);
    const BinormalsPerSubMeshArray& binormals =
        _descBinormalsPerSubMesh(meshRef);
    const VertexColorsPerSubMeshArray& vtxColors =
        _descVertexColorsPerSubMesh(meshRef);
    VertexBuffersPerSubMeshArray& vertexBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& indexBuffers = _indexBufferPerSubMesh(meshRef);

    void* tempIndexBuffer = nullptr;
    const uint32_t subMeshCount = (uint32_t)positions.size();
    vertexBuffers.resize(subMeshCount);
    indexBuffers.resize(subMeshCount);
    _aabbPerSubMesh(meshRef).resize(subMeshCount);

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      // Build AABB
      {
        Math::AABB& aabb = _aabbPerSubMesh(meshRef)[subMeshIdx];
        Math::initAABB(aabb);

        for (uint32_t posIdx = 0u; posIdx < positions[subMeshIdx].size();
             ++posIdx)
        {
          Math::mergePointToAABB(aabb, positions[subMeshIdx][posIdx]);
        }
      }

      Renderer::Vulkan::Resources::BufferRef posVertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshPositionVb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            posVertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            posVertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            posVertexBuffer) = Renderer::Vulkan::BufferType::kVertex;
        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            posVertexBuffer) =
            (uint32_t)positions[subMeshIdx].size() * sizeof(uint16_t) * 3u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                posVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < positions[subMeshIdx].size(); ++i)
        {
          uint32_t packedPosition0 = glm::packHalf2x16(glm::vec2(
              positions[subMeshIdx][i].x, positions[subMeshIdx][i].y));
          uint32_t packedPosition1 =
              glm::packHalf2x16(glm::vec2(positions[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedPosition0;
          tempBuffer[i * 3u + 1u] = packedPosition0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedPosition1;
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            posVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(posVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(posVertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef uv0VertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshUv0Vb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            uv0VertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            uv0VertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            uv0VertexBuffer) = Renderer::Vulkan::BufferType::kVertex;
        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            uv0VertexBuffer) =
            (uint32_t)uv0s[subMeshIdx].size() * sizeof(uint16_t) * 2u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                uv0VertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < uv0s[subMeshIdx].size(); ++i)
        {
          uint32_t packedUv = glm::packHalf2x16(uv0s[subMeshIdx][i]);

          tempBuffer[i * 2u] = packedUv;
          tempBuffer[i * 2u + 1u] = packedUv >> 16u;
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            uv0VertexBuffer) = tempBuffer;

        buffersToCreate.push_back(uv0VertexBuffer);
        vertexBuffers[subMeshIdx].push_back(uv0VertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef normalVertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshNormalVb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            normalVertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            normalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            normalVertexBuffer) = Renderer::Vulkan::BufferType::kVertex;
        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            normalVertexBuffer) =
            (uint32_t)normals[subMeshIdx].size() * sizeof(uint16_t) * 3u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                normalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < normals[subMeshIdx].size(); ++i)
        {
          uint32_t packedNormal0 = glm::packHalf2x16(
              glm::vec2(normals[subMeshIdx][i].x, normals[subMeshIdx][i].y));
          uint32_t packedNormal1 =
              glm::packHalf2x16(glm::vec2(normals[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedNormal0;
          tempBuffer[i * 3u + 1u] = packedNormal0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedNormal1;
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            normalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(normalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(normalVertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef tangentVertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshTangentVb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            tangentVertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            tangentVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            tangentVertexBuffer) = Renderer::Vulkan::BufferType::kVertex;
        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            tangentVertexBuffer) =
            (uint32_t)tangents[subMeshIdx].size() * sizeof(uint16_t) * 3u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                tangentVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < tangents[subMeshIdx].size(); ++i)
        {
          uint32_t packedTangent0 = glm::packHalf2x16(
              glm::vec2(tangents[subMeshIdx][i].x, tangents[subMeshIdx][i].y));
          uint32_t packedTangent1 =
              glm::packHalf2x16(glm::vec2(tangents[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedTangent0;
          tempBuffer[i * 3u + 1u] = packedTangent0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedTangent1;
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            tangentVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(tangentVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(tangentVertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef binormalVertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshBinormalVb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            binormalVertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            binormalVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            binormalVertexBuffer) = Renderer::Vulkan::BufferType::kVertex;
        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            binormalVertexBuffer) =
            (uint32_t)binormals[subMeshIdx].size() * sizeof(uint16_t) * 3u;

        // Convert to half
        uint16_t* tempBuffer = (uint16_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                binormalVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < binormals[subMeshIdx].size(); ++i)
        {
          uint32_t packedBinormal0 = glm::packHalf2x16(glm::vec2(
              binormals[subMeshIdx][i].x, binormals[subMeshIdx][i].y));
          uint32_t packedBinormal1 =
              glm::packHalf2x16(glm::vec2(binormals[subMeshIdx][i].z, 0.0f));

          tempBuffer[i * 3u] = packedBinormal0;
          tempBuffer[i * 3u + 1u] = packedBinormal0 >> 16u;
          tempBuffer[i * 3u + 2u] = packedBinormal1;
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            binormalVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(binormalVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(binormalVertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef vtxColorVertexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(
              _N(MeshVtxColorVb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(
            vtxColorVertexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            vtxColorVertexBuffer,
            Dod::Resources::ResourceFlags::kResourceVolatile);
        Renderer::Vulkan::Resources::BufferManager::_descBufferType(
            vtxColorVertexBuffer) = Renderer::Vulkan::BufferType::kVertex;

        Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
            vtxColorVertexBuffer) =
            (uint32_t)vtxColors[subMeshIdx].size() * sizeof(uint32_t);

        // Convert color
        uint32_t* tempBuffer = (uint32_t*)Tlsf::MainAllocator::allocate(
            Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
                vtxColorVertexBuffer));
        tempBuffersToRelease.push_back(tempBuffer);

        for (uint32_t i = 0u; i < vtxColors[subMeshIdx].size(); ++i)
        {
          tempBuffer[i] = Math::convertColorToBGRA(vtxColors[subMeshIdx][i]);
        }
        Renderer::Vulkan::Resources::BufferManager::_descInitialData(
            vtxColorVertexBuffer) = tempBuffer;

        buffersToCreate.push_back(vtxColorVertexBuffer);
        vertexBuffers[subMeshIdx].push_back(vtxColorVertexBuffer);
      }

      Renderer::Vulkan::Resources::BufferRef indexBuffer =
          Renderer::Vulkan::Resources::BufferManager::createBuffer(_N(MeshIb));
      {
        Renderer::Vulkan::Resources::BufferManager::resetToDefault(indexBuffer);

        Renderer::Vulkan::Resources::BufferManager::addResourceFlags(
            indexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);

        if (indices[subMeshIdx].size() <= 0xFFFF)
        {
          uint32_t indexBufferSizeInBytes =
              (uint16_t)indices[subMeshIdx].size() * sizeof(uint16_t);
          uint16_t* tempIndexBuffer =
              (uint16_t*)Tlsf::MainAllocator::allocate(indexBufferSizeInBytes);
          tempBuffersToRelease.push_back(tempIndexBuffer);

          for (uint32_t i = 0u; i < indices[subMeshIdx].size(); ++i)
          {
            tempIndexBuffer[i] = (uint16_t)indices[subMeshIdx][i];
          }

          Renderer::Vulkan::Resources::BufferManager::_descBufferType(
              indexBuffer) = Renderer::Vulkan::BufferType::kIndex16;
          Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
              indexBuffer) = indexBufferSizeInBytes;
          Renderer::Vulkan::Resources::BufferManager::_descInitialData(
              indexBuffer) = tempIndexBuffer;
        }
        else
        {
          Renderer::Vulkan::Resources::BufferManager::_descBufferType(
              indexBuffer) = Renderer::Vulkan::BufferType::kIndex32;
          Renderer::Vulkan::Resources::BufferManager::_descSizeInBytes(
              indexBuffer) =
              (uint32_t)indices[subMeshIdx].size() * sizeof(uint32_t);
          Renderer::Vulkan::Resources::BufferManager::_descInitialData(
              indexBuffer) = (void*)indices[subMeshIdx].data();
        }

        buffersToCreate.push_back(indexBuffer);
        indexBuffers[subMeshIdx] = indexBuffer;
      }
    }

    createPxTriangleMesh(meshRef);
  }

  Renderer::Vulkan::Resources::BufferManager::createResources(buffersToCreate);

  for (uint32_t i = 0u; i < tempBuffersToRelease.size(); ++i)
  {
    Tlsf::MainAllocator::free(tempBuffersToRelease[i]);
  }
  tempBuffersToRelease.clear();
}

// <-

void MeshManager::destroyResources(const MeshRefArray& p_Meshes)
{
  Renderer::Vulkan::Resources::BufferRefArray buffersToDestroy;

  for (uint32_t i = 0u; i < p_Meshes.size(); ++i)
  {
    MeshRef meshRef = p_Meshes[i];

    VertexBuffersPerSubMeshArray& subMeshVtxBuffers =
        _vertexBuffersPerSubMesh(meshRef);
    IndexBufferPerSubMeshArray& subMeshIdxBuffers =
        _indexBufferPerSubMesh(meshRef);

    for (uint32_t i = 0u; i < subMeshVtxBuffers.size(); ++i)
    {
      for (uint32_t j = 0u; j < subMeshVtxBuffers[i].size(); ++j)
      {
        Renderer::Vulkan::Resources::BufferRef bufferRef =
            subMeshVtxBuffers[i][j];
        _INTR_ASSERT(bufferRef.isValid());

        buffersToDestroy.push_back(bufferRef);
      }
    }

    for (uint32_t i = 0u; i < subMeshIdxBuffers.size(); ++i)
    {
      Renderer::Vulkan::Resources::BufferRef bufferRef = subMeshIdxBuffers[i];
      _INTR_ASSERT(bufferRef.isValid());

      buffersToDestroy.push_back(bufferRef);
    }

    _vertexBuffersPerSubMesh(meshRef).clear();
    _indexBufferPerSubMesh(meshRef).clear();
  }

  Renderer::Vulkan::Resources::BufferManager::destroyResources(
      buffersToDestroy);

  for (uint32_t i = 0u; i < buffersToDestroy.size(); ++i)
  {
    Renderer::Vulkan::Resources::BufferManager::destroyBuffer(
        buffersToDestroy[i]);
  }
}
}
}
}
