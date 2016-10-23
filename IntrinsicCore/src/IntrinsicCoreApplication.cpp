// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// Precompiled header file
#include "stdafx.h"

// Renderer includes
#include "IntrinsicRendererVulkanRenderSystem.h"

#pragma warning(disable : 4996)

namespace Intrinsic
{
namespace Core
{
_INTR_HASH_MAP(Name, Intrinsic::Core::Dod::Components::ComponentManagerEntry)
Application::_componentManagerMapping;
_INTR_HASH_MAP(Name, Intrinsic::Core::Dod::Resources::ResourceManagerEntry)
Application::_resourceManagerMapping;

_INTR_HASH_MAP(Name, Intrinsic::Core::Dod::PropertyCompilerEntry)
Application::_componentPropertyCompilerMapping;
_INTR_HASH_MAP(Name, Intrinsic::Core::Dod::PropertyCompilerEntry)
Application::_resourcePropertyCompilerMapping;

_INTR_ARRAY(Intrinsic::Core::Dod::Components::ComponentManagerEntry)
Application::_orderedComponentManagers;

enki::TaskScheduler Application::_scheduler;
bool Application::_running = true;

void Application::init(void* p_PlatformHandle, void* p_PlatformWindow)
{
  // Init. physics
  Physics::System::init();

  // Threading
  _scheduler.Initialize(std::max(enki::GetNumHardwareThreads() - 1u, 1u));

  // Init. managers
  initManagers();

  // Init. renderer
  Renderer::Vulkan::RenderSystem::init(p_PlatformHandle, p_PlatformWindow);

// MicroProfile init.
#if defined(_INTR_PROFILING_ENABLED)
  {
    MicroProfileOnThreadCreate("Main");

    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    MicroProfileStartContextSwitchTrace();

#if defined(MICROPROFILE_GPU_TIMERS_VULKAN)
    MicroProfileGpuInitVulkan(
        &Renderer::Vulkan::RenderSystem::_vkDevice,
        &Renderer::Vulkan::RenderSystem::_vkPhysicalDevice,
        &Renderer::Vulkan::RenderSystem::_vkQueue,
        &Renderer::Vulkan::RenderSystem::_vkGraphicsQueueFamilyIndex, 1u);
#endif // MICROPROFILE_GPU_TIMERS_VULKAN
  }
#endif // _INTR_PROFILING_ENABLED

  // Load resource managers
  {
    Resources::MeshManager::loadFromMultipleFiles("managers/meshes/",
                                                  ".mesh.json");
    Resources::MeshManager::createAllResources();
    Resources::ScriptManager::loadFromSingleFile(
        "managers/Script.manager.json");
    Resources::ScriptManager::createAllResources();
    Resources::PostEffectManager::loadFromSingleFile(
        "managers/PostEffect.manager.json");
  }

  // Init. world
  {
    World::init();
    World::load("worlds/" + Settings::Manager::_initialWorld);
  }

  // Init. game states
  {
    GameStates::Editing::init();
  }
}

void Application::initManagers()
{
  // Init. component managers
  {
    Entity::EntityManager::init();
    Components::NodeManager::init();
    Components::MeshManager::init();
    Components::CameraManager::init();
    Components::ScriptManager::init();
    Components::PostEffectVolumeManager::init();
    Components::RigidBodyManager::init();
    Components::CharacterControllerManager::init();
    Components::CameraControllerManager::init();
    Components::PlayerManager::init();
  }

  // Init resource managers
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
