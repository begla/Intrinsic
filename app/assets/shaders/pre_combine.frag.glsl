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

layout (binding = 0) uniform PerInstance
{
  mat4 invProjMatrix;
  mat4 invViewProjMatrix;
  vec4 camPosition;
} uboPerInstance;

layout (binding = 1) uniform sampler2D albedoTex;
layout (binding = 2) uniform sampler2D normalTex;
layout (binding = 3) uniform sampler2D param0Tex;
layout (binding = 4) uniform sampler2D albedoTransparentsTex;
layout (binding = 5) uniform sampler2D normalTransparentsTex;
layout (binding = 6) uniform sampler2D param0TransparentsTex;
layout (binding = 7) uniform sampler2D lightBufferTex;
layout (binding = 8) uniform sampler2D lightBufferTransparentsTex;
layout (binding = 9) uniform sampler2D depthBufferTex;
layout (binding = 10) uniform sampler2D depthBufferTransparentsTex;
layout (binding = 11) MATERIAL_BUFFER;

layout (binding = 12) uniform sampler3D volumetricLightingScatteringBufferTex;

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

void main()
{
  const vec4 camParams = vec4(1.0, 10000.0, 1.0, 1.0 / 10000.0);

  outColor = vec4(0.0, 0.0, 0.0, 1.0);

  const float opaqueDepth = textureLod(depthBufferTex, inUV0, 0.0).r;
  float fogDepth = opaqueDepth.r;

  const vec4 albedoTransparents = textureLod(albedoTransparentsTex, inUV0, 0.0).rgba;
  const vec3 albedo = textureLod(albedoTex, inUV0, 0.0).rgb;
  
  const vec4 param0 = textureLod(param0Tex, inUV0, 0.0).rgba;
  const uint matBufferEntryIdx = uint(param0.y);
  const MaterialParameters matParams = materialParameters[matBufferEntryIdx];

  const vec3 lighting = textureLod(lightBufferTex, inUV0, 0.0).rgb;

  if (albedoTransparents.a < 1.0) 
  {
    const vec4 normalTransparents = textureLod(normalTransparentsTex, inUV0, 0.0).rgba;
    const vec3 nTranspVS = decodeNormal(normalTransparents.xy);

    const vec4 param0Transparents = textureLod(param0TransparentsTex, inUV0, 0.0).rgba;
    const uint matBufferEntryIdxTranp = uint(param0Transparents.y);
    const MaterialParameters transpMatParams = materialParameters[matBufferEntryIdxTranp];

    const float transparentDepth = textureLod(depthBufferTransparentsTex, inUV0, 0.0).r;
    fogDepth = min(fogDepth, transparentDepth.r);
    const float linTranspDepth = linearizeDepth(transparentDepth, camParams.x, camParams.y);

    // Refraction
    const float distStrength = transpMatParams.refractionFactor;

    const vec2 distortedUV = inUV0 + nTranspVS.xy * (distStrength / -linTranspDepth);
    const vec3 albedoDistored = textureLod(albedoTex, distortedUV, 0.0).rgb;
    const float opaqueDepthDistored = textureLod(depthBufferTex, distortedUV, 0.0).r;
    const vec3 lightingTransparents = textureLod(lightBufferTransparentsTex, inUV0, 0.0).rgb;
    const vec3 lightingDistored = textureLod(lightBufferTex, distortedUV, 0.0).rgb;

    vec3 opaque = albedoDistored * lightingDistored;
    float waterFogDepth = opaqueDepthDistored;

    // Only apply refraction if the opaque object is actually behind the transp. surface
    if (opaqueDepthDistored.r < transparentDepth.r)
    {
      opaque = albedo * lighting;
      waterFogDepth = opaqueDepth;
    }

    // Water fog
    const float waterFogDensity = 50.0;
    const float waterFogMaxBlendFactor = 1.0;
    const vec3 waterFogColor0 = vec3(0.2, 0.5, 1.0);
    const vec3 waterFogColor1 = vec3(0.0, 0.0, 0.0);

    const float linDepthOpaque = linearizeDepth(waterFogDepth, camParams.x, camParams.y);
    const float linDepthTransp = linearizeDepth(transparentDepth, camParams.x, camParams.y);

    const float waterFog = min(1.0 - exp(-(linDepthOpaque - linDepthTransp) * waterFogDensity), waterFogMaxBlendFactor);
    const float waterFogDecay = clamp(pow(waterFog, 4.0), 0.0, 1.0);

    opaque.rgb = mix(opaque.rgb, mix(waterFogColor0, waterFogColor1, waterFogDecay), waterFog);

    const vec3 transparents = albedoTransparents.rgb * lightingTransparents;

    outColor.rgb = mix(transparents, opaque, albedoTransparents.a);
  }
  else
  {
    outColor.rgb = albedo * lighting;
  }

  vec4 fog = vec4(vec3(0.0), 1.0);

  if (fogDepth < 1.0)
  {
    const float sunDensity = 12.0;
    const float fogDensity = 0.0004; 
    const float fogStart = 1000.0;
    const float fogMaxBlendFactor = 0.95;
    const vec3 fogSunColor = vec3(1.0, 0.9, 0.8); 
    const vec3 fogSkyColor = vec3(0.2, 0.5, 1.0);

    const vec3 posWS = unproject(inUV0, fogDepth, uboPerInstance.invViewProjMatrix);
    const vec3 ray = posWS - uboPerInstance.camPosition.xyz;
    const float rayDist = length(ray);
    const vec3 rayDir = ray / rayDist;

    const vec3 lightVec = vec3(1.0, 0.45, -0.15);
    const float sunAmount = max(dot(rayDir, lightVec), 0.0);
    fog.a = 1.0 - min(max((1.0 - exp((-rayDist + fogStart) * fogDensity)), 0.0), fogMaxBlendFactor);
    fog.rgb = (1.0 - fog.a) * mix(fogSkyColor, fogSunColor, pow(sunAmount, sunDensity));
  }

  {
    const vec3 volLightingCoord = screenSpacePosToCellIndex(vec3(inUV0, depthToVolumeZ(depthToVSDepth(fogDepth, camParams.x, camParams.y))));
    fog += textureLod(volumetricLightingScatteringBufferTex, volLightingCoord, 0.0);
  }

  outColor.rgb = outColor.rgb * fog.aaa + fog.rgb;
}
