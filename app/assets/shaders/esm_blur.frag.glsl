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

#define KERNEL_RADIUS uboPerInstance.blurParams.x

layout(binding = 0) uniform PerInstance
{
  vec4 blurParams;
  uvec4 arrayIdx;
}
uboPerInstance;

layout(binding = 1) uniform sampler2DArray inputTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

vec4 blur(vec2 uv, inout float w_total)
{
  w_total += 1.0;
  return texture(inputTex, vec3(uv, uboPerInstance.arrayIdx.x));
}

void main()
{
  vec4 center_c = texture(inputTex, vec3(inUV0, uboPerInstance.arrayIdx.x));

  vec2 resDir = textureSize(inputTex, 0).xy;
  resDir = 1.0 / resDir * uboPerInstance.blurParams.zw;

  vec4 c_total = center_c;
  float w_total = 1.0;

  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = inUV0 + resDir * r;
    c_total += blur(uv, w_total);
  }

  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = inUV0 - resDir * r;
    c_total += blur(uv, w_total);
  }

  outColor = c_total / w_total;
}
