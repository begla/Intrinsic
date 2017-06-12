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

// Forward decls.
namespace physx
{
class PxTriangleMesh;
}

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
typedef Dod::Ref MeshRef;
typedef _INTR_ARRAY(MeshRef) MeshRefArray;

typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec3)) PositionsPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec2)) UVsPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(uint32_t)) IndicesPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec3)) NormalsPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec3)) TangentsPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec3)) BinormalsPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(glm::vec4)) VertexColorsPerSubMeshArray;
typedef _INTR_ARRAY(Name) MaterialNamesPerSubMeshArray;
typedef _INTR_ARRAY(_INTR_ARRAY(Dod::Ref)) VertexBuffersPerSubMeshArray;
typedef _INTR_ARRAY(Dod::Ref) IndexBufferPerSubMeshArray;
typedef _INTR_ARRAY(Math::AABB) AABBPerSubMeshArray;

struct MeshData : Dod::Resources::ResourceDataBase
{
  MeshData() : Dod::Resources::ResourceDataBase(_INTR_MAX_MESH_COUNT)
  {
    descPositionsPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descUV0sPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descIndicesPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descNormalsPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descTangentsPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descBinormalsPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descVertexColorsPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    descMaterialNamesPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    vertexBuffersPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    indexBufferPerSubMesh.resize(_INTR_MAX_MESH_COUNT);
    aabbPerSubMesh.resize(_INTR_MAX_MESH_COUNT);

    pxTriangleMesh.resize(_INTR_MAX_MESH_COUNT);
  }

  // <-

  // Description
  _INTR_ARRAY(PositionsPerSubMeshArray) descPositionsPerSubMesh;
  _INTR_ARRAY(UVsPerSubMeshArray) descUV0sPerSubMesh;
  _INTR_ARRAY(IndicesPerSubMeshArray) descIndicesPerSubMesh;
  _INTR_ARRAY(NormalsPerSubMeshArray) descNormalsPerSubMesh;
  _INTR_ARRAY(TangentsPerSubMeshArray) descTangentsPerSubMesh;
  _INTR_ARRAY(BinormalsPerSubMeshArray) descBinormalsPerSubMesh;
  _INTR_ARRAY(VertexColorsPerSubMeshArray) descVertexColorsPerSubMesh;
  _INTR_ARRAY(MaterialNamesPerSubMeshArray) descMaterialNamesPerSubMesh;

  // Resources
  _INTR_ARRAY(VertexBuffersPerSubMeshArray) vertexBuffersPerSubMesh;
  _INTR_ARRAY(IndexBufferPerSubMeshArray) indexBufferPerSubMesh;
  _INTR_ARRAY(AABBPerSubMeshArray) aabbPerSubMesh;

  _INTR_ARRAY(physx::PxTriangleMesh*) pxTriangleMesh;
};

