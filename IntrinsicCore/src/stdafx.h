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

#pragma once

// Windows includes
#if defined(_WIN32)
#define NOMINMAX
#include "windows.h"
#endif // _WIN32

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

// GLM related includes
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"

// Intrinsic::Core related includes
#include "IntrinsicCorePrerequisites.h"

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
}
}
using namespace Intrinsic::Core;

// Core related includes
#include "IntrinsicCoreTriangleOptimizer.h"
#include "IntrinsicCoreSettingsManager.h"
#include "IntrinsicCoreLogManager.h"
#include "IntrinsicCoreLockFreeStack.h"
#include "IntrinsicStringUtil.h"
#include "IntrinsicCoreSimd.h"
#include "IntrinsicCoreMath.h"
#include "IntrinsicCoreName.h"
#include "IntrinsicCoreJsonHelper.h"
#include "IntrinsicCoreTimingHelper.h"
#include "IntrinsicCoreDod.h"
#include "IntrinsicCoreEntity.h"
#include "IntrinsicCoreDodResources.h"
#include "IntrinsicCoreDodComponents.h"
#include "IntrinsicCoreApplication.h"
#include "IntrinsicCoreAlgorithm.h"
#include "IntrinsicCoreResourcesEventListener.h"
#include "IntrinsicCoreResourcesEvent.h"
#include "IntrinsicCoreResourcesFrustum.h"
#include "IntrinsicCoreResourcesMesh.h"
#include "IntrinsicCoreResourcesPostEffect.h"
#include "IntrinsicCoreComponentsNode.h"
#include "IntrinsicCoreComponentsMesh.h"
#include "IntrinsicCoreComponentsRigidBody.h"
#include "IntrinsicCoreComponentsCamera.h"
#include "IntrinsicCoreComponentsCameraController.h"
#include "IntrinsicCoreComponentsCharacterController.h"
#include "IntrinsicCoreComponentsPlayer.h"
#include "IntrinsicCoreComponentsPostEffectVolume.h"

#include "IntrinsicCoreWorld.h"
#include "IntrinsicCoreTaskManager.h"
#include "IntrinsicCorePhysicsSystem.h"
#include "IntrinsicCoreInputSystem.h"
#include "IntrinsicCoreSystemEventProviderSDL.h"
#include "IntrinsicCoreGameStatesEditing.h"
#include "IntrinsicCoreGameStatesMain.h"
#include "IntrinsicCoreGameStatesManager.h"
#include "IntrinsicCoreResourcesScript.h"
#include "IntrinsicCoreComponentsScript.h"
#include "IntrinsicCorePhysxHelper.h"

// Renderer includes
#include "stdafx_vulkan.h"
