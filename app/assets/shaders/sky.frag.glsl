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

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "gbuffer.inc.glsl"
#include "lib_lighting.glsl"
#include "ubos.inc.glsl"

// Ubos
PER_MATERIAL_UBO;
PER_INSTANCE_UBO;

PER_FRAME_DATA(3);

// Input
layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inPosVS;
layout (location = 3) in vec3 inUpVS;

// Output
layout (location = 0) out vec4 outAlbedo;

float getSkyModelConfig(uint channelIdx, uint configIdx)
{
  const uint idx = configIdx + channelIdx * 9u;
  const uvec2 packedDataOffset = uvec2(idx / 4u, idx % 4u);
  return uboPerFrame.skyModelConfigs[packedDataOffset.x][packedDataOffset.y];
}

/// Based on 'An Analytic Model for Full Spectral Sky-Dome Radiance'
/// ACM Transactions on Graphics (Proceedings of ACM SIGGRAPH 2012)
vec3 calculateSkyModelRadiance(
  vec3 theta, 
  vec3 gamma
)
{
  vec3 configuration[9];
  for (uint i=0; i<9; ++i)
    configuration[i] = vec3(
      getSkyModelConfig(0, i),
      getSkyModelConfig(1, i), 
      getSkyModelConfig(2, i)
    );
  
  const vec3 expM = exp(configuration[4] * gamma);
  const vec3 rayM = cos(gamma)*cos(gamma);
  const vec3 mieM = (vec3(1.0) + cos(gamma)*cos(gamma)) / pow((1.0 
    + configuration[8]*configuration[8] - 2.0*configuration[8]*cos(gamma)), vec3(1.5));
  const vec3 zenith = sqrt(cos(theta));

  return (vec3(1.0) + configuration[0] * exp(configuration[1] / (cos(theta) + vec3(0.01)))) *
          (configuration[2] + configuration[3] * expM + configuration[5] * rayM 
            + configuration[6] * mieM + configuration[7] * zenith);
}

void main()
{
  vec4 albedo = vec4(0.0);
  const vec3 V = normalize(inPosVS);

  const float theta = acos(max(dot(V, inUpVS), 0.0001));
  const float gamma = acos(max(dot(V, uboPerFrame.sunLightDirVS.xyz), 0.0001));

  // Apply sky model
  albedo.rgb += clamp(calculateSkyModelRadiance(vec3(theta), vec3(gamma)) 
  	* uboPerFrame.skyModelRadiances.rgb, 0.0, 100.0);
  //albedo.rgb += sampleSH(uboPerFrame.skyLightSH, inNormal);

  // Sun/Moon
  albedo.rgb += uboPerFrame.sunLightColorAndIntensity.xyz * uboPerFrame.sunLightColorAndIntensity.w 
    * pow(clamp(dot(uboPerFrame.sunLightDirVS.xyz, V), 0.0, 1.0), 1500.0);

  outAlbedo = vec4(albedo.rgb, 1.0);
}
