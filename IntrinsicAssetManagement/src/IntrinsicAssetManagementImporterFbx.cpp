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

// FBX
#define FBXSDK_NAMESPACE_USING 1
#undef snprintf
#include "fbxsdk.h"

// Helper
#include "IntrinsicAssetManagementHelperFbx.h"

using namespace RResources;
using namespace CResources;

namespace Intrinsic
{
namespace AssetManagement
{
namespace Importers
{
namespace
{
FbxManager* _fbxManager = nullptr;

void stripDuplicateVertices(Dod::Ref p_MeshRef)
{
  const uint32_t subMeshCount =
      (uint32_t)MeshManager::_descIndicesPerSubMesh(p_MeshRef).size();

  _INTR_LOG_INFO("Stripping duplicate vertices for %u sub meshes...",
                 subMeshCount);

  for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
  {
    _INTR_HASH_MAP(uint32_t, uint32_t) indexMap;

    const uint32_t indexCount =
        (uint32_t)MeshManager::_descIndicesPerSubMesh(p_MeshRef)[subMeshIdx]
            .size();

    _INTR_ARRAY(glm::vec3) strippedPositions;
    strippedPositions.reserve(indexCount);

    _INTR_ARRAY(glm::vec2) strippedUv0s;
    strippedUv0s.reserve(indexCount);

    _INTR_ARRAY(glm::vec3) stripepdNormals;
    stripepdNormals.reserve(indexCount);

    _INTR_ARRAY(glm::vec3) strippedTangents;
    strippedTangents.reserve(indexCount);

    _INTR_ARRAY(glm::vec3) strippedBinormals;
    strippedBinormals.reserve(indexCount);

    _INTR_ARRAY(uint32_t) strippedIndices;
    strippedIndices.reserve(indexCount);

    _INTR_ARRAY(glm::vec4) strippedVertexColors;
    strippedVertexColors.reserve(indexCount);

    for (uint32_t idxId = 0u; idxId < indexCount; ++idxId)
    {
      const uint32_t idx =
          MeshManager::_descIndicesPerSubMesh(p_MeshRef)[subMeshIdx][idxId];

      struct Vertex
      {
        glm::vec3 position;
        glm::vec2 uv0;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 binormal;
        glm::vec4 vtxColor;
      } vtx;

      vtx.position =
          MeshManager::_descPositionsPerSubMesh(p_MeshRef)[subMeshIdx][idx];
      vtx.uv0 = MeshManager::_descUV0sPerSubMesh(p_MeshRef)[subMeshIdx][idx];
      vtx.normal =
          MeshManager::_descNormalsPerSubMesh(p_MeshRef)[subMeshIdx][idx];
      vtx.tangent =
          MeshManager::_descTangentsPerSubMesh(p_MeshRef)[subMeshIdx][idx];
      vtx.binormal =
          MeshManager::_descBinormalsPerSubMesh(p_MeshRef)[subMeshIdx][idx];
      vtx.vtxColor =
          MeshManager::_descVertexColorsPerSubMesh(p_MeshRef)[subMeshIdx][idx];

      const uint32_t vertexHash = Math::hash((const char*)&vtx, sizeof(vtx));

      auto vertexIt = indexMap.find(vertexHash);
      if (vertexIt != indexMap.end())
      {
        strippedIndices.push_back(vertexIt->second);
      }
      else
      {
        const uint32_t newIdx = (uint32_t)strippedPositions.size();
        indexMap[vertexHash] = newIdx;
        strippedIndices.push_back(newIdx);

        strippedPositions.push_back(vtx.position);
        strippedUv0s.push_back(vtx.uv0);
        stripepdNormals.push_back(vtx.normal);
        strippedTangents.push_back(vtx.tangent);
        strippedBinormals.push_back(vtx.binormal);
        strippedVertexColors.push_back(vtx.vtxColor);
      }
    }

    _INTR_LOG_INFO(
        "Stripped %u duplicate vertices from sub mesh #%u",
        (uint32_t)MeshManager::_descPositionsPerSubMesh(p_MeshRef)[subMeshIdx]
                .size() -
            (uint32_t)strippedPositions.size(),
        subMeshIdx);

    _INTR_ARRAY(uint32_t) optimizedIndices;
    optimizedIndices.resize(strippedIndices.size());

    // Cache-optimize faces
    TriangleOptimizer::optimizeFaces(
        strippedIndices.data(), (uint32_t)strippedIndices.size(),
        (uint32_t)strippedPositions.size(), optimizedIndices.data(), 32u);

    MeshManager::_descPositionsPerSubMesh(p_MeshRef)[subMeshIdx] =
        strippedPositions;
    MeshManager::_descUV0sPerSubMesh(p_MeshRef)[subMeshIdx] = strippedUv0s;
    MeshManager::_descNormalsPerSubMesh(p_MeshRef)[subMeshIdx] =
        stripepdNormals;
    MeshManager::_descTangentsPerSubMesh(p_MeshRef)[subMeshIdx] =
        strippedTangents;
    MeshManager::_descBinormalsPerSubMesh(p_MeshRef)[subMeshIdx] =
        strippedBinormals;
    MeshManager::_descVertexColorsPerSubMesh(p_MeshRef)[subMeshIdx] =
        strippedVertexColors;
    MeshManager::_descIndicesPerSubMesh(p_MeshRef)[subMeshIdx] =
        optimizedIndices;
  }
}

void importMesh(FbxMesh* p_Mesh, _INTR_ARRAY(MeshRef) & p_ImportedMeshes)
{
  const char* meshName = p_Mesh->GetNode()->GetName();
  FbxAMatrix nodeTransform = p_Mesh->GetNode()->EvaluateGlobalTransform();
  FbxAMatrix normalTransform = nodeTransform;
  normalTransform.SetT(FbxVector4(0.0, 0.0, 0.0, 0.0));
  normalTransform.SetS(FbxVector4(1.0f, 1.0f, 1.0f));

  const uint32_t badPolygonsRemoved = p_Mesh->RemoveBadPolygons();
  _INTR_LOG_INFO("Removed %u bad polygons from mesh...", badPolygonsRemoved);

  FbxMesh* triangleMesh = p_Mesh;

  if (!p_Mesh->IsTriangleMesh())
  {
    _INTR_LOG_INFO("Conversion to triangle mesh necessary...");

    FbxGeometryConverter cnvt = FbxGeometryConverter(_fbxManager);
    triangleMesh =
        (FbxMesh*)cnvt.Triangulate((FbxNodeAttribute*)triangleMesh, false);
    _INTR_ASSERT(triangleMesh);

    _INTR_LOG_INFO("Done!");
  }

  _INTR_LOG_INFO("Trying to generate tangent data...");

  if (triangleMesh->GenerateTangentsData(0, true))
  {
    _INTR_LOG_INFO("Done!");
  }
  else
  {
    _INTR_LOG_INFO("Failed...");
  }

  _INTR_LOG_INFO("Importing mesh (materials and sub meshes)...");

  // Create or update existing mesh resource
  bool newMesh = false;
  MeshRef meshRef =
      MeshManager::_getResourceByName(triangleMesh->GetNode()->GetName());

  if (!meshRef.isValid())
  {
    meshRef = MeshManager::createMesh(triangleMesh->GetNode()->GetName());
    newMesh = true;
  }
  else
  {
    MeshRefArray meshesToDestroy;
    meshesToDestroy.push_back(meshRef);

    MeshManager::destroyResources(meshesToDestroy);
  }
  MeshManager::resetToDefault(meshRef);

  PositionsPerSubMeshArray& posArray =
      MeshManager::_descPositionsPerSubMesh(meshRef);
  IndicesPerSubMeshArray& indexArray =
      MeshManager::_descIndicesPerSubMesh(meshRef);
  NormalsPerSubMeshArray& normalArray =
      MeshManager::_descNormalsPerSubMesh(meshRef);
  TangentsPerSubMeshArray& tangentArray =
      MeshManager::_descTangentsPerSubMesh(meshRef);
  BinormalsPerSubMeshArray& binormalArray =
      MeshManager::_descBinormalsPerSubMesh(meshRef);
  UVsPerSubMeshArray& uv0Array = MeshManager::_descUV0sPerSubMesh(meshRef);
  VertexColorsPerSubMeshArray& vertexColorArray =
      MeshManager::_descVertexColorsPerSubMesh(meshRef);
  MaterialNamesPerSubMeshArray& materialArray =
      MeshManager::_descMaterialNamesPerSubMesh(meshRef);
  uint32_t subMeshCount = 0u;

  FbxGeometryElementMaterial* elementMat = triangleMesh->GetElementMaterial();
  FbxGeometryElement::EMappingMode matMappingMode =
      elementMat->GetMappingMode();
  FbxLayerElementArrayTemplate<int>& matIdxs = elementMat->GetIndexArray();

  _INTR_ARRAY(uint32_t) polygonCountPerSubMesh;

  // Find the amount of sub meshes/materials
  if (matMappingMode == FbxGeometryElement::eAllSame)
  {
    subMeshCount = 1u;
    polygonCountPerSubMesh.resize(1u);
    polygonCountPerSubMesh[0] = triangleMesh->GetPolygonCount();
  }
  else if (matMappingMode == FbxGeometryElement::eByPolygon)
  {
    for (uint32_t pIdx = 0u; pIdx < (uint32_t)triangleMesh->GetPolygonCount();
         ++pIdx)
    {
      uint32_t matIdx = (uint32_t)matIdxs.GetAt(pIdx);
      if (subMeshCount < matIdx + 1u)
      {
        subMeshCount = matIdx + 1u;
        polygonCountPerSubMesh.resize(subMeshCount);
      }

      polygonCountPerSubMesh[matIdx]++;
    }
  }
  else
  {
    _INTR_LOG_ERROR("Failed to import mesh");
    return;
  }

  // Create dummy materials
  MaterialRefArray materialsToCreate;

  for (uint32_t i = 0u; i < subMeshCount; ++i)
  {
    const char* materialName = p_Mesh->GetNode()->GetMaterial(i)->GetName();
    MaterialRef materialRef = MaterialManager::_getResourceByName(materialName);

    if (!materialRef.isValid())
    {
      materialRef = MaterialManager::createMaterial(materialName);
      MaterialManager::resetToDefault(materialRef);

      materialsToCreate.push_back(materialRef);
    }
  }
  MaterialManager::createResources(materialsToCreate);

  posArray.resize(subMeshCount);
  indexArray.resize(subMeshCount);
  normalArray.resize(subMeshCount);
  tangentArray.resize(subMeshCount);
  binormalArray.resize(subMeshCount);
  uv0Array.resize(subMeshCount);
  vertexColorArray.resize(subMeshCount);
  materialArray.resize(subMeshCount);

  _INTR_LOG_INFO("Found %u different material/sub meshes", subMeshCount);

  for (uint32_t i = 0u; i < subMeshCount; ++i)
  {
    const uint32_t vertexCount = polygonCountPerSubMesh[i] * 3u;

    posArray[i].reserve(vertexCount);
    indexArray[i].reserve(vertexCount);
    normalArray[i].reserve(vertexCount);
    tangentArray[i].reserve(vertexCount);
    binormalArray[i].reserve(vertexCount);
    uv0Array[i].reserve(vertexCount);
    vertexColorArray[i].reserve(vertexCount);

    const char* materialName = p_Mesh->GetNode()->GetMaterial(i)->GetName();
    materialArray[i] = materialName;

    _INTR_LOG_INFO("Total vertex count for sub mesh #%u: %u", i, vertexCount);
  }

  const bool hasUv0 = triangleMesh->GetElementUVCount() > 0u;
  const bool hasNormals = triangleMesh->GetElementNormalCount() > 0u;
  const bool hasTangents = triangleMesh->GetElementTangentCount() > 0u;
  const bool hasBinormals = triangleMesh->GetElementBinormalCount() > 0u;
  const bool hasVertexColor = triangleMesh->GetElementVertexColorCount() > 0u;

  uint32_t vtxId = 0u;
  for (uint32_t pIdx = 0u; pIdx < (uint32_t)triangleMesh->GetPolygonCount();
       ++pIdx)
  {
    const uint32_t polySize = triangleMesh->GetPolygonSize(pIdx);
    const uint32_t matIdx = matIdxs.GetAt(pIdx);

    for (uint32_t vIdx = 0u; vIdx < 3u; ++vIdx, ++vtxId)
    {
      const int pVertex = triangleMesh->GetPolygonVertex(pIdx, vIdx);

      // Position
      {
        FbxVector4 pos =
            nodeTransform.MultT(triangleMesh->GetControlPointAt(pVertex));
        posArray[matIdx].push_back(
            glm::vec3((float)pos[0], (float)pos[1], (float)pos[2]));
      }

      // UV0
      if (hasUv0)
      {
        FbxVector2 uv0 = HelperFbx::getElement(triangleMesh->GetElementUV(0),
                                               triangleMesh, pIdx, vIdx, vtxId);
        uv0Array[matIdx].push_back(glm::vec2((float)uv0[0], (float)uv0[1]));
      }
      else
      {
        uv0Array[matIdx].push_back(glm::vec2(0.0f, 0.0f));
      }

      // Normals
      if (hasNormals)
      {
        FbxVector4 normal = HelperFbx::getElement(
            triangleMesh->GetElementNormal(), triangleMesh, pIdx, vIdx, vtxId);

        FbxVector4 finalNormal = normal;
        finalNormal = normalTransform.MultT(finalNormal);

        normalArray[matIdx].push_back(glm::vec3((float)finalNormal[0],
                                                (float)finalNormal[1],
                                                (float)finalNormal[2]));
      }
      else
      {
        normalArray[matIdx].push_back(glm::vec3(0.0f, 1.0f, 0.0f));
      }

      // Tangents
      if (hasTangents)
      {
        FbxVector4 tangent = HelperFbx::getElement(
            triangleMesh->GetElementTangent(), triangleMesh, pIdx, vIdx, vtxId);

        FbxVector4 finalTangent = tangent;
        finalTangent = normalTransform.MultT(finalTangent);

        tangentArray[matIdx].push_back(glm::vec3((float)finalTangent[0],
                                                 (float)finalTangent[1],
                                                 (float)finalTangent[2]));
      }
      else
      {
        tangentArray[matIdx].push_back(glm::vec3(0.0f, 0.0f, 1.0f));
      }

      // Binormals
      if (hasBinormals)
      {
        FbxVector4 binormal =
            HelperFbx::getElement(triangleMesh->GetElementBinormal(),
                                  triangleMesh, pIdx, vIdx, vtxId);

        FbxVector4 finalBinormal = binormal;
        finalBinormal = normalTransform.MultT(finalBinormal);

        binormalArray[matIdx].push_back(glm::vec3((float)finalBinormal[0],
                                                  (float)finalBinormal[1],
                                                  (float)finalBinormal[2]));
      }
      else
      {
        binormalArray[matIdx].push_back(glm::vec3(1.0f, 0.0f, 0.0f));
      }

      // Color
      if (hasVertexColor)
      {
        FbxColor color =
            HelperFbx::getElement(triangleMesh->GetElementVertexColor(),
                                  triangleMesh, pIdx, vIdx, vtxId);
        vertexColorArray[matIdx].push_back(
            glm::vec4((float)color[0], (float)color[1], (float)color[2],
                      (float)color[3]));
      }
      else
      {
        vertexColorArray[matIdx].push_back(glm::vec4(1.0f));
      }

      // Index
      {
        indexArray[matIdx].push_back((uint32_t)posArray[matIdx].size() - 1u);
      }
    }
  }

  stripDuplicateVertices(meshRef);
  p_ImportedMeshes.push_back(meshRef);
}

// <-

_INTR_INLINE void importMeshesFromNode(FbxNode* p_Node,
                                       _INTR_ARRAY(MeshRef) & p_ImportedMeshes)
{
  _INTR_ASSERT(p_Node);

  bool triangleMeshFound = false;
  bool meshFound = false;

  for (uint32_t i = 0u; i < (uint32_t)p_Node->GetNodeAttributeCount(); ++i)
  {
    FbxNodeAttribute* nodeAttribute = p_Node->GetNodeAttributeByIndex(i);

    if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
    {
      const char* meshName = p_Node->GetName();
      FbxMesh* mesh = (FbxMesh*)nodeAttribute;

      if (mesh->IsTriangleMesh())
      {
        _INTR_LOG_INFO("Triangle mesh found in node '%s'! Importing...",
                       meshName);

        importMesh((FbxMesh*)nodeAttribute, p_ImportedMeshes);
        triangleMeshFound = true;
      }

      meshFound = true;
    }
  }

  if (!triangleMeshFound && meshFound)
  {
    _INTR_LOG_WARNING("No triangle mesh found in node attributes, trying to "
                      "import anyhow...");

    for (uint32_t i = 0u; i < (uint32_t)p_Node->GetNodeAttributeCount(); ++i)
    {
      FbxNodeAttribute* nodeAttribute = p_Node->GetNodeAttributeByIndex(i);

      if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
      {
        const char* meshName = p_Node->GetName();
        _INTR_LOG_INFO("Found mesh in node '%s'", meshName);

        FbxMesh* mesh = (FbxMesh*)nodeAttribute;

        importMesh((FbxMesh*)nodeAttribute, p_ImportedMeshes);
        break;
      }
    }
  }

  for (uint32_t i = 0u; i < (uint32_t)p_Node->GetChildCount(); ++i)
  {
    importMeshesFromNode(p_Node->GetChild(i), p_ImportedMeshes);
  }
}
}

void Fbx::init()
{
  _INTR_ASSERT(_fbxManager == nullptr);
  _fbxManager = FbxManager::Create();
  FbxIOSettings* ioSettings = FbxIOSettings::Create(_fbxManager, IOSROOT);
  _fbxManager->SetIOSettings(ioSettings);
}

// <-

void Fbx::destroy()
{
  _INTR_ASSERT(_fbxManager);

  _fbxManager->Destroy();
  _fbxManager = nullptr;
}

// <-

bool Fbx::importMeshesFromFile(const _INTR_STRING& p_FilePath,
                               _INTR_ARRAY(MeshRef) & p_ImportedMeshes)
{
  FbxImporter* importer = FbxImporter::Create(_fbxManager, "");

  if (!importer->Initialize(p_FilePath.c_str(), -1,
                            _fbxManager->GetIOSettings()))
  {
    return false;
  }

  FbxScene* scene = FbxScene::Create(_fbxManager, "Scene");
  {
    importer->Import(scene);
    importer->Destroy();
  }

  FbxNode* rootNode = scene->GetRootNode();
  if (rootNode == nullptr)
  {
    return false;
  }

  _INTR_LOG_INFO("Importing meshes from nodes...");
  _INTR_LOG_PUSH();

  importMeshesFromNode(rootNode, p_ImportedMeshes);

  _INTR_LOG_POP();

  return true;
}
}
}
}
