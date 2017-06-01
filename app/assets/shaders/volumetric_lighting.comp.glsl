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
#include "ubos.inc.glsl"

layout (binding = 0) uniform PerInstance
{
  mat4 projMatrix;
  mat4 prevViewProjMatrix;

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

  vec4 haltonSamples;
} uboPerInstance;

PER_FRAME_DATA(1);

layout (binding = 2) uniform sampler2DArrayShadow shadowBufferTex;
layout (binding = 3, r11f_g11f_b10f) uniform image3D output0Tex;
layout (binding = 4) uniform sampler3D prevVolLightBufferTex;
layout (binding = 5) buffer LightBuffer
{
  Light lights[];
};
layout (binding = 6) buffer LightIndexBuffer
{
  uint lightIndices[];
};
layout (binding = 7) buffer IrradProbeBuffer
{
  IrradProbe irradProbes[];
};
layout (binding = 8) buffer IrradProbeIndexBuffer
{
  uint irradProbeIndices[];
};
layout (binding = 9) uniform sampler2DArray shadowBufferExpTex;
layout (binding = 10) uniform sampler2D kelvinLutTex;

// TODO
const vec3 heightRefPosWS = vec3(0.0, 700.0, 0.0);
const float heightAttenuationFactor = 0.025;
 
// Based on AC4 volumetric fog
layout (local_size_x = 4u, local_size_y = 4u, local_size_z = 4u) in; 
void main()
{
  const float densityFactor = uboPerInstance.data0.x;
  vec3 cellIndex = vec3(gl_GlobalInvocationID.xyz) + 0.5;
  const float localLightIntens = uboPerInstance.data0.y;

  // Temporal reprojection
  float reprojWeight = 0.85; 

  const vec3 posSSCenter = cellIndexToScreenSpacePos(cellIndex);
  const float linDepthCenter = volumeZToDepth(posSSCenter.z);
  const vec3 posWSCenter = uboPerInstance.camPos.xyz + screenToViewSpacePos(posSSCenter.xy, 
    uboPerInstance.eyeWSVectorX.xyz, uboPerInstance.eyeWSVectorY.xyz, uboPerInstance.eyeWSVectorZ.xyz,
    linDepthCenter, uboPerInstance.projMatrix);
  vec4 posSSPrevFrame = uboPerInstance.prevViewProjMatrix * vec4(posWSCenter, 1.0);
  reprojWeight *= posSSPrevFrame.z > 1.0 ? 1.0 : 0.0;
  posSSPrevFrame.xy /= posSSPrevFrame.ww;
  posSSPrevFrame.xy = posSSPrevFrame.xy * 0.5 + 0.5;
  posSSPrevFrame.xyz = vec3(posSSPrevFrame.xy, depthToVolumeZ(posSSPrevFrame.z));
  const vec4 reprojFog = textureLod(prevVolLightBufferTex, screenSpacePosToCellIndex(posSSPrevFrame.xyz), 0.0);

  // Temporal super-sampling
  const vec3 offset = (uboPerInstance.haltonSamples.xyz * 2.0 - 1.0) * 0.5;
  cellIndex += offset.xyz * vec3(1.0, 1.0, 1.0);

  const vec3 posSS = cellIndexToScreenSpacePos(cellIndex);
  const float linDepth = volumeZToDepth(posSS.z);
  const float layerThick = volumeZToDepth(posSS.z + 1.0 / VOLUME_DIMENSIONS.z) - linDepth;
  const vec3 posVS = screenToViewSpacePos(posSS.xy, 
    uboPerInstance.eyeVSVectorX.xyz, uboPerInstance.eyeVSVectorY.xyz, uboPerInstance.eyeVSVectorZ.xyz,
    linDepth, uboPerInstance.projMatrix);
  const vec3 posWS = uboPerInstance.camPos.xyz + screenToViewSpacePos(posSS.xy, 
    uboPerInstance.eyeWSVectorX.xyz, uboPerInstance.eyeWSVectorY.xyz, uboPerInstance.eyeWSVectorZ.xyz,
    linDepth, uboPerInstance.projMatrix);
  const vec3 rayWS = normalize(posWS - uboPerInstance.camPos.xyz); 

  // Calc. density
  float density = densityFactor * layerThick;
  vec3 finalHeightPos = heightRefPosWS;

  // Noise
#if 1
  float noiseAccum = 1.0;
  noiseAccum *= noise(posWS * clamp(5.0 / linDepth, 0.005, 0.08) + uboPerInstance.eyeWSVectorX.w * 0.5);
  noiseAccum *= noise(posWS * clamp(10.0 / linDepth, 0.005, 0.08) + uboPerInstance.eyeWSVectorX.w * 0.75 + 0.382871);
  density *= clamp(noiseAccum, 0.0, 1.0);
  //finalHeightPos.y -= noise(posWS * clamp(10.0 / linDepth, 0.005, 0.08) + uboPerInstance.eyeWSVectorX.w * 0.75 + 0.75827618) * 50.0;
#endif

  const float heightAttenuation = 1.0 - clamp(exp(-(finalHeightPos.y - posWS.y) * heightAttenuationFactor), 0.0, 1.0);
  density *= heightAttenuation;  

  // Lighting
  vec3 lighting = vec3(0.0);

  if (density > EPSILON)
  {
    // Sunlight
    vec4 posLS;
    uint shadowMapIdx = findBestFittingSplit(posVS.xyz, posLS, uboPerInstance.shadowViewProjMatrix);

    float shadowAttenuation = 1.0; 

    if (shadowMapIdx != uint(-1)) 
    {
      const vec4 shadowSample = texture(shadowBufferExpTex, vec3(posLS.xy, shadowMapIdx));
      shadowAttenuation = clamp(calculateShadowESM(shadowSample, posLS.z)*1.1 - 0.1, 0.0, 1.0);
    }

    // TODO: Hardcoded light color
    const vec3 lightColor = vec3(1.0, 0.4, 0.2) 
      * sampleSH(uboPerFrame.skyLightSH, uboPerFrame.skyLightDirWS.xyz) / MATH_PI;
    lighting += shadowAttenuation * lightColor;
  }

  const uvec3 gridPos = calcGridPosForViewPos(posVS, uboPerInstance.nearFar, uboPerInstance.nearFarWidthHeight);
  const uint clusterIdx = calcClusterIndex(gridPos);
  const bool gridPosValid = isGridPosValid(gridPos);

  // Sky light
  vec3 irrad = sampleSH(uboPerFrame.skyLightSH, rayWS) / MATH_PI;

  // Local irradiance
  if (gridPosValid
    && density > EPSILON)
  {
    const uint irradProbeCount = irradProbeIndices[clusterIdx];

    for (uint pi=0; pi<irradProbeCount; ++pi)
    {
      IrradProbe probe = irradProbes[irradProbeIndices[clusterIdx + pi + 1]];

      const float distToProbe = distance(posVS, probe.posAndRadius.xyz);
      if (distToProbe < probe.posAndRadius.w)
      {
        const float fadeRange = probe.posAndRadius.w * probe.data0.x;
        const float fadeStart = probe.posAndRadius.w - fadeRange;
        const float fade = pow(1.0 - max(distToProbe - fadeStart, 0.0) 
          / fadeRange, probe.data0.y);
        
        irrad = mix(irrad, sampleSH(probe.data, rayWS) / MATH_PI, fade);
      }
    }
  }

  lighting += irrad * uboPerInstance.data0.z;

  // Local lights
  if (gridPosValid
    && density > EPSILON)
  {
    uint lightCount = lightIndices[clusterIdx];
    for (uint li=0; li<lightCount; ++li)
    {
      Light light = lights[lightIndices[clusterIdx + li + 1]];

      const vec3 lightDistVec = light.posAndRadius.xyz - posVS;
      const float dist = length(lightDistVec);
      const float att = calcInverseSqrFalloff(light.posAndRadius.w, dist);
      if (att * light.colorAndIntensity.w < MIN_FALLOFF) continue;

      lighting += att * light.colorAndIntensity.rgb 
        * light.colorAndIntensity.a * kelvinToRGB(light.temp.r, kelvinLutTex) / MATH_PI;
    }   
  }

  const vec4 fog = vec4(density * lighting, density);
  imageStore(output0Tex, ivec3(cellIndex), mix(fog, reprojFog, reprojWeight));
}    
