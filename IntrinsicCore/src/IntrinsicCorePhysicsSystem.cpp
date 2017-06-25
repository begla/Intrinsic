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
#include "foundation/PxFoundationVersion.h"
#include "PxPhysicsVersion.h"
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

struct PhysXErrorCallback : public physx::PxErrorCallback
{
  void reportError(physx::PxErrorCode::Enum code, const char* message,
                   const char* file, int line) override
  {
    _INTR_LOG_WARNING("PhysX Error Message: \"%s\" in file \"%s\" line %i",
                      message, file, line);
  }
} _physXErrorCallback;
}

// Static members
physx::PxPhysics* System::_pxPhysics;
physx::PxFoundation* System::_pxFoundation;
physx::PxCooking* System::_pxCooking;
physx::PxCpuDispatcher* System::_pxCpuDispatcher;
physx::PxScene* System::_pxScene;

void System::init()
{
  static physx::PxDefaultAllocator defaultAllocator;

  physx::PxTolerancesScale toleranceScale;

  _pxFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, defaultAllocator,
                                     _physXErrorCallback);
  _INTR_ASSERT(_pxFoundation);

  _pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *_pxFoundation,
                               toleranceScale, false, nullptr);
  _INTR_ASSERT(_pxPhysics);

  _pxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *_pxFoundation,
                               physx::PxCookingParams(toleranceScale));
  _INTR_ASSERT(_pxCooking);

  physx::PxCookingParams params(_pxPhysics->getTolerancesScale());
  params.meshCookingHint = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
  _pxCooking->setParams(params);

  _pxCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(
      Application::_scheduler.GetNumTaskThreads());
  _INTR_ASSERT(_pxCpuDispatcher);

  physx::PxSceneDesc sceneDesc = physx::PxSceneDesc(toleranceScale);
  sceneDesc.cpuDispatcher = _pxCpuDispatcher;
  sceneDesc.gravity = physx::PxVec3(0.0f, -30.0f, 0.0f);
  sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

  _pxScene = _pxPhysics->createScene(sceneDesc);
  _INTR_ASSERT(_pxScene);

  // Updates internal PhysX parameters
  setDebugRenderingFlags(_debugRenderingFlags);
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
      R::RenderPass::Debug::renderLine(PhysicsHelper::convert(line.pos0),
                                       PhysicsHelper::convert(line.pos1),
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
