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

/* __PREPROCESSOR DEFINES__ */

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_noise.glsl"
#include "gbuffer_vertex.inc.glsl"

out gl_PerVertex { vec4 gl_Position; };

// Ubos
PER_INSTANCE_UBO;

// Input
INPUT();

// Output
layout(location = 0) out vec2 outUV0;

void main()
{
  vec3 localPos = inPosition;
  const vec3 initialWorldPos =
      (uboPerInstance.worldMatrix * vec4(inPosition.xyz, 1.0)).xyz;
  const vec3 worldNormalUnorm =
      (uboPerInstance.worldMatrix * vec4(inNormal.xyz, 0.0)).xyz;
  const vec3 worldNormal = normalize(worldNormalUnorm);
  const vec2 windStrength = calcWindStrength(uboPerInstance.data0.w);
  const vec3 pivotWS = vec3(uboPerInstance.worldMatrix[3]);

#if defined(GRASS)
  applyGrassWind(localPos, initialWorldPos, uboPerInstance.data0.w,
                 windStrength);
#else
  applyTreeWind(localPos, initialWorldPos, pivotWS, worldNormal, inColor.r,
                uboPerInstance.data0.w, windStrength);
#endif // GRASS

  const vec3 worldPos =
      (uboPerInstance.worldMatrix * vec4(localPos.xyz, 1.0)).xyz -
      worldNormalUnorm.xyz * 0.03; // Shadow bias
  gl_Position = uboPerInstance.viewProjMatrix * vec4(worldPos, 1.0);

  outUV0 = inUV0;
}
