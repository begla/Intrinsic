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
#include "lib_lighting.glsl"

layout (binding = 0) uniform PerInstance
{
  uvec4 arrayIdx;
} uboPerInstance;

layout (binding = 1) uniform sampler2DArray inputTex;

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

void main()
{
  vec4 depth = textureGather(inputTex, vec3(inUV0, uboPerInstance.arrayIdx), 0);
    
  vec2 warpedDepth[4];
  warpedDepth[0] = warpDepth(depth.x);
  warpedDepth[1] = warpDepth(depth.y);
  warpedDepth[2] = warpDepth(depth.z);
  warpedDepth[3] = warpDepth(depth.w);
  
  vec4 outputEVSM[4];
  outputEVSM[0] = vec4(warpedDepth[0], warpedDepth[0] * warpedDepth[0]);
  outputEVSM[1] = vec4(warpedDepth[1], warpedDepth[1] * warpedDepth[1]);
  outputEVSM[2] = vec4(warpedDepth[2], warpedDepth[2] * warpedDepth[2]);
  outputEVSM[3] = vec4(warpedDepth[3], warpedDepth[3] * warpedDepth[3]);

  vec4 result = outputEVSM[0] + outputEVSM[1] + outputEVSM[2] + outputEVSM[3];
  outColor = result * 0.25;
}
