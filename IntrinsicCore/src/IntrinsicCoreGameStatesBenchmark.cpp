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
struct Path
{
  _INTR_ARRAY(glm::vec3) nodePositions;
};

_INTR_ARRAY(Path) _paths;
uint32_t _pathIdx = 0u;
float _pathPos = 0.0f;
}

void Benchmark::init() {}

// <-

void Benchmark::activate()
{
  Entity::EntityRef entityRef =
      Entity::EntityManager::getEntityByName(_N(BenchmarkCamera));
  _INTR_ASSERT(entityRef.isValid());
  Components::CameraRef cameraRef =
      Components::CameraManager::getComponentForEntity(entityRef);
  _INTR_ASSERT(cameraRef.isValid());

  World::setActiveCamera(cameraRef);

  // Parse benchmark
  _INTR_STRING filePath =
      "worlds/" +
      Entity::EntityManager::_name(
          Components::NodeManager::_entity(World::getRootNode()))
          .getString() +
      ".benchmark.json";

  rapidjson::Document benchmarkDesc;
  {
    FILE* fp = fopen(filePath.c_str(), "rb");

    if (fp == nullptr)
    {
      _INTR_LOG_ERROR("Failed to load benchmark from file '%s'...",
                      filePath.c_str());
      return;
    }

    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileReadStream is(fp, readBuffer, 65536u);
      benchmarkDesc.ParseStream(is);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(readBuffer);
  }

  _pathIdx = 0u;
  _pathPos = 0.0f;
  _paths.clear();

  // Assemble paths
  for (uint32_t i = 0u; i < benchmarkDesc.Size(); ++i)
  {
    rapidjson::Value& pathDesc = benchmarkDesc[i];
    rapidjson::Value& nodeDescs = pathDesc["nodes"];

    Path path = {};

    for (uint32_t j = 0u; j < nodeDescs.Size(); ++j)
    {
      Entity::EntityRef nodeEntity =
          Entity::EntityManager::getEntityByName(nodeDescs[j].GetString());
      path.nodePositions.push_back(Components::NodeManager::_worldPosition(
          Components::NodeManager::getComponentForEntity(nodeEntity)));
    }

    _paths.push_back(path);
  }
}

// <-

void Benchmark::deativate() {}

// <-

void Benchmark::update(float p_DeltaT)
{
  _INTR_PROFILE_CPU("Game States", "Benchmark Game State Update");

  const Components::NodeRef camNodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::CameraManager::_entity(World::getActiveCamera()));

  const Path& currentPath = _paths[_pathIdx];

  const glm::vec3 newCamPos =
      Math::bezierQuadratic(currentPath.nodePositions, _pathPos);
  const glm::vec3 newCamPos1 = Math::bezierQuadratic(
      currentPath.nodePositions, glm::clamp(_pathPos + 0.01f, 0.0f, 1.0f));

  const glm::quat newCamOrientation = glm::rotation(
      glm::vec3(0.0f, 0.0f, -1.0f), glm::normalize(newCamPos - newCamPos1));

  Components::NodeManager::_position(camNodeRef) = newCamPos;
  Components::NodeManager::_orientation(camNodeRef) = newCamOrientation;
  Components::NodeManager::updateTransforms(camNodeRef);

  _pathPos += p_DeltaT * 0.1f;
  if (_pathPos >= 1.0f)
  {
    _pathIdx = (_pathIdx + 1u) % _paths.size();
    _pathPos = 0.0f;
  }
}
}
}
}
