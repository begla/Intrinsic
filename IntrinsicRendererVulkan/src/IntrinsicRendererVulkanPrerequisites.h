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

#pragma once

#define _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT 128u

#define _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT 2u

#define _INTR_VK_PER_INSTANCE_BLOCK_SMALL_SIZE_IN_BYTES 256u
#define _INTR_VK_PER_INSTANCE_BLOCK_SMALL_COUNT _INTR_MAX_DRAW_CALL_COUNT
#define _INTR_VK_PER_INSTANCE_BLOCK_LARGE_SIZE_IN_BYTES 2048u
#define _INTR_VK_PER_INSTANCE_BLOCK_LARGE_COUNT 256u

#define _INTR_VK_PER_MATERIAL_BLOCK_SIZE_IN_BYTES 256u
#define _INTR_VK_PER_MATERIAL_BLOCK_COUNT _INTR_MAX_MATERIAL_COUNT

#define _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES                          \
  (_INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT *                                   \
       _INTR_VK_PER_INSTANCE_BLOCK_SMALL_SIZE_IN_BYTES *                       \
       _INTR_VK_PER_INSTANCE_BLOCK_SMALL_COUNT +                               \
   _INTR_VK_PER_INSTANCE_DATA_BUFFER_COUNT *                                   \
       _INTR_VK_PER_INSTANCE_BLOCK_LARGE_SIZE_IN_BYTES *                       \
       _INTR_VK_PER_INSTANCE_BLOCK_LARGE_COUNT)
#define _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES                          \
  (_INTR_VK_PER_MATERIAL_BLOCK_SIZE_IN_BYTES *                                 \
   _INTR_VK_PER_MATERIAL_BLOCK_COUNT)

#define _INTR_PSSM_SPLIT_COUNT 4u
#define _INTR_MAX_SHADOW_MAP_COUNT 4u
#define _INTR_MAX_FRUSTUMS_PER_FRAME_COUNT (_INTR_MAX_SHADOW_MAP_COUNT + 1u)

// Vulkan macros
#if !defined(_INTR_FINAL_BUILD)
#define _INTR_PROFILE_GPU_MARKER_REGION(_name)                                 \
  GpuMarkerRegion _INTR_CONCAT(marker, __COUNTER__) = GpuMarkerRegion(_name)
#define _INTR_VK_CHECK_RESULT(x) assert(x == VK_SUCCESS)
#else
#define _INTR_VK_CHECK_RESULT(x)
#define _INTR_PROFILE_GPU_MARKER_REGION(_name)
#endif // _INTR_FINAL_BUILD