struct MeshManager
    : Dod::Resources::ResourceManagerBase<MeshData, _INTR_MAX_MESH_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static MeshRef createMesh(const Name& p_Name)
  {
    MeshRef ref = Dod::Resources::ResourceManagerBase<
        MeshData, _INTR_MAX_MESH_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
    _descPositionsPerSubMesh(p_Ref).clear();
    _descUV0sPerSubMesh(p_Ref).clear();
    _descIndicesPerSubMesh(p_Ref).clear();
    _descNormalsPerSubMesh(p_Ref).clear();
    _descTangentsPerSubMesh(p_Ref).clear();
    _descBinormalsPerSubMesh(p_Ref).clear();
    _descVertexColorsPerSubMesh(p_Ref).clear();
    _descMaterialNamesPerSubMesh(p_Ref).clear();
    _aabbPerSubMesh(p_Ref).clear();
  }

  // <-

  _INTR_INLINE static void destroyMesh(MeshRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        MeshData, _INTR_MAX_MESH_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(MeshRef p_Ref, bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        MeshData, _INTR_MAX_MESH_COUNT>::_compileDescriptor(p_Ref,
                                                            p_GenerateDesc,
                                                            p_Properties,
                                                            p_Document);
    if (!p_GenerateDesc)
    {
      rapidjson::Value positionsPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value uv0sPerSubMesh = rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value normalPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value tangentsPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value binormalsPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value vertexColorsPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value indicesPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);
      rapidjson::Value materialNamesPerSubMesh =
          rapidjson::Value(rapidjson::kArrayType);

      for (uint32_t subMeshIdx = 0u;
           subMeshIdx < _descPositionsPerSubMesh(p_Ref).size(); ++subMeshIdx)
      {
        rapidjson::Value positions = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value uv0s = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value normals = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value tangents = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value binormals = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value vertexColors = rapidjson::Value(rapidjson::kArrayType);
        rapidjson::Value indices = rapidjson::Value(rapidjson::kArrayType);

        for (uint32_t vtxIdx = 0u;
             vtxIdx < _descPositionsPerSubMesh(p_Ref)[subMeshIdx].size();
             ++vtxIdx)
        {
          const glm::vec3 position =
              _descPositionsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          positions.PushBack(JsonHelper::createVec(p_Document, position),
                             p_Document.GetAllocator());

          const glm::vec2 uv0 = _descUV0sPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          uv0s.PushBack(JsonHelper::createVec(p_Document, uv0),
                        p_Document.GetAllocator());

          const glm::vec3 normal =
              _descNormalsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          normals.PushBack(JsonHelper::createVec(p_Document, normal),
                           p_Document.GetAllocator());

          const glm::vec3 tangent =
              _descTangentsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          tangents.PushBack(JsonHelper::createVec(p_Document, tangent),
                            p_Document.GetAllocator());

          const glm::vec3 binormal =
              _descBinormalsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          binormals.PushBack(JsonHelper::createVec(p_Document, binormal),
                             p_Document.GetAllocator());

          const glm::vec4 vtxColor =
              _descVertexColorsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx];
          vertexColors.PushBack(JsonHelper::createVec(p_Document, vtxColor),
                                p_Document.GetAllocator());
        }

        for (uint32_t i = 0u;
             i < _descIndicesPerSubMesh(p_Ref)[subMeshIdx].size(); ++i)
        {
          const uint32_t idx = _descIndicesPerSubMesh(p_Ref)[subMeshIdx][i];
          indices.PushBack(idx, p_Document.GetAllocator());
        }

        positionsPerSubMesh.PushBack(positions, p_Document.GetAllocator());
        uv0sPerSubMesh.PushBack(uv0s, p_Document.GetAllocator());
        normalPerSubMesh.PushBack(normals, p_Document.GetAllocator());
        tangentsPerSubMesh.PushBack(tangents, p_Document.GetAllocator());
        binormalsPerSubMesh.PushBack(binormals, p_Document.GetAllocator());
        vertexColorsPerSubMesh.PushBack(vertexColors,
                                        p_Document.GetAllocator());
        indicesPerSubMesh.PushBack(indices, p_Document.GetAllocator());

        rapidjson::Value materialName = rapidjson::Value(
            _descMaterialNamesPerSubMesh(p_Ref)[subMeshIdx].getString().c_str(),
            p_Document.GetAllocator());
        materialNamesPerSubMesh.PushBack(materialName,
                                         p_Document.GetAllocator());
      }

      p_Properties.AddMember("positionsPerSubMesh", positionsPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("uv0sPerSubMesh", uv0sPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("normalPerSubMesh", normalPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("tangentsPerSubMesh", tangentsPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("binormalsPerSubMesh", binormalsPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("vertexColorsPerSubMesh", vertexColorsPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("indicesPerSubMesh", indicesPerSubMesh,
                             p_Document.GetAllocator());
      p_Properties.AddMember("materialNamesPerSubMesh", materialNamesPerSubMesh,
                             p_Document.GetAllocator());
    }
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(MeshRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        MeshData, _INTR_MAX_MESH_COUNT>::_initFromDescriptor(p_Ref,
                                                             p_Properties);

    rapidjson::Value& positionsPerSubMesh = p_Properties["positionsPerSubMesh"];
    rapidjson::Value& uv0sPerSubMesh = p_Properties["uv0sPerSubMesh"];
    rapidjson::Value& normalPerSubMesh = p_Properties["normalPerSubMesh"];
    rapidjson::Value& tangentsPerSubMesh = p_Properties["tangentsPerSubMesh"];
    rapidjson::Value& binormalsPerSubMesh = p_Properties["binormalsPerSubMesh"];
    rapidjson::Value& vertexColorsPerSubMesh =
        p_Properties["vertexColorsPerSubMesh"];
    rapidjson::Value& indicesPerSubMesh = p_Properties["indicesPerSubMesh"];
    rapidjson::Value& materialNamesPerSubMesh =
        p_Properties["materialNamesPerSubMesh"];

    const uint32_t subMeshCount = positionsPerSubMesh.Size();
    _descPositionsPerSubMesh(p_Ref).resize(subMeshCount);
    _descUV0sPerSubMesh(p_Ref).resize(subMeshCount);
    _descNormalsPerSubMesh(p_Ref).resize(subMeshCount);
    _descTangentsPerSubMesh(p_Ref).resize(subMeshCount);
    _descBinormalsPerSubMesh(p_Ref).resize(subMeshCount);
    _descVertexColorsPerSubMesh(p_Ref).resize(subMeshCount);
    _descIndicesPerSubMesh(p_Ref).resize(subMeshCount);
    _descMaterialNamesPerSubMesh(p_Ref).resize(subMeshCount);

    for (uint32_t subMeshIdx = 0u; subMeshIdx < positionsPerSubMesh.Size();
         ++subMeshIdx)
    {
      rapidjson::Value& positions = positionsPerSubMesh[subMeshIdx];
      _descPositionsPerSubMesh(p_Ref)[subMeshIdx].resize(positions.Size());

      rapidjson::Value& uv0s = uv0sPerSubMesh[subMeshIdx];
      _descUV0sPerSubMesh(p_Ref)[subMeshIdx].resize(uv0s.Size());

      rapidjson::Value& normals = normalPerSubMesh[subMeshIdx];
      _descNormalsPerSubMesh(p_Ref)[subMeshIdx].resize(normals.Size());

      rapidjson::Value& tangents = tangentsPerSubMesh[subMeshIdx];
      _descTangentsPerSubMesh(p_Ref)[subMeshIdx].resize(tangents.Size());

      rapidjson::Value& binormals = binormalsPerSubMesh[subMeshIdx];
      _descBinormalsPerSubMesh(p_Ref)[subMeshIdx].resize(binormals.Size());

      rapidjson::Value& vertexColors = vertexColorsPerSubMesh[subMeshIdx];
      _descVertexColorsPerSubMesh(p_Ref)[subMeshIdx].resize(
          vertexColors.Size());

      rapidjson::Value& indices = indicesPerSubMesh[subMeshIdx];
      _descIndicesPerSubMesh(p_Ref)[subMeshIdx].resize(indices.Size());

      for (uint32_t vtxIdx = 0u; vtxIdx < positions.Size(); ++vtxIdx)
      {
        _descPositionsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec3(positions[vtxIdx]);
        _descUV0sPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec2(uv0s[vtxIdx]);
        _descNormalsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec3(normals[vtxIdx]);
        _descTangentsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec3(tangents[vtxIdx]);
        _descBinormalsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec3(binormals[vtxIdx]);
        _descVertexColorsPerSubMesh(p_Ref)[subMeshIdx][vtxIdx] =
            JsonHelper::readVec4(vertexColors[vtxIdx]);
      }

      for (uint32_t i = 0u; i < indices.Size(); ++i)
      {
        _descIndicesPerSubMesh(p_Ref)[subMeshIdx][i] = indices[i].GetUint();
      }
    }

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      _descMaterialNamesPerSubMesh(p_Ref)[subMeshIdx] =
          materialNamesPerSubMesh[subMeshIdx].GetString();
    }
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<MeshData, _INTR_MAX_MESH_COUNT>::
        _saveToMultipleFiles<rapidjson::Writer<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void
  saveToMultipleFilesSingleResource(MeshRef p_Ref, const char* p_Path,
                                    const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<MeshData, _INTR_MAX_MESH_COUNT>::
        _saveToMultipleFilesSingleResource<
            rapidjson::Writer<rapidjson::FileWriteStream>>(
            p_Ref, p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<MeshData, _INTR_MAX_MESH_COUNT>::
        _loadFromMultipleFiles(p_Path, p_Extension, initFromDescriptor,
                               resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  static void createResources(const MeshRefArray& p_Meshes);

  // <-

  static void destroyResources(const MeshRefArray& p_Meshes);

  // Description
  _INTR_INLINE static PositionsPerSubMeshArray&
  _descPositionsPerSubMesh(MeshRef p_Ref)
  {
    return _data.descPositionsPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static UVsPerSubMeshArray& _descUV0sPerSubMesh(MeshRef p_Ref)
  {
    return _data.descUV0sPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static IndicesPerSubMeshArray&
  _descIndicesPerSubMesh(MeshRef p_Ref)
  {
    return _data.descIndicesPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static NormalsPerSubMeshArray&
  _descNormalsPerSubMesh(MeshRef p_Ref)
  {
    return _data.descNormalsPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static TangentsPerSubMeshArray&
  _descTangentsPerSubMesh(MeshRef p_Ref)
  {
    return _data.descTangentsPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static TangentsPerSubMeshArray&
  _descBinormalsPerSubMesh(MeshRef p_Ref)
  {
    return _data.descBinormalsPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static VertexColorsPerSubMeshArray&
  _descVertexColorsPerSubMesh(MeshRef p_Ref)
  {
    return _data.descVertexColorsPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static MaterialNamesPerSubMeshArray&
  _descMaterialNamesPerSubMesh(MeshRef p_Ref)
  {
    return _data.descMaterialNamesPerSubMesh[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static VertexBuffersPerSubMeshArray&
  _vertexBuffersPerSubMesh(MeshRef p_Ref)
  {
    return _data.vertexBuffersPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static IndexBufferPerSubMeshArray&
  _indexBufferPerSubMesh(MeshRef p_Ref)
  {
    return _data.indexBufferPerSubMesh[p_Ref._id];
  }
  _INTR_INLINE static AABBPerSubMeshArray& _aabbPerSubMesh(MeshRef p_Ref)
  {
    return _data.aabbPerSubMesh[p_Ref._id];
  }

  _INTR_INLINE static physx::PxTriangleMesh*& _pxTriangleMesh(MeshRef p_Ref)
  {
    return _data.pxTriangleMesh[p_Ref._id];
  }
};
}
}
}
