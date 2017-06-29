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
#include "lib_noise.glsl"
#include "lib_lighting.glsl"
#include "lib_buffers.glsl"
#include "lib_clustering.glsl"
#include "ubos.inc.glsl"

layout(binding = 0) uniform PerInstance
{
  mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT];

  vec4 nearFarWidthHeight;
  vec4 nearFar;

  vec4 data0;
}
uboPerInstance;

PER_FRAME_DATA(1);

layout(set = 1, binding = 1) uniform samplerCube globalCubeTextures[4095];

layout(binding = 2) uniform sampler2D albedoTex;
layout(binding = 3) uniform sampler2D normalTex;
layout(binding = 4) uniform sampler2D parameter0Tex;
layout(binding = 5) uniform sampler2D depthTex;
layout(binding = 6) uniform sampler2D ssaoTex;
layout(binding = 7) uniform sampler2DArrayShadow shadowBufferTex;
layout(binding = 8) MATERIAL_BUFFER;
layout(std430, binding = 9) buffer readonly LightBuffer { Light lights[]; };
layout(std430, binding = 10) buffer readonly LightIndexBuffer
{
  uint lightIndices[];
};
layout(std430, binding = 11) buffer readonly IrradProbeBuffer
{
  IrradProbe irradProbes[];
};
layout(std430, binding = 12) buffer readonly IrradProbeIndexBuffer
{
  uint irradProbeIndices[];
};
layout(std430, binding = 13) buffer readonly SpecProbeBuffer
{
  SpecProbe specProbes[];
};
layout(std430, binding = 14) buffer readonly SpecProbeIndexBuffer
{
  uint specProbeIndices[];
};
layout(binding = 15) uniform sampler2D kelvinLutTex;
layout(binding = 16) uniform sampler2D noiseTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

#include "lighting.inc.glsl"

