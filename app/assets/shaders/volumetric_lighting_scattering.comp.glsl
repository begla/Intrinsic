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
layout (binding = 2, r11f_g11f_b10f) uniform image3D volLightScatterBufferTex;

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
  vec4 currentValue = texelFetch(volLightBufferTex, ivec3(gl_GlobalInvocationID.xy, 0), 0) * 0.0001;
  write(ivec3(gl_GlobalInvocationID.xy, 0), currentValue);

  for (int z=1; z<VOLUME_DEPTH; ++z)
  {
    const ivec3 cellIdx = ivec3(gl_GlobalInvocationID.xy, z);
    const vec4 nextValue = texelFetch(volLightBufferTex, cellIdx, 0);
    currentValue = accum(currentValue, nextValue);
    write(cellIdx, currentValue);
  }
} 
    