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

// Vulkan settings
#define _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT 128u

#define _INTR_VK_PER_INSTANCE_UNIFORM_MEMORY_IN_BYTES 256u * 1024u * 1024u
#define _INTR_VK_PER_MATERIAL_UNIFORM_MEMORY_IN_BYTES 1u * 1024u * 1024u

#define _INTR_VK_PER_INSTANCE_PAGE_SMALL_SIZE_IN_BYTES 256u
#define _INTR_VK_PER_INSTANCE_PAGE_SMALL_COUNT _INTR_MAX_DRAW_CALL_COUNT
#define _INTR_VK_PER_INSTANCE_PAGE_LARGE_SIZE_IN_BYTES 2048u
#define _INTR_VK_PER_INSTANCE_PAGE_LARGE_COUNT 256u

#define _INTR_VK_PER_MATERIAL_PAGE_SIZE_IN_BYTES 256u
#define _INTR_VK_PER_MATERIAL_PAGE_COUNT _INTR_MAX_MATERIAL_COUNT

#define _INTR_PSSM_SPLIT_COUNT 4u
#define _INTR_MAX_SHADOW_MAP_COUNT 16u
#define _INTR_MAX_FRUSTUMS_PER_FRAME_COUNT (_INTR_MAX_SHADOW_MAP_COUNT + 1u)

// Vulkan macros
#if !defined(_INTR_FINAL_BUILD)
#define _INTR_PROFILE_GPU_MARKER_REGION(_name)                                 \
  GpuMarkerRegion gpuMarker = GpuMarkerRegion(_name)
#define _INTR_VK_CHECK_RESULT(x) assert(x == VK_SUCCESS)
#else
#define _INTR_VK_CHECK_RESULT(x)
#define _INTR_PROFILE_GPU_MARKER_REGION(_name)
#endif // _INTR_FINAL_BUILD
