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

#include "lib_math.glsl"
#include "lib_vol_lighting.glsl"

layout (binding = 0) uniform PerInstance
{
  float _dummy;
} uboPerInstance; 

layout (binding = 1) uniform sampler3D volLightBufferTex;
layout (binding = 2, rgba16f) uniform image3D volLightScatterBufferTex;

// Based on AC4 volumetric fog
// https://goo.gl/xEgT9O
void write(ivec3 cellIdx, vec4 accum)
{
  const vec4 result = vec4(accum.rgb, clamp(exp(-accum.a), 0.0, 1.0));
  imageStore(volLightScatterBufferTex, cellIdx, result);
}

vec4 accum(vec4 prev, vec4 next)
{  
  const vec3 l = prev.rgb + clamp(exp(-prev.a), 0.0, 1.0) * next.rgb;
  return vec4(l.rgb, prev.a + next.a);
}

layout (local_size_x = 8u, local_size_y = 8u, local_size_z = 1u) in;
void main()
{
  vec4 currentValue = texelFetch(volLightBufferTex, ivec3(gl_GlobalInvocationID.xy, 0), 0);
  write(ivec3(gl_GlobalInvocationID.xy, 0), currentValue);

  for (int z=1; z<VOLUME_DEPTH; ++z)
  {
    const ivec3 cellIdx = ivec3(gl_GlobalInvocationID.xy, z);
    const vec4 nextValue = texelFetch(volLightBufferTex, cellIdx, 0);
    currentValue = accum(currentValue, nextValue);
    write(cellIdx, currentValue);
  }
} 
    