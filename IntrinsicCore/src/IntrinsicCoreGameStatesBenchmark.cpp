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

// PhysX includes
#include "extensions/PxDistanceJoint.h"
#include "PxRigidDynamic.h"
#include "extensions/PxRigidBodyExt.h"
#include "PxScene.h"
#include "extensions/PxJoint.h"

namespace Intrinsic
{
namespace Core
{
namespace GameStates
{
namespace
{
_INTR_ARRAY(Benchmark::Path) _paths;
_INTR_ARRAY(Benchmark::Data) _benchmarkData;
uint32_t _pathIdx = 0u;
float _pathPos = 0.0f;
rapidjson::Document _benchmarkDesc;
}

void Benchmark::init() {}

// <-

void Benchmark::activate()
{
  Entity::EntityRef entityRef =
      Entity::EntityManager::getEntityByName(_N(BenchmarkCamera));
  if (!entityRef.isValid())
  {
    _INTR_LOG_ERROR("'BenchmarkCamera' not available...");
    return;
  }

  Components::CameraRef cameraRef =
      Components::CameraManager::getComponentForEntity(entityRef);
  _INTR_ASSERT(cameraRef.isValid());

  World::setActiveCamera(cameraRef);

  _pathIdx = 0u;
  _pathPos = 0.0f;
  _paths.clear();
  _benchmarkData.clear();

  parseBenchmark(_benchmarkDesc);
  assembleBenchmarkPaths(_benchmarkDesc, _paths);
  _benchmarkData.resize(_paths.size());

  _INTR_LOG_INFO("Starting benchmark...\n---");
  if (!_paths.empty())
  {
    _INTR_LOG_INFO("Benchmarking path '%s'...", _paths[0u].name.c_str());
  }
}

// <-

void Benchmark::deativate() {}

// <-

void Benchmark::parseBenchmark(rapidjson::Document& p_BenchmarkDesc)
{
  const _INTR_STRING& worldFilePath = World::_filePath;

  _INTR_STRING worldFileName, worldFileExtension;
  StringUtil::extractFileNameAndExtension(worldFilePath, worldFileName,
                                          worldFileExtension);

  // Parse benchmark
  _INTR_STRING filePath = "worlds/" + worldFileName + ".benchmark.json";

  {
    FILE* fp = fopen(filePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_ERROR("Failed to load benchmark from file '%s'...",
                      filePath.c_str());
      return;
    }

    char* readBuffer = (char*)Memory::Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      p_BenchmarkDesc.ParseStream(is);
      fclose(fp);
    }
    Memory::Tlsf::MainAllocator::free(readBuffer);
  }
}

// <-

void Benchmark::assembleBenchmarkPaths(
    const rapidjson::Document& p_BenchmarkDesc, _INTR_ARRAY(Path) & p_Paths)
{
  // Assemble paths
  for (uint32_t i = 0u; i < p_BenchmarkDesc.Size(); ++i)
  {
    const rapidjson::Value& pathDesc = p_BenchmarkDesc[i];
    const rapidjson::Value& nodeDescs = pathDesc["nodes"];

    Path path;
    path.name = pathDesc["name"].GetString();
    path.camSpeed = pathDesc["camSpeed"].GetFloat();
    path.currentTime = pathDesc["currentTime"].GetFloat();

    for (uint32_t j = 0u; j < nodeDescs.Size(); ++j)
    {
      Entity::EntityRef nodeEntity =
          Entity::EntityManager::getEntityByName(nodeDescs[j].GetString());
      const glm::vec3 worldPos = Components::NodeManager::_worldPosition(
          Components::NodeManager::getComponentForEntity(nodeEntity));

      path.nodePositions.push_back(worldPos);
    }

    p_Paths.push_back(path);
  }
}

// <-

void Benchmark::update(float p_DeltaT)
{
  _INTR_PROFILE_CPU("Game States", "Benchmark");

  if (_paths.empty())
  {
    return;
  }

  const Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(World::_activeCamera));

  const Path& currentPath = _paths[_pathIdx];
  Data& data = _benchmarkData[_pathIdx];

  const glm::vec3 newCamPos =
      Math::bezierQuadratic(currentPath.nodePositions, _pathPos);
  const glm::vec3 newCamPos1 = Math::bezierQuadratic(
      currentPath.nodePositions, glm::clamp(_pathPos + 0.01f, 0.0f, 1.0f));

  const glm::vec3 camForward = glm::normalize(newCamPos1 - newCamPos);
  glm::vec3 camRight =
      glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)) * camForward;
  camRight.y = 0.0f;
  camRight = glm::normalize(camRight);

  const glm::vec3 camUp = glm::cross(camForward, camRight);

  Components::NodeManager::_position(camNodeRef) = newCamPos;
  Components::NodeManager::_orientation(camNodeRef) =
      glm::quat_cast(glm::mat3(camRight, camUp, camForward));
  Components::NodeManager::updateTransforms(camNodeRef);

  World::_currentTime = currentPath.currentTime;

  _pathPos += p_DeltaT * currentPath.camSpeed;
  data.meanFps = 1.0f / TaskManager::_lastActualFrameDuration;

  if (_pathPos >= 1.0f)
  {
    _pathIdx = _pathIdx + 1u;

    // Benchmark finished
    if (_pathIdx >= _paths.size())
    {
      // Calc. total score
      {
        float totalScore = 0.0f;
        for (uint32_t i = 0u; i < _benchmarkData.size(); ++i)
          totalScore += _benchmarkData[i].calcScore();
        totalScore /= _benchmarkData.size();

        _INTR_LOG_INFO("Finished benchmarking, total score: %u\n---",
                       (uint32_t)totalScore);
      }

      // Reset data
      {
        _pathIdx = 0u;
        for (uint32_t i = 0u; i < _benchmarkData.size(); ++i)
        {
          _benchmarkData[i] = Data();
        }
      }
    }
    else
    {
      _INTR_LOG_INFO("Score: %u", data.calcScore());
    }

    _INTR_LOG_INFO("Benchmarking path '%s'...", _paths[_pathIdx].name.c_str());
    _pathPos = 0.0f;
  }
}
}
}
}
