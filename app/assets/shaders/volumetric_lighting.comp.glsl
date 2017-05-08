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
#include "lib_lighting.glsl"
#include "lib_vol_lighting.glsl"
#include "lib_noise.glsl"

layout (binding = 0) uniform PerInstance
{
  mat4 projMatrix;
  mat4 viewProjMatrix;
  mat4 prevViewProjMatrix;
  mat4 viewMatrix;
  mat4 invViewMatrix;

  vec4 eyeVSVectorX;
  vec4 eyeVSVectorY;
  vec4 eyeVSVectorZ;

  vec4 eyeWSVectorX;
  vec4 eyeWSVectorY;
  vec4 eyeWSVectorZ;

  vec4 data0;

  vec4 camPos;

  mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT];

  vec4 nearFar;
  vec4 nearFarWidthHeight;
} uboPerInstance; 

layout (binding = 1) uniform sampler2DArrayShadow shadowBufferTex;
layout (binding = 2, rgba16f) uniform image3D output0Tex;
layout (binding = 3) uniform sampler3D prevVolLightBufferTex;
layout (binding = 4) buffer LightBuffer
{
  Light lights[];
};
layout (binding = 5) buffer LightIndexBuffer
{
  uint lightIndices[];
};
 
// Based on AC4 volumetric fog
// https://goo.gl/xEgT9O
layout (local_size_x = 4u, local_size_y = 4u, local_size_z = 4u) in; 
void main()
{
  const float sunDensity = 40.0; 
  const vec3 fogSunColor = 50.0 * vec3(1.0, 0.9, 0.8); 
  const vec3 fogSkyColor = 40.0 * vec3(0.8, 1.0, 1.0);
  const float minShadowAttenuation = 0.05;
  const float heightAttenuationFactor = 0.025;
  const float scatteringFactor = uboPerInstance.data0.x;
  float reprojWeight = 0.95; 

  // ->

  const vec3 cellIndex = vec3(gl_GlobalInvocationID.xyz);

  const vec3 posSS = cellIndexToScreenSpacePos(cellIndex);
  const float linDepth = volumeZToDepth(posSS.z);
  const float layerThick = volumeZToDepth(posSS.z + 1.0 / VOLUME_DIMENSIONS.z) - linDepth;

  // Temporal reprojection
  const vec3 posSSReproj = cellIndexToScreenSpacePos(vec3(gl_GlobalInvocationID.xyz));
  const vec3 posWSReproj = uboPerInstance.camPos.xyz + screenToViewSpacePos(posSSReproj.xy, 
    uboPerInstance.eyeWSVectorX.xyz, uboPerInstance.eyeWSVectorY.xyz, uboPerInstance.eyeWSVectorZ.xyz,
     linDepth, uboPerInstance.projMatrix);
  vec4 posSSPrevFrame = uboPerInstance.prevViewProjMatrix * vec4(posWSReproj, 1.0);
  reprojWeight *= posSSPrevFrame.w > 0.0 ? 1.0 : 0.0;
  posSSPrevFrame.xy /= posSSPrevFrame.ww;
  posSSPrevFrame.xy = posSSPrevFrame.xy * 0.5 + 0.5;
  posSSPrevFrame.xyz = vec3(posSSPrevFrame.xy, depthToVolumeZ(posSSPrevFrame.z));
  const vec4 reprojFog = textureLod(prevVolLightBufferTex, screenSpacePosToCellIndex(posSSPrevFrame.xyz), 0.0);

  const vec3 posVS = screenToViewSpacePos(posSS.xy, 
    uboPerInstance.eyeVSVectorX.xyz, uboPerInstance.eyeVSVectorY.xyz, uboPerInstance.eyeVSVectorZ.xyz,
    linDepth, uboPerInstance.projMatrix);
  const vec3 posWS = uboPerInstance.camPos.xyz + screenToViewSpacePos(posSS.xy, 
    uboPerInstance.eyeWSVectorX.xyz, uboPerInstance.eyeWSVectorY.xyz, uboPerInstance.eyeWSVectorZ.xyz,
    linDepth, uboPerInstance.projMatrix);
  const vec3 rayWS = normalize(posWS - uboPerInstance.camPos.xyz);

  const vec3 lightVecWS = vec3(1.0, 0.45, -0.15);
  const vec3 heightRefPosWS = vec3(0.0, 700.0, 0.0);

  const float sunAmount = max(dot(rayWS, lightVecWS), 0.0);

  vec4 posLS;
  uint shadowMapIdx = findBestFittingSplit(posVS.xyz, posLS, uboPerInstance.shadowViewProjMatrix);
  float shadowAttenuation = 1.0; 

  if (shadowMapIdx != uint(-1)) 
  {
    //shadowAttenuation = witnessPCF(posLS.xyz, shadowMapIdx, shadowBufferTex);
    shadowAttenuation = texture(shadowBufferTex, vec4(posLS.xy, shadowMapIdx, posLS.z));
  }

  shadowAttenuation = max(minShadowAttenuation, shadowAttenuation);
  const float heightAttenuation = 1.0 - clamp(exp(-(heightRefPosWS.y - posWS.y) * heightAttenuationFactor), 0.0, 1.0);
  const vec3 lightColor = mix(fogSkyColor, fogSunColor, pow(sunAmount, sunDensity));
  const float scattering = scatteringFactor * layerThick;

  vec4 accumFog = vec4(0.0);
  float noiseAccum = 1.0;

  const uvec3 gridPos = calcGridPosForViewPos(posVS, uboPerInstance.nearFar, uboPerInstance.nearFarWidthHeight);
  const uint clusterIdx = calcClusterIndex(gridPos);

  uint lightCount = lightIndices[clusterIdx];

  for (uint li=0; li<lightCount; ++li)
  {
    Light light = lights[lightIndices[clusterIdx + li + 1]];

    const vec3 lightDistVec = light.posAndRadius.xyz - posVS;
    const float dist = length(lightDistVec);
    const float att = calcInverseSqrFalloff(light.posAndRadius.w, dist);

    // FIX ME
    accumFog += vec4(att * 50.0 * scattering * light.color.rgb, att * scattering);
  }

  // Noise
  noiseAccum *= noise(posWS * 0.25 + uboPerInstance.eyeWSVectorX.w * 1.0);
  noiseAccum *= noise(posWS * 0.15 + uboPerInstance.eyeWSVectorX.w * 0.75 + 0.382871);

  // Sky light
  accumFog += vec4(noiseAccum * scattering * heightAttenuation * shadowAttenuation * lightColor, scattering);
  
  const vec4 finalFog = mix(accumFog, reprojFog, reprojWeight);
  imageStore(output0Tex, ivec3(cellIndex), finalFog);
}    
 