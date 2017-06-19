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

// Windows includes
#if defined(_WIN32)
#define NOMINMAX
#include "windows.h"
#endif // _WIN32

// GLM and GLI related includes
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gli/gli.hpp>

// Tinydir
#include "tinydir/tinydir.h"

// SDL
#include "SDL.h"
#include "SDL_syswm.h"
#undef Bool
#undef Success

// TLSF
#include "tlsf.h"

// Rlutil
#include "rlutil/rlutil.h"

// rapidjson related includes
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

// Sparsepp
#include "sparsepp/sparsepp.h"

// STL related includes
#include "stdint.h"
#include "assert.h"
#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <fstream>
#include <atomic>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <thread>

// Core related includes
#include "IntrinsicCoreVersion.h"
#include "IntrinsicCorePrerequisites.h"

// Logging
#include "IntrinsicCoreLogManager.h"

// STL allocator include
#include "IntrinsicCoreTlsfAllocator.h"
#include "IntrinsicCoreStlAllocator.h"

// Lua related includes
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#if !defined(_INTR_FINAL_BUILD)
#define SOL_CHECK_ARGUMENTS
#define SOL_SAFE_USERTYPE
#else
#define SOL_NO_EXCEPTIONS
#endif // _INTR_FINAL_BUILD
#include <sol.hpp>

// Profiling
#if defined(_INTR_PROFILING_ENABLED)
#define MICROPROFILE_ENABLED 1
#define MICROPROFILE_GPU_TIMERS 1
#include "microprofile.h"
#undef snprintf
#endif // _INTR_PROFILING_ENABLED

// Threading
#include "TaskScheduler.h"

#if defined(_WIN32)
#include "IntrinsicCoreThreadingWin32.h"
#else
#include "IntrinsicCoreThreadingUnix.h"
#endif // _WIN32

// Some shortcuts
namespace Intrinsic
{
namespace Core
{
namespace Resources
{
}
namespace Components
{
}
}
}

using namespace Intrinsic;
using namespace Intrinsic::Core;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
}
}
}
}

// Namespace aliases
namespace RV = Intrinsic::Renderer::Vulkan;
namespace RVResources = RV::Resources;

namespace CResources = Intrinsic::Core::Resources;
namespace CComponents = Intrinsic::Core::Components;

// Core related includes
#include "IntrinsicCoreTriangleOptimizer.h"
#include "IntrinsicCoreSettingsManager.h"
#include "IntrinsicCoreLockFreeStack.h"
#include "IntrinsicCoreLinearOffsetAllocator.h"
#include "IntrinsicCoreLockFreeFixedBlockAllocator.h"
#include "IntrinsicStringUtil.h"
#include "IntrinsicUtil.h"
#include "IntrinsicCoreSimd.h"
#include "IntrinsicCoreMath.h"
#include "IntrinsicCoreName.h"
#include "IntrinsicCoreTimingHelper.h"
#include "IntrinsicCoreDod.h"
#include "IntrinsicCoreIBL.h"
#include "IntrinsicCoreJsonHelper.h"
#include "IntrinsicCoreEntity.h"
#include "IntrinsicCoreDodResources.h"
#include "IntrinsicCoreDodComponents.h"
#include "IntrinsicCoreApplication.h"
#include "IntrinsicCoreAlgorithm.h"
#include "IntrinsicCoreResourcesEventListener.h"
#include "IntrinsicCoreResourcesEvent.h"
#include "IntrinsicCoreResourcesFrustum.h"
#include "IntrinsicCoreResourcesMesh.h"
#include "IntrinsicCoreComponentsNode.h"
#include "IntrinsicCoreComponentsMesh.h"
#include "IntrinsicCoreComponentsSwarm.h"
#include "IntrinsicCoreComponentsRigidBody.h"
#include "IntrinsicCoreComponentsCamera.h"
#include "IntrinsicCoreComponentsCameraController.h"
#include "IntrinsicCoreComponentsCharacterController.h"
#include "IntrinsicCoreComponentsPlayer.h"
#include "IntrinsicCoreComponentsLight.h"
#include "IntrinsicCoreComponentsIrradianceProbe.h"
#include "IntrinsicCoreComponentsDecal.h"

#include "IntrinsicCoreWorld.h"
#include "IntrinsicCoreResourcesPostEffect.h"
#include "IntrinsicCoreComponentsPostEffectVolume.h"

#include "IntrinsicCoreTaskManager.h"
#include "IntrinsicCorePhysicsSystem.h"
#include "IntrinsicCoreInputSystem.h"
#include "IntrinsicCoreSystemEventProviderSDL.h"
#include "IntrinsicCoreGameStatesEditing.h"
#include "IntrinsicCoreGameStatesMain.h"
#include "IntrinsicCoreGameStatesBenchmark.h"
#include "IntrinsicCoreGameStatesManager.h"
#include "IntrinsicCoreResourcesScript.h"
#include "IntrinsicCoreComponentsScript.h"
#include "IntrinsicCorePhysxHelper.h"

// Renderer includes
#include "stdafx_vulkan.h"
