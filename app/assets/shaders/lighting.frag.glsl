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
#include "lib_noise.glsl"
#include "lib_lighting.glsl"
#include "lib_buffers.glsl"

layout (binding = 0) uniform PerInstance
{
  mat4 viewMatrix;
  mat4 invProjMatrix;
  mat4 invViewMatrix;

  mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT];

  vec4 nearFarWidthHeight;
  vec4 nearFar;
} uboPerInstance; 

layout (binding = 1) uniform sampler2D albedoTex;
layout (binding = 2) uniform sampler2D normalTex;
layout (binding = 3) uniform sampler2D parameter0Tex;
layout (binding = 4) uniform sampler2D depthTex;
layout (binding = 5) uniform sampler2D ssaoTex;
layout (binding = 12) uniform sampler2D kelvinLutTex;
layout (binding = 6) uniform samplerCube irradianceTex;
layout (binding = 7) uniform samplerCube specularTex;
layout (binding = 8) uniform sampler2DArrayShadow shadowBufferTex;
layout (binding = 9) MATERIAL_BUFFER;
layout (binding = 10) buffer LightBuffer
{
  Light lights[];
};
layout (binding = 11) buffer LightIndexBuffer
{
  uint lightIndices[];
};

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

const vec4 mainLightDir = vec4(1.0, 0.45, -0.275, 0.0);
const float mainLightTemp = 3200.0;
const float mainLightIntens = 20.0;

const float translDistortion = 0.2;
const float translPower = 12.0;
const float translScale = 1.0;
const vec3 translAmbient = vec3(0.0);

void main()
{
  const vec4 albedoSample = textureLod(albedoTex, inUV0, 0.0);
  const float depth = textureLod(depthTex, inUV0, 0.0).x;

  // Pass through sky
  if (depth == 1.0)
  { 
    outColor.rgba = albedoSample;
    return;
  }

  outColor.rgba = vec4(0.0);

  const vec3 posVS = unproject(inUV0, depth, uboPerInstance.invProjMatrix);

  const vec4 ssaoSample = textureLod(ssaoTex, inUV0, 0.0);
  const vec4 normalSample = textureLod(normalTex, inUV0, 0.0);
  const vec4 parameter0Sample = textureLod(parameter0Tex, inUV0, 0.0);
  const MaterialParameters matParams = materialParameters[uint(parameter0Sample.y)];

  LightingData d;
  d.baseColor = albedoSample.rgb;  
  d.metalMask = parameter0Sample.r;
  d.specular = normalSample.b;
  d.roughness = normalSample.a;
  initLightingDataFromGBuffer(d);

  d.N = normalize(decodeNormal(normalSample.rg));  
  d.L = normalize(uboPerInstance.viewMatrix * mainLightDir).xyz;
  d.V = -normalize(posVS); 
  d.energy = vec3(mainLightIntens);
  calculateLightingData(d);

  // Ambient lighting
  const vec3 normalWS = (uboPerInstance.invViewMatrix * vec4(d.N, 0.0)).xyz;
  const vec3 vWS = (uboPerInstance.invViewMatrix * vec4(d.V, 0.0)).xyz;

  const vec3 R0 = 2.0 * dot(vWS, normalWS) * normalWS - vWS;
  const vec3 R = mix(normalWS, R0, (1.0 - d.roughness2) * (sqrt(1.0 - d.roughness2) + d.roughness2));

  const vec3 irrad = d.diffuseColor * texture(irradianceTex, R).rgb;
  outColor.rgb += min(ssaoSample.r, parameter0Sample.z) * irrad; 

  // Specular
  const float specLod = burleyToMip(d.roughness, dot(normalWS, R0));
  const vec3 spec = d.specularColor * textureLod(specularTex, R, specLod).rgb;

  outColor.rgb += parameter0Sample.z * spec;

  // Emissive
  outColor.rgb += matParams.emissiveIntensity * d.baseColor;

  const vec3 mainLightColor = kelvinToRGB(mainLightTemp, kelvinLutTex);

  // Direct lighting
  float shadowAttenuation = calcShadowAttenuation(posVS, uboPerInstance.shadowViewProjMatrix, shadowBufferTex);
  outColor.rgb += shadowAttenuation * mainLightColor * calcLighting(d);

  // Translucency
  const float translThickness = matParams.translucencyThickness;

  vec3 translLightVector = d.L + d.N * translDistortion;
  float translDot = exp2(clamp(dot(d.V, -translLightVector), 0.0, 1.0) * translPower - translPower) * translScale;
  vec3 transl = (translDot + translAmbient) * translThickness;
  vec3 translColor = d.diffuseColor * mainLightColor * transl;

  outColor.rgb += translColor;

  const uvec3 gridPos = calcGridPosForViewPos(posVS, uboPerInstance.nearFar, uboPerInstance.nearFarWidthHeight);
  const uint clusterIdx = calcClusterIndex(gridPos);

  // Point lights
  const uint lightCount = lightIndices[clusterIdx];

  // DEBUG: Visualize clusters
  /*if (lightCount > 0)
  {
    outColor = vec4(gridPos / vec3(gridRes), 0.0);
    return;
  }*/

  for (uint li=0; li<lightCount; ++li)
  {
    Light light = lights[lightIndices[clusterIdx + li + 1]];

    const vec3 lightDistVec = light.posAndRadius.xyz - posVS;
    const float dist = length(lightDistVec);
    d.L = lightDistVec / dist;
    d.energy = vec3(light.colorAndIntensity.a);
    calculateLightingData(d);

    outColor.rgb += 
      calcInverseSqrFalloff(light.posAndRadius.w, dist) 
      * calcLighting(d) * light.colorAndIntensity.rgb 
      * kelvinToRGB(light.temp.r, kelvinLutTex);
  }

  outColor.a = 1.0;
}
  