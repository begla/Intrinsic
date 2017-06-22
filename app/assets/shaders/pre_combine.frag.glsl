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
#include "lib_buffers.glsl"
#include "lib_vol_lighting.glsl"
#include "ubos.inc.glsl"
#include "lib_lighting.glsl"

PER_INSTANCE_DATA_PRE_COMBINE;

layout(binding = 1) uniform sampler2D albedoTex;
layout(binding = 2) uniform sampler2D normalTex;
layout(binding = 3) uniform sampler2D param0Tex;
layout(binding = 4) uniform sampler2D albedoTranspTex;
layout(binding = 5) uniform sampler2D normTranspTex;
layout(binding = 6) uniform sampler2D param0TranspTex;
layout(binding = 7) uniform sampler2D lightBufferTex;
layout(binding = 8) uniform sampler2D lightBufferTranspTex;
layout(binding = 9) uniform sampler2D depthBufferTex;
layout(binding = 10) uniform sampler2D depthBufferTranspTex;
layout(binding = 11) MATERIAL_BUFFER;
layout(binding = 12) uniform sampler3D volLightScatteringBufferTex;

PER_FRAME_DATA(13);

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

const float fogDensity = 0.001;
const float fogStart = 1200.0;
const float fogSunScatteringIntens = 0.0;

const float waterFogDensity = 60.0;
const float waterFogMaxBlendFactor = 1.0;
const float waterFogDecayExp = 16.0;

const vec4 waterFogColor0 = vec4(0.0, 1.0, 1.0, 1.0);
const vec4 waterFogColor1 = vec4(0.0, 0.2, 0.2, 1.0);

void main()
{
  outColor = vec4(0.0, 0.0, 0.0, 1.0);

  const float opaqueDepth = textureLod(depthBufferTex, inUV0, 0.0).r;
  float fogDepth = opaqueDepth.r;

  vec4 albedoTransparents = textureLod(albedoTranspTex, inUV0, 0.0).rgba;

  const vec4 param0 = textureLod(param0Tex, inUV0, 0.0).rgba;
  const uint matBufferEntryIdx = uint(param0.y);
  const MaterialParameters matParams = materialParameters[matBufferEntryIdx];

  const vec3 lighting = textureLod(lightBufferTex, inUV0, 0.0).rgb;

  if (albedoTransparents.a > EPSILON)
  {
    const vec3 normTranspVS =
        decodeNormal(textureLod(normTranspTex, inUV0, 0.0).rg);
    const float depthTransp = textureLod(depthBufferTranspTex, inUV0, 0.0).r;

    // Fresnel
    const vec3 posVS =
        unproject(inUV0, depthTransp, uboPerInstance.invProjMatrix);
    const vec3 V = -normalize(posVS);
    const float F =
        F_Schlick(albedoTransparents.a, clamp(dot(V, normTranspVS), 0.0, 1.0));

    const vec4 param0Transp = textureLod(param0TranspTex, inUV0, 0.0).rgba;
    const uint matBufferEntryIdxTransp = uint(param0Transp.y);
    const MaterialParameters matParamsTransp =
        materialParameters[matBufferEntryIdxTransp];

    fogDepth = min(fogDepth, depthTransp.r);
    const float depthLinTransp = linearizeDepth(
        depthTransp, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

    // Refraction
    const float distStrength =
        albedoTransparents.a * matParamsTransp.refractionFactor;

    const vec2 distortedUV =
        inUV0 + normTranspVS.xy * (distStrength / -depthLinTransp);
    const float depthDistorted = textureLod(depthBufferTex, distortedUV, 0.0).r;
    const vec3 lightingTransp =
        textureLod(lightBufferTranspTex, inUV0, 0.0).rgb;
    const vec3 lightingDistored =
        textureLod(lightBufferTex, distortedUV, 0.0).rgb;

    vec3 opaque = lightingDistored;
    float waterFogDepth = depthDistorted;

    // Only apply refraction if the opaque object is actually behind the transp.
    // surface
    if (depthDistorted.r < depthTransp.r)
    {
      opaque = lighting;
      waterFogDepth = opaqueDepth;
    }

    // Water fog
    const vec3 normTranspWS =
        (uboPerInstance.invViewMatrix * vec4(normTranspVS.xyz, 0.0)).xyz;
    const vec3 fogIrrad =
        sampleSH(uboPerFrame.skyLightSH, normTranspWS) / MATH_PI;

    const float linDepthOpaque = linearizeDepth(
        waterFogDepth, uboPerInstance.camParams.x, uboPerInstance.camParams.y);
    const float linDepthTransp = linearizeDepth(
        depthTransp, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

    const float waterFog =
        1.0 - exp(-(linDepthOpaque - linDepthTransp) * waterFogDensity);
    const vec4 waterFogColor =
        mix(waterFogColor0, waterFogColor1, F * 0.5 + 0.5);

    opaque.rgb = mix(opaque.rgb, fogIrrad * waterFogColor.rgb, waterFog);
    outColor.rgb = mix(opaque, lightingTransp, albedoTransparents.a);
  }
  else
  {
    outColor.rgb = lighting;
  }

  vec4 fog = vec4(vec3(0.0), 1.0);

  // Distance fog
  if (fogDepth < 1.0)
  {
    const vec3 posWS =
        unproject(inUV0, fogDepth, uboPerInstance.invViewProjMatrix);
    const vec3 ray = posWS - uboPerInstance.camPosition.xyz;
    const float rayDist = length(ray);
    const vec3 rayDir = ray / rayDist;

    // Generate distance fog as a mixture of skylight irradiance and sunlight
    // contribution
    const float sunAmount =
        fogSunScatteringIntens *
        max(dot(rayDir, uboPerFrame.sunLightDirWS.xyz), 0.0);
    vec3 fogColor = sampleSH(uboPerFrame.skyLightSH, rayDir) / MATH_PI;
    fogColor += fogColor * uboPerFrame.sunLightColorAndIntensity.xyz *
                uboPerFrame.sunLightColorAndIntensity.w * pow(sunAmount, 8.0);

    fog.a = clamp(exp((-rayDist + fogStart) * fogDensity), 0.0, 1.0);
    fog.rgb = (1.0 - fog.a) * fogColor;
  }

  // Volumetrics
  {
    const vec3 volLightingCoord = screenSpacePosToCellIndex(
        vec3(inUV0,
             depthToVolumeZ(depthToVSDepth(fogDepth, uboPerInstance.camParams.x,
                                           uboPerInstance.camParams.y))));
    fog += textureLod(volLightScatteringBufferTex, volLightingCoord, 0.0);
  }

  outColor.rgb = outColor.rgb * fog.aaa + fog.rgb;
}
