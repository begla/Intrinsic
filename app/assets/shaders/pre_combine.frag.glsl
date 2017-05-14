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
#include "lib_buffers.glsl"
#include "lib_vol_lighting.glsl"
#include "ubos.inc.glsl"

PER_INSTANCE_DATA_PRE_COMBINE;

layout (binding = 1) uniform sampler2D albedoTex;
layout (binding = 2) uniform sampler2D normalTex;
layout (binding = 3) uniform sampler2D param0Tex;
layout (binding = 4) uniform sampler2D albedoTranspTex;
layout (binding = 5) uniform sampler2D normTranspTex;
layout (binding = 6) uniform sampler2D param0TranspTex;
layout (binding = 7) uniform sampler2D lightBufferTex;
layout (binding = 8) uniform sampler2D lightBufferTranspTex;
layout (binding = 9) uniform sampler2D depthBufferTex;
layout (binding = 10) uniform sampler2D depthBufferTranspTex;
layout (binding = 11) MATERIAL_BUFFER;

layout (binding = 12) uniform sampler3D volLightScatteringBufferTex;

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

const float fogSunDensity = 12.0;
const float fogDensity = 0.0004; 
const float fogStart = 1000.0;
const float fogMaxBlendFactor = 0.95;
const vec3 fogSunColor = vec3(1.0, 0.9, 0.8); 
const vec3 fogSkyColor = vec3(0.2, 0.5, 1.0);

const float waterFogDensity = 20.0;
const float waterFogMaxBlendFactor = 0.8;
const vec3 waterFogColor0 = vec3(0.2, 0.5, 1.0);
const vec3 waterFogColor1 = vec3(0.0, 0.0, 0.0);

void main()
{
  outColor = vec4(0.0, 0.0, 0.0, 1.0);

  const float opaqueDepth = textureLod(depthBufferTex, inUV0, 0.0).r;
  float fogDepth = opaqueDepth.r;

  const vec4 albedoTransparents = textureLod(albedoTranspTex, inUV0, 0.0).rgba;
  const vec3 albedo = textureLod(albedoTex, inUV0, 0.0).rgb;
  
  const vec4 param0 = textureLod(param0Tex, inUV0, 0.0).rgba;
  const uint matBufferEntryIdx = uint(param0.y);
  const MaterialParameters matParams = materialParameters[matBufferEntryIdx];

  const vec3 lighting = textureLod(lightBufferTex, inUV0, 0.0).rgb;

  if (albedoTransparents.a > 0.0) 
  {
    const vec3 normTranspVS = decodeNormal(textureLod(normTranspTex, inUV0, 0.0).rg);

    const vec4 param0Transp = textureLod(param0TranspTex, inUV0, 0.0).rgba;
    const uint matBufferEntryIdxTransp = uint(param0Transp.y);
    const MaterialParameters matParamsTransp = materialParameters[matBufferEntryIdxTransp];

    const float depthTransp = textureLod(depthBufferTranspTex, inUV0, 0.0).r;
    fogDepth = min(fogDepth, depthTransp.r);
    const float depthLinTransp = linearizeDepth(depthTransp, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

    // Refraction
    const float distStrength = matParamsTransp.refractionFactor;

    const vec2 distortedUV = inUV0 + normTranspVS.xy * (distStrength / -depthLinTransp);
    const vec3 albedoDistored = textureLod(albedoTex, distortedUV, 0.0).rgb;
    const float depthDistorted = textureLod(depthBufferTex, distortedUV, 0.0).r;
    const vec3 lightingTransp = textureLod(lightBufferTranspTex, inUV0, 0.0).rgb;
    const vec3 lightingDistored = textureLod(lightBufferTex, distortedUV, 0.0).rgb;

    vec3 opaque = albedoDistored * lightingDistored;
    float waterFogDepth = depthDistorted;

    // Only apply refraction if the opaque object is actually behind the transp. surface
    if (depthDistorted.r < depthTransp.r)
    {
      opaque = albedo * lighting;
      waterFogDepth = opaqueDepth;
    }

    // Water fog
    const float linDepthOpaque = linearizeDepth(waterFogDepth, uboPerInstance.camParams.x, uboPerInstance.camParams.y);
    const float linDepthTransp = linearizeDepth(depthTransp, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

    const float waterFog = min(1.0 - exp(-(linDepthOpaque - linDepthTransp) * waterFogDensity), waterFogMaxBlendFactor);
    const float waterFogDecay = clamp(pow(waterFog, 4.0), 0.0, 1.0);

    opaque.rgb = mix(opaque.rgb, mix(waterFogColor0, waterFogColor1, waterFogDecay), albedoTransparents.a * waterFog);
    const vec3 transp = albedoTransparents.rgb * lightingTransp;
    outColor.rgb = mix(opaque, transp, albedoTransparents.a);
  }
  else
  {
    outColor.rgb = albedo * lighting;
  }

  vec4 fog = vec4(vec3(0.0), 1.0);

  // Distance fog
  if (fogDepth < 1.0)
  {
    const vec3 posWS = unproject(inUV0, fogDepth, uboPerInstance.invViewProjMatrix);
    const vec3 ray = posWS - uboPerInstance.camPosition.xyz;
    const float rayDist = length(ray);
    const vec3 rayDir = ray / rayDist;

    const vec3 lightVec = vec3(1.0, 0.45, -0.15);
    const float sunAmount = max(dot(rayDir, lightVec), 0.0);
    fog.a = 1.0 - min(max((1.0 - exp((-rayDist + fogStart) * fogDensity)), 0.0), fogMaxBlendFactor);
    fog.rgb = (1.0 - fog.a) * mix(fogSkyColor, fogSunColor, pow(sunAmount, fogSunDensity));
  }

  // Volumetrics
  {
    const vec3 volLightingCoord = screenSpacePosToCellIndex(vec3(inUV0, 
      depthToVolumeZ(depthToVSDepth(fogDepth, uboPerInstance.camParams.x, uboPerInstance.camParams.y))));
    fog += textureLod(volLightScatteringBufferTex, volLightingCoord, 0.0);
  }

  outColor.rgb = outColor.rgb * fog.aaa + fog.rgb;
}
