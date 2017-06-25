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

#if defined(_INTR_PROFILING_ENABLED)
MICROPROFILE_DEFINE(MAIN, "MAIN", "Main", 0x00ff00);
#endif // _INTR_PROFILING_ENABLED

namespace Intrinsic
{
namespace Core
{
namespace
{
float _stepAccum = 0.0f;
const float _stepSize = 0.016f;

struct PhysicsUpdateTaskSet : enki::ITaskSet
{
  virtual ~PhysicsUpdateTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    const float modDeltaT =
        TaskManager::_lastDeltaT * TaskManager::_timeModulator;
    _stepAccum += modDeltaT;

    while (_stepAccum > _stepSize)
    {
      Physics::System::dispatchSimulation(modDeltaT);
      Physics::System::syncSimulation();

      _stepAccum -= modDeltaT;
    }
  };

} _physicsUpdateTaskSet;
}

// Static members
float TaskManager::_lastDeltaT = 0.016f;
float TaskManager::_lastActualFrameDuration = 0.016f;
float TaskManager::_totalTimePassed = 0.0f;
uint32_t TaskManager::_frameCounter = 0u;
uint64_t TaskManager::_lastUpdate = 0u;
float TaskManager::_timeModulator = 1.0f;

void TaskManager::executeTasks()
{
#if defined(_INTR_PROFILING_ENABLED)
  MICROPROFILE_SCOPE(MAIN);
#endif // _INTR_PROFILING_ENABLED

  _INTR_PROFILE_CPU("TaskManager", "Execute Tasks");

  if (_frameCounter > 0u)
  {
    _INTR_PROFILE_CPU("TaskManager", "Limit To Max FPS");

    _lastDeltaT = std::min(
        (TimingHelper::getMicroseconds() - _lastUpdate) * 0.000001f, 0.3f);
    _lastActualFrameDuration = _lastDeltaT;

    // Adjust deltaT to target frame rate
    while (_lastDeltaT < Settings::Manager::_targetFrameRate)
    {
      _lastDeltaT = (TimingHelper::getMicroseconds() - _lastUpdate) * 0.000001f;
      std::this_thread::yield();
    }
  }

  // Avoid very high deltaTs due to stalls
  _lastDeltaT = std::min(_lastDeltaT, 0.1f);

  const float modDeltaT = _lastDeltaT * _timeModulator;
  _totalTimePassed += modDeltaT;
  _lastUpdate = TimingHelper::getMicroseconds();

  {
    _INTR_PROFILE_CPU("TaskManager", "Non-Rendering Tasks");

    // Events and input
    {
      _INTR_PROFILE_CPU("TaskManager", "Pump Events");

      Input::System::reset();
      SystemEventProvider::SDL::pumpEvents();
    }

    // Game state update
    {
      GameStates::Manager::update(modDeltaT);
    }

    // Scripts
    {
      Components::ScriptManager::tickScripts(
          Components::ScriptManager::_activeRefs, modDeltaT);
    }

    // Physics
    {
      _INTR_PROFILE_CPU("TaskManager", "Update From Physics Results");

      Components::RigidBodyManager::updateNodesFromActors(
          Components::RigidBodyManager::_activeRefs);
      Components::RigidBodyManager::updateActorsFromNodes(
          Components::RigidBodyManager::_activeRefs);
      Physics::System::renderLineDebugGeometry();
    }

    // Swarms
    {
      _INTR_PROFILE_CPU("TaskManager", "Swarms");

      Components::SwarmManager::simulateSwarms(
          Components::SwarmManager::_activeRefs, modDeltaT);
    }

    // Update the day/night cycle
    {
      World::updateDayNightCycle(modDeltaT);
    }

    // Post effect system
    {
      Components::PostEffectVolumeManager::blendPostEffects(
          Components::PostEffectVolumeManager::_activeRefs);
    }

    // Fire events
    {
      Resources::EventManager::fireEvents();
    }
  }

  {
    _INTR_PROFILE_CPU("TaskManager", "Rendering Tasks");

    // Process physics during rendering
    Application::_scheduler.AddTaskSetToPipe(&_physicsUpdateTaskSet);

    // Rendering
    R::RenderProcess::Default::renderFrame(modDeltaT);

    Application::_scheduler.WaitforTaskSet(&_physicsUpdateTaskSet);
  }

  ++_frameCounter;
}
}
}
