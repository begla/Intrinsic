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
#include "ubos.inc.glsl"

PER_INSTANCE_DATA_SSAO_TEMP_REPROJ;

layout(binding = 1) uniform sampler2D ssaoTex;
layout(binding = 2) uniform sampler2D ssaoPrevTex;
layout(binding = 3) uniform sampler2D depthBufferTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

void main()
{
  const float depth = texture(depthBufferTex, inUV0, 0.0).r;
  const float ao = texture(ssaoTex, inUV0, 0.0).r;

  const vec3 viewPos = unproject(inUV0, depth, uboPerInstance.invProjMatrix);
  const vec4 prevViewPos =
      uboPerInstance.prevViewMatrix *
      vec4(unproject(inUV0, depth, uboPerInstance.invViewProjMatrix), 1.0);
  vec4 prevUV = uboPerInstance.projMatrix * prevViewPos;
  prevUV /= prevUV.w;
  prevUV.xy = prevUV.xy * 0.5 + 0.5;

  const float prevAo = texture(ssaoPrevTex, prevUV.xy).r;
  const float prevDepth = texture(depthBufferTex, prevUV.xy).r;

  // Distance weight (ghosting)
  float weight = distance(prevViewPos.xyz, viewPos.xyz) * 8.0;
  // AO intensity weight (trailing)
  weight += abs(ao - prevAo) * 2.0;

  outColor = vec4(mix(prevAo, ao, clamp(weight, 0.1, 1.0)));
}
