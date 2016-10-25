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
#include "foundation/PxFoundation.h"
#include "foundation/PxErrorCallback.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxDefaultErrorCallback.h"
#include "extensions/PxDefaultSimulationFilterShader.h"
#include "extensions/PxDefaultAllocator.h"
#include "common/PxTolerancesScale.h"
#include "extensions/PxVisualDebuggerExt.h"
#include "common/PxRenderBuffer.h"

namespace Intrinsic
{
namespace Core
{
namespace Physics
{
namespace
{
uint32_t _debugRenderingFlags = 0u;
}

// Static members
physx::PxPhysics* System::_pxPhysics;
physx::PxFoundation* System::_pxFoundation;
physx::PxCooking* System::_pxCooking;
physx::PxCpuDispatcher* System::_pxCpuDispatcher;
physx::PxScene* System::_pxScene;

void System::init()
{
  static physx::PxDefaultErrorCallback defaultErrorCallback;
  static physx::PxDefaultAllocator defaultAllocator;

  physx::PxTolerancesScale toleranceScale;

  _pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocator,
                                     defaultErrorCallback);
  _INTR_ASSERT(_pxFoundation);

  _pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *_pxFoundation,
                               toleranceScale, false, nullptr);
  _INTR_ASSERT(_pxPhysics);

  _pxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *_pxFoundation,
                               physx::PxCookingParams(toleranceScale));
  _INTR_ASSERT(_pxCooking);

  _pxCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2u);
  _INTR_ASSERT(_pxCpuDispatcher);

  physx::PxSceneDesc sceneDesc = physx::PxSceneDesc(toleranceScale);
  sceneDesc.cpuDispatcher = _pxCpuDispatcher;
  sceneDesc.gravity = physx::PxVec3(0.0f, -30.0f, 0.0f);
  sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

  _pxScene = _pxPhysics->createScene(sceneDesc);
  _INTR_ASSERT(_pxScene);

  // Try to connect to the visual debugger
  if (_pxPhysics->getPvdConnectionManager())
  {
    const char* hostIp = "127.0.0.1";
    int port = 5425;
    unsigned int timeout = 100;

    physx::PxVisualDebuggerConnectionFlags connectionFlags =
        physx::PxVisualDebuggerExt::getAllConnectionFlags();

    physx::PxVisualDebuggerConnection* conn =
        physx::PxVisualDebuggerExt::createConnection(
            _pxPhysics->getPvdConnectionManager(), hostIp, port, timeout,
            connectionFlags);
  }

  // Updates internal PhysX parameters
  setDebugRenderingFlags(_debugRenderingFlags);
}

// <-

void System::updatePvdCamera()
{
  _INTR_PROFILE_CPU("Physics", "Update PVD Camera");

  Components::CameraRef activeCameraRef = World::getActiveCamera();

  if (activeCameraRef.isValid() && _pxPhysics->getVisualDebugger())
  {
    Components::NodeRef activeCameraNodeRef =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCameraRef));

    physx::PxVec3 up =
        PhysxHelper::convert(Components::CameraManager::_up(activeCameraRef));
    physx::PxVec3 forward = PhysxHelper::convert(
        Components::CameraManager::_forward(activeCameraRef));
    physx::PxVec3 orig = PhysxHelper::convert(
        Components::NodeManager::_worldPosition(activeCameraNodeRef));

    _pxPhysics->getVisualDebugger()->updateCamera("MainCamera", orig, up,
                                                  orig + forward);
  }
}

// <-

void System::renderLineDebugGeometry()
{
  _INTR_PROFILE_CPU("Physics", "Render Line Debug Geometry");

  if ((_debugRenderingFlags & DebugRenderingFlags::kEnabled) > 0u &&
      (GameStates::Manager::getActiveGameState() ==
       GameStates::GameState::kEditing))
  {
    const physx::PxRenderBuffer& rb = _pxScene->getRenderBuffer();
    for (uint32_t i = 0; i < rb.getNbLines(); i++)
    {
      const physx::PxDebugLine& line = rb.getLines()[i];
      Renderer::Vulkan::RenderPass::Debug::renderLine(
          PhysxHelper::convert(line.pos0), PhysxHelper::convert(line.pos1),
          line.color0, line.color1);
    }
  }
}

// <-

void System::setDebugRenderingFlags(uint32_t p_DebugRenderingFlags)
{
  _debugRenderingFlags = p_DebugRenderingFlags;

  if ((_debugRenderingFlags & DebugRenderingFlags::kEnabled) > 0u)
  {
    _pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,
                                        1.0f);
    _pxScene->setVisualizationParameter(
        physx::PxVisualizationParameter::eACTOR_AXES, 1.0f);
    _pxScene->setVisualizationParameter(
        physx::PxVisualizationParameter::eCOLLISION_AABBS, 1.0f);
    _pxScene->setVisualizationParameter(
        physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
  }
}

// <-

void System::dispatchSimulation(float p_DeltaT)
{
  _INTR_PROFILE_CPU("Physics", "Dispatch Simulation");

  _pxScene->simulate(p_DeltaT);
}

// <-

void System::syncSimulation()
{
  _INTR_PROFILE_CPU("Physics", "Fetch Results");

  _pxScene->fetchResults(true);
}
}
}
}
