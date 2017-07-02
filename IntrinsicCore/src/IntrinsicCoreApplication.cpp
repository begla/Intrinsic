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
#include <cmath>

#pragma warning(disable : 4996)

namespace Intrinsic
{
namespace Core
{
_INTR_HASH_MAP(Name, Dod::Components::ComponentManagerEntry)
Application::_componentManagerMapping;
_INTR_HASH_MAP(Name, Dod::Resources::ResourceManagerEntry)
Application::_resourceManagerMapping;

_INTR_HASH_MAP(Name, Dod::PropertyCompilerEntry)
Application::_componentPropertyCompilerMapping;
_INTR_HASH_MAP(Name, Dod::PropertyCompilerEntry)
Application::_resourcePropertyCompilerMapping;

_INTR_ARRAY(Dod::Components::ComponentManagerEntry)
Application::_orderedComponentManagers;

enki::TaskScheduler Application::_scheduler;
bool Application::_running = true;

void Application::init(void* p_PlatformHandle, void* p_PlatformWindow)
{
  // Initializes physics
  Physics::System::init();

  // Threading
  _scheduler.Initialize(std::min(enki::GetNumHardwareThreads(), 6u));

  // Initializes managers
  initManagers();

  // Initializes renderer
  R::RenderSystem::init(p_PlatformHandle, p_PlatformWindow);

// MicroProfile init.
#if defined(_INTR_PROFILING_ENABLED)
  {
    MicroProfileOnThreadCreate("Main");

    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    MicroProfileStartContextSwitchTrace();

#if defined(MICROPROFILE_GPU_TIMERS_VULKAN)
    MicroProfileGpuInitVulkan(
        &R::RenderSystem::_vkDevice, &R::RenderSystem::_vkPhysicalDevice,
        &R::RenderSystem::_vkQueue,
        &R::RenderSystem::_vkGraphicsAndComputeQueueFamilyIndex, 1u);
#endif // MICROPROFILE_GPU_TIMERS_VULKAN
  }
#endif // _INTR_PROFILING_ENABLED

  // Load resource managers
  {
    Resources::MeshManager::loadFromMultipleFiles("managers/meshes/",
                                                  ".mesh.json");
    Resources::MeshManager::createAllResources();

    Resources::ScriptManager::loadFromMultipleFiles("managers/scripts/",
                                                    ".script.json");
    Resources::ScriptManager::createAllResources();

    Resources::PostEffectManager::loadFromMultipleFiles(
        "managers/post_effects/", ".post_effect.json");
  }

  // Initializes world
  {
    World::init();
    World::load("worlds/" + Settings::Manager::_initialWorld);
  }

  // Initializes game states
  {
    GameStates::Editing::init();
  }

  {
    R::RenderSystem::onViewportChanged();
  }
}

void Application::initManagers()
{
  // Initializes component managers
  {
    GameStates::Manager::init();
    Entity::EntityManager::init();
    Components::NodeManager::init();
    Components::MeshManager::init();
    Components::CameraManager::init();
    Components::ScriptManager::init();
    Components::PostEffectVolumeManager::init();
    Components::RigidBodyManager::init();
    Components::CharacterControllerManager::init();
    Components::SwarmManager::init();
    Components::CameraControllerManager::init();
    Components::PlayerManager::init();
    Components::LightManager::init();
    Components::IrradianceProbeManager::init();
    Components::SpecularProbeManager::init();
    Components::DecalManager::init();
  }

  // Initialize resource managers
  {
    Resources::FrustumManager::init();
    Resources::MeshManager::init();
    Resources::ScriptManager::init();
    Resources::PostEffectManager::init();
  }
}

void Application::initEventSystem()
{
  Resources::EventManager::init();
  Resources::EventListenerManager::init();

  // Input
  SystemEventProvider::SDL::init();
  Input::System::init();
}

void Application::shutdown() { _running = false; }
}
}
