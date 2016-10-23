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

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#define ADD_THREADS_X 64
#define ADD_THREADS_Y 2

layout (binding = 0) uniform PerInstance
{
  uvec4 dim;
  ivec4 mipLevel;
} uboPerInstance;

layout (binding = 1) uniform sampler2D input0Tex;
layout (binding = 2) uniform sampler2D input1Tex;
layout (binding = 3, rgba16f) uniform image2D output0Tex;

layout (local_size_x = ADD_THREADS_X, local_size_y = ADD_THREADS_Y) in;
void main()
{
  imageStore(output0Tex, ivec2(gl_GlobalInvocationID.xy),
    0.5 * (texelFetch(input0Tex, ivec2(gl_GlobalInvocationID.xy), uboPerInstance.mipLevel.x) 
      + textureLod(input1Tex, vec2((gl_GlobalInvocationID.xy + 0.5) / uboPerInstance.dim.xy), uboPerInstance.mipLevel.y)));
}
