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
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace TriangleOptimizer
{
// based on
// https://github.com/bkaradzic/bgfx/blob/master/3rdparty/forsyth-too/forsythtriangleorderoptimizer.cpp
void optimizeFaces(const uint32_t* indexList, uint32_t indexCount,
                   uint32_t vertexCount, uint32_t* newIndexList,
                   uint32_t lruCacheSize);

namespace
{
// code for computing vertex score was taken, as much as possible
// directly from the original publication.
float computeVertexCacheScore(int cachePosition, int vertexCacheSize)
{
  const float FindVertexScore_CacheDecayPower = 1.5f;
  const float FindVertexScore_LastTriScore = 0.75f;

  float score = 0.0f;
  if (cachePosition < 0)
  {
    // Vertex is not in FIFO cache - no score.
  }
  else
  {
    if (cachePosition < 3)
    {
      // This vertex was used in the last triangle,
      // so it has a fixed score, whichever of the three
      // it's in. Otherwise, you can get very different
      // answers depending on whether you add
      // the triangle 1,2,3 or 3,1,2 - which is silly.
      score = FindVertexScore_LastTriScore;
    }
    else
    {
      _INTR_ASSERT(cachePosition < vertexCacheSize);
      // Points for being high in the cache.
      const float scaler = 1.0f / (vertexCacheSize - 3);
      score = 1.0f - (cachePosition - 3) * scaler;
      score = powf(score, FindVertexScore_CacheDecayPower);
    }
  }

  return score;
}

float computeVertexValenceScore(uint32_t numActiveFaces)
{
  const float FindVertexScore_ValenceBoostScale = 2.0f;
  const float FindVertexScore_ValenceBoostPower = 0.5f;

  float score = 0.f;

  // Bonus points for having a low number of tris still to
  // use the vert, so we get rid of lone verts quickly.
  float valenceBoost = powf(static_cast<float>(numActiveFaces),
                            -FindVertexScore_ValenceBoostPower);
  score += FindVertexScore_ValenceBoostScale * valenceBoost;

  return score;
}

const int kMaxVertexCacheSize = 64;
const uint32_t kMaxPrecomputedVertexValenceScores = 64;
float s_vertexCacheScores[kMaxVertexCacheSize + 1][kMaxVertexCacheSize];
float s_vertexValenceScores[kMaxPrecomputedVertexValenceScores];

bool computeVertexScores()
{
  for (int cacheSize = 0; cacheSize <= kMaxVertexCacheSize; ++cacheSize)
  {
    for (int cachePos = 0; cachePos < cacheSize; ++cachePos)
    {
      s_vertexCacheScores[cacheSize][cachePos] =
          computeVertexCacheScore(cachePos, cacheSize);
    }
  }

  for (uint32_t valence = 0; valence < kMaxPrecomputedVertexValenceScores;
       ++valence)
  {
    s_vertexValenceScores[valence] = computeVertexValenceScore(valence);
  }

  return true;
}
bool s_vertexScoresComputed = computeVertexScores();

float findVertexScore(uint32_t numActiveFaces, uint32_t cachePosition,
                      uint32_t vertexCacheSize)
{
  _INTR_ASSERT(s_vertexScoresComputed);

  if (numActiveFaces == 0)
  {
    // No tri needs this vertex!
    return -1.0f;
  }

  float score = 0.f;
  if (cachePosition < vertexCacheSize)
  {
    score += s_vertexCacheScores[vertexCacheSize][cachePosition];
  }

  if (numActiveFaces < kMaxPrecomputedVertexValenceScores)
  {
    score += s_vertexValenceScores[numActiveFaces];
  }
  else
  {
    score += computeVertexValenceScore(numActiveFaces);
  }

  return score;
}

struct OptimizeVertexData
{
  float score;
  uint32_t activeFaceListStart;
  uint32_t activeFaceListSize;
  uint32_t cachePos0;
  uint32_t cachePos1;
  OptimizeVertexData()
      : score(0.f), activeFaceListStart(0), activeFaceListSize(0), cachePos0(0),
        cachePos1(0)
  {
  }
};
}

void optimizeFaces(const uint32_t* indexList, uint32_t indexCount,
                   uint32_t vertexCount, uint32_t* newIndexList,
                   uint32_t lruCacheSize)
{
  std::vector<OptimizeVertexData> vertexDataList;
  vertexDataList.resize(vertexCount);

  // compute face count per vertex
  for (uint32_t i = 0; i < indexCount; ++i)
  {
    uint32_t index = indexList[i];
    _INTR_ASSERT(index < vertexCount);
    OptimizeVertexData& vertexData = vertexDataList[index];
    vertexData.activeFaceListSize++;
  }

  std::vector<uint32_t> activeFaceList;

  const uint32_t kEvictedCacheIndex = std::numeric_limits<uint32_t>::max();

  {
    // allocate face list per vertex
    uint32_t curActiveFaceListPos = 0;
    for (uint32_t i = 0; i < vertexCount; ++i)
    {
      OptimizeVertexData& vertexData = vertexDataList[i];
      vertexData.cachePos0 = kEvictedCacheIndex;
      vertexData.cachePos1 = kEvictedCacheIndex;
      vertexData.activeFaceListStart = curActiveFaceListPos;
      curActiveFaceListPos += vertexData.activeFaceListSize;
      vertexData.score = findVertexScore(vertexData.activeFaceListSize,
                                         vertexData.cachePos0, lruCacheSize);
      vertexData.activeFaceListSize = 0;
    }
    activeFaceList.resize(curActiveFaceListPos);
  }

  // fill out face list per vertex
  for (uint32_t i = 0; i < indexCount; i += 3)
  {
    for (uint32_t j = 0; j < 3; ++j)
    {
      uint32_t index = indexList[i + j];
      OptimizeVertexData& vertexData = vertexDataList[index];
      activeFaceList[vertexData.activeFaceListStart +
                     vertexData.activeFaceListSize] = i;
      vertexData.activeFaceListSize++;
    }
  }

  std::vector<uint8_t> processedFaceList;
  processedFaceList.resize(indexCount);

  uint32_t vertexCacheBuffer[(kMaxVertexCacheSize + 3) * 2];
  uint32_t* cache0 = vertexCacheBuffer;
  uint32_t* cache1 = vertexCacheBuffer + (kMaxVertexCacheSize + 3);
  uint32_t entriesInCache0 = 0;

  uint32_t bestFace = 0;
  float bestScore = -1.f;

  const float maxValenceScore =
      findVertexScore(1, kEvictedCacheIndex, lruCacheSize) * 3.f;

  for (uint32_t i = 0; i < indexCount; i += 3)
  {
    if (bestScore < 0.f)
    {
      // no verts in the cache are used by any unprocessed faces so
      // search all unprocessed faces for a new starting point
      for (uint32_t j = 0; j < indexCount; j += 3)
      {
        if (processedFaceList[j] == 0)
        {
          uint32_t face = j;
          float faceScore = 0.f;
          for (uint32_t k = 0; k < 3; ++k)
          {
            uint32_t index = indexList[face + k];
            OptimizeVertexData& vertexData = vertexDataList[index];
            _INTR_ASSERT(vertexData.activeFaceListSize > 0);
            _INTR_ASSERT(vertexData.cachePos0 >= lruCacheSize);
            faceScore += vertexData.score;
          }

          if (faceScore > bestScore)
          {
            bestScore = faceScore;
            bestFace = face;

            _INTR_ASSERT(bestScore <= maxValenceScore);
            if (bestScore >= maxValenceScore)
            {
              break;
            }
          }
        }
      }
      _INTR_ASSERT(bestScore >= 0.f);
    }

    processedFaceList[bestFace] = 1;
    uint32_t entriesInCache1 = 0;

    // add bestFace to LRU cache and to newIndexList
    for (uint32_t v = 0; v < 3; ++v)
    {
      uint32_t index = indexList[bestFace + v];
      newIndexList[i + v] = index;

      OptimizeVertexData& vertexData = vertexDataList[index];

      if (vertexData.cachePos1 >= entriesInCache1)
      {
        vertexData.cachePos1 = entriesInCache1;
        cache1[entriesInCache1++] = index;

        if (vertexData.activeFaceListSize == 1)
        {
          --vertexData.activeFaceListSize;
          continue;
        }
      }

      _INTR_ASSERT(vertexData.activeFaceListSize > 0);
      uint32_t* begin = &activeFaceList[vertexData.activeFaceListStart];
      uint32_t* end = &(activeFaceList[vertexData.activeFaceListStart +
                                       vertexData.activeFaceListSize - 1]) +
                      1;
      uint32_t* it = std::find(begin, end, bestFace);
      _INTR_ASSERT(it != end);
      std::swap(*it, *(end - 1));
      --vertexData.activeFaceListSize;
      vertexData.score = findVertexScore(vertexData.activeFaceListSize,
                                         vertexData.cachePos1, lruCacheSize);
    }

    // move the rest of the old verts in the cache down and compute their new
    // scores
    for (uint32_t c0 = 0; c0 < entriesInCache0; ++c0)
    {
      uint32_t index = cache0[c0];
      OptimizeVertexData& vertexData = vertexDataList[index];

      if (vertexData.cachePos1 >= entriesInCache1)
      {
        vertexData.cachePos1 = entriesInCache1;
        cache1[entriesInCache1++] = index;
        vertexData.score = findVertexScore(vertexData.activeFaceListSize,
                                           vertexData.cachePos1, lruCacheSize);
      }
    }

    // find the best scoring triangle in the current cache (including up to 3
    // that were just evicted)
    bestScore = -1.f;
    for (uint32_t c1 = 0; c1 < entriesInCache1; ++c1)
    {
      uint32_t index = cache1[c1];
      OptimizeVertexData& vertexData = vertexDataList[index];
      vertexData.cachePos0 = vertexData.cachePos1;
      vertexData.cachePos1 = kEvictedCacheIndex;
      for (uint32_t j = 0; j < vertexData.activeFaceListSize; ++j)
      {
        uint32_t face = activeFaceList[vertexData.activeFaceListStart + j];
        float faceScore = 0.f;
        for (uint32_t v = 0; v < 3; v++)
        {
          uint32_t faceIndex = indexList[face + v];
          OptimizeVertexData& faceVertexData = vertexDataList[faceIndex];
          faceScore += faceVertexData.score;
        }
        if (faceScore > bestScore)
        {
          bestScore = faceScore;
          bestFace = face;
        }
      }
    }

    std::swap(cache0, cache1);
    entriesInCache0 = std::min(entriesInCache1, lruCacheSize);
  }
}
}
}
}
