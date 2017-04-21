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

  mat4 shadowViewProjMatrix[PSSM_SPLIT_COUNT];
} uboPerInstance; 

layout (binding = 1) uniform sampler2D albedoTex;
layout (binding = 2) uniform sampler2D normalTex;
layout (binding = 3) uniform sampler2D parameter0Tex;
layout (binding = 4) uniform sampler2D depthTex;
layout (binding = 5) uniform samplerCube irradianceTex;
layout (binding = 6) uniform samplerCube specularTex;
layout (binding = 7) uniform sampler2DArrayShadow shadowBufferTex;
layout (binding = 8) MATERIAL_BUFFER;

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

void main()
{
  const float depth = textureLod(depthTex, inUV0, 0.0).x;
  if (depth == 1.0)
  { 
    outColor.rgba = vec4(1.0, 1.0, 1.0, 1.0);
    return;
  }

  outColor.rgba = vec4(0.0);

  const vec3 posVS = unproject(inUV0, depth, uboPerInstance.invProjMatrix);

  const vec4 normalSample = textureLod(normalTex, inUV0, 0.0);
  const vec4 albedoSample = textureLod(albedoTex, inUV0, 0.0);
  const vec4 parameter0Sample = textureLod(parameter0Tex, inUV0, 0.0);
  const MaterialParameters matParams = materialParameters[uint(parameter0Sample.y)];

  LightingData d;
  d.baseColor = albedoSample.rgb;  
  d.metalMask = parameter0Sample.r;
  d.specular = normalSample.b;
  d.roughness = normalSample.a;

  d.N = normalize(decodeNormal(normalSample.rg));  
  d.L = normalize(uboPerInstance.viewMatrix * vec4(1.0, 0.45, -0.15, 0.0)).xyz;
  d.V = -normalize(posVS); 
  d.energy = 30.0 * vec3(1.0, 1.0, 1.0);
  initLightingData(d);

  // Ambient lighting
  const vec3 normalWS = (uboPerInstance.invViewMatrix * vec4(d.N, 0.0)).xyz;
  const vec3 vWS = (uboPerInstance.invViewMatrix * vec4(d.V, 0.0)).xyz;

  const vec3 R0 = 2.0 * dot(vWS, normalWS) * normalWS - vWS;
  const vec3 R = mix(normalWS, R0, (1.0 - d.roughness2) * (sqrt(1.0 - d.roughness2) + d.roughness2));

  const vec3 irrad = d.diffuseColor * textureLod(irradianceTex, R, 0.0).rgb;
  outColor.rgb += irrad; 

  const float maxSpecPower = 999999.0;
  const float k0 = 0.00098; const float k1 = 0.9921;
  const float maxT = (exp2(-10.0/sqrt(maxSpecPower)) - k0)/k1;

  float specPower = (2.0 / d.roughness2) - 2.0;
  specPower /= (4.0 * max(dot(normalWS, R), 1.0e-6)); 

  const float smulMaxT = (exp2(-10.0 / sqrt(specPower)) - k0) / k1;

  const int mipOffset = 3;
  const int numMips = 9;

  const float specLod = float(numMips-1-mipOffset) * (1.0 - clamp(smulMaxT/maxT, 0.0, 1.0));
  const vec3 spec = d.specularColor * textureLod(specularTex, R, specLod).rgb;

  outColor.rgb += spec;

  // Directional main light shadows
  float shadowAttenuation = calcShadowAttenuation(posVS, uboPerInstance.shadowViewProjMatrix, shadowBufferTex);

  // Direct lighting
  const vec3 lightColor = vec3(1.0, 0.7, 0.75);
  outColor.rgb += shadowAttenuation * calcLighting(d) * lightColor;

  // Translucency
  const float translDistortion = 1.0;
  const float translPower = 3.0;
  const float translScale = 5.0;
  const vec3 translAmbient = vec3(0.0);
  const float translThickness = matParams.translucencyThickness;

  vec3 translLightVector = d.L + d.N * translDistortion;
  float translDot = exp2(clamp(dot(d.V, -translLightVector), 0.0, 1.0) * translPower - translPower) * translScale;
  vec3 transl = (translDot + translAmbient) * translThickness;
  vec3 translColor = d.diffuseColor * lightColor * transl;

  outColor.rgb += shadowAttenuation * translColor;

  outColor.a = 1.0;
}
  