void main()
{
  const vec4 albedoSample = textureLod(albedoTex, inUV0, 0.0);
  const float depth = textureLod(depthTex, inUV0, 0.0).x;

  // Pass through sky
  if (depth >= 1.0)
  {
    outColor.rgba = albedoSample;
    return;
  }

  const vec4 parameter0Sample = textureLod(parameter0Tex, inUV0, 0.0);
  const MaterialParameters matParams =
      materialParameters[uint(parameter0Sample.y)];
  const vec4 normalSample = textureLod(normalTex, inUV0, 0.0);

  // Unlit materials
  if ((matParams.materialFlags & 1) > 0)
  {
    const vec3 N = normalize(decodeNormal(normalSample.rg));
    const vec3 L =
        (uboPerFrame.viewMatrix * vec4(normalize(vec3(0.25, 0.5, 0.25)), 0.0))
            .xyz;
    const float fakeShading =
        clamp(dot(L, N), 0.0, 1.0) + clamp(dot(-L, N), 0.0, 1.0) * 0.25;

    outColor.rgba = albedoSample * clamp(fakeShading + 0.1, 0.0, 1.0);
    return;
  }

  outColor.rgba = vec4(0.0);

  const vec4 ssaoSample = textureLod(ssaoTex, inUV0, 0.0);
  const float occlusion = parameter0Sample.z;
  const float emissive = parameter0Sample.w;

  // Setup lighting data for sunlight
  LightingData d;
  {
    d.baseColor = albedoSample.rgb;
    d.metalMask = parameter0Sample.r;
    d.specular = normalSample.b;
    d.roughness = normalSample.a;

    initLightingDataFromGBuffer(d);
  }

  {
    d.posVS = unproject(inUV0, depth, uboPerFrame.invProjMatrix);
    d.N = normalize(decodeNormal(normalSample.rg));
    d.energy = uboPerFrame.sunLightColorAndIntensity.w;
    calculateLightingDataBase(d);

    d.L = uboPerFrame.sunLightDirVS.xyz;
    calculateDiscL(d, 0.025 * MATH_PI);
    calculateLobeEnergySphere(d, 0.025 * MATH_PI);
    calculateLightingData(d);
  }

  const uvec3 gridPos = calcGridPosForViewPos(
      d.posVS, uboPerInstance.nearFar, uboPerInstance.nearFarWidthHeight);

  // Ambient lighting
  const vec3 normalWS = (uboPerFrame.invViewMatrix * vec4(d.N, 0.0)).xyz;
  const vec3 vWS = (uboPerFrame.invViewMatrix * vec4(d.V, 0.0)).xyz;

  const vec3 posWS = (uboPerFrame.invViewMatrix * vec4(d.posVS, 1.0)).xyz;
  const vec3 camPosWS = (uboPerFrame.invViewMatrix * vec4(vec3(0.0), 1.0)).xyz;

  const vec3 camDirUnorm = posWS - camPosWS;
  const vec3 reflectDirUnormWS = reflect(camDirUnorm, normalWS);

  const vec3 R0 = 2.0 * dot(vWS, normalWS) * normalWS - vWS;
  vec3 R =
      mix(normalWS, R0,
          (1.0 - d.roughness2) * (sqrt(1.0 - d.roughness2) + d.roughness2));

  // Irradiance
  vec3 irrad =
      d.diffuseColor * sampleSH(uboPerFrame.skyLightSH, normalWS) / MATH_PI;
  {
    const uint clusterIdx =
        calcClusterIndex(gridPos, maxIrradProbeCountPerCluster) / 2;
    const uint irradProbeCount = irradProbeIndices[clusterIdx];

    for (uint pi = 0; pi < irradProbeCount; pi += 2)
    {
      const uint packedProbeIndices =
          irradProbeIndices[clusterIdx + pi / 2 + 1];

      IrradProbe probe = irradProbes[packedProbeIndices & 0xFFFF];
      calcLocalIrradiance(probe, d, normalWS, irrad, 1.0);

      probe = irradProbes[packedProbeIndices >> 16];
      calcLocalIrradiance(probe, d, normalWS, irrad,
                          float(pi + 1 < irradProbeCount));
    }
  }

  outColor.rgb += uboPerInstance.data0.y * min(ssaoSample.r, occlusion) * irrad;

  // Specular
  {
    vec3 spec = vec3(0.0f);

    const uint clusterIdx =
        calcClusterIndex(gridPos, maxSpecProbeCountPerCluster) / 2;
    const uint specProbeCount = specProbeIndices[clusterIdx];
    const float specMipIdx = roughnessToMipIdx(d.roughness);

//#define DEBUG_VIS_CLUSTERS
#if defined(DEBUG_VIS_CLUSTERS)
    if (specProbeCount > 0)
    {
      outColor = vec4(gridPos / vec3(gridRes) * specProbeCount / 64.0, 0.0);
      return;
    }
#endif // DEBUG_VIS_CLUSTERS

    for (uint pi = 0; pi < specProbeCount; pi += 2)
    {
      const uint packedProbeIndices = specProbeIndices[clusterIdx + pi / 2 + 1];

      SpecProbe probe = specProbes[packedProbeIndices & 0xFFFF];

      // Parallax correction
      /*{
        const vec3 boxMaxWS = vec3(10.0, 10000.0, 20.0);
        const vec3 boxMinWS = vec3(-10.0, 0.0, -20.0);
        const vec3 i0 = (boxMaxWS - posWS) / reflectDirUnormWS;
        const vec3 i1 = (boxMinWS - posWS) / reflectDirUnormWS;
        const vec3 iMax = max(i0, i1);
        const float dist = min(iMax.x, min(iMax.y, iMax.z));
        const vec3 iWS = posWS + reflectDirUnormWS * dist;

        const vec3 R0 = iWS - (uboPerFrame.invViewMatrix * vec4(probe.posAndRadius.xyz, 1.0)).xyz;
        R = mix(normalWS, R0,
          (1.0 - d.roughness2) * (sqrt(1.0 - d.roughness2) + d.roughness2));
      }*/

      calcLocalSpecular(probe, d, spec, R, specMipIdx, uboPerInstance.data0.w,
                        1.0);

      probe = specProbes[packedProbeIndices >> 16];
      calcLocalSpecular(probe, d, spec, R, specMipIdx, uboPerInstance.data0.w,
                        float(pi + 1 < specProbeCount));
    }

    outColor.rgb += d.specularColor * uboPerInstance.data0.z * spec;
  }

  // Emissive
  outColor.rgb += emissive * matParams.emissiveIntensity * d.baseColor;

  // Cloud shadows
  const float cloudShadows = mix(
      1.0, clamp(texture(noiseTex,
                         clamp(posWS.xz / 5000.0 * 0.5 + 0.5, 0.0, 1.0) * 5.0 +
                             uboPerInstance.data0.x * 0.025)
                             .r *
                         3.0 -
                     0.5,
                 0.1, 1.0),
      uboPerFrame.postParams0.w);

  // Sunlight
  {
    const vec4 sunLightColorAndIntensity =
        uboPerFrame.sunLightColorAndIntensity;
    float shadowAttenuation =
        cloudShadows *
        calcShadowAttenuation(d.posVS, uboPerInstance.shadowViewProjMatrix,
                              shadowBufferTex);
    outColor.rgb +=
        shadowAttenuation * sunLightColorAndIntensity.rgb * calcLighting(d);

    calcTransl(d, matParams, clamp(uboPerFrame.sunLightDirWS.y - 0.3, 0.0,
                                   1.0), // Dim for low sun
               sunLightColorAndIntensity,
               outColor);
  }

  // Point lights
  {
    const uint clusterIdx =
        calcClusterIndex(gridPos, maxLightCountPerCluster) / 2;
    const uint lightCount = lightIndices[clusterIdx];

    for (uint li = 0; li < lightCount; li += 2)
    {
      const uint packedLightIndices = lightIndices[clusterIdx + li / 2u + 1u];

      Light light = lights[packedLightIndices & 0xFFFF];
      calcPointLightLighting(light, d, matParams, outColor);

      light = lights[packedLightIndices >> 16];
      light.colorAndIntensity.w *= float(li + 1 < lightCount);
      calcPointLightLighting(light, d, matParams, outColor);
    }
  }
}
