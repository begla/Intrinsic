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
#include "gbuffer.inc.glsl"

// Ubos
PER_MATERIAL_UBO;
PER_INSTANCE_UBO;

// Bindings
BINDINGS_GBUFFER;
layout (binding = 6) uniform sampler2D gbufferDepthTex;
layout (binding = 7) uniform sampler2D foamTex;
layout (binding = 8) uniform sampler2D noiseTex;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec4 inPosition;

// Output
OUTPUT

const float edgeFadeDistance = 15.0;
const float waterBaseAlpha = 0.5;

void main()
{ 
  vec3 screenPos = inPosition.xyz / inPosition.w;
  screenPos.xy = screenPos.xy * 0.5 + 0.5;
  screenPos.z = linearizeDepth(screenPos.z, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

  const float opaqueDepth = linearizeDepth(textureLod(gbufferDepthTex, screenPos.xy, 0.0).r, 
    uboPerInstance.camParams.x, uboPerInstance.camParams.y);
  if (opaqueDepth < screenPos.z)
  {
    discard;
  }

  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);

  const vec2 uv0 = UV0_TRANSFORM_ANIMATED(inUV0);
  const vec2 uv0InverseAnim = UV0_TRANSFORM_ANIMATED_FACTOR(inUV0, vec2(0.2, 0.9));
  const vec2 uv0Raw = UV0(inUV0);

  vec4 albedo = texture(albedoTex, uv0);
  albedo.a = waterBaseAlpha;

  const vec4 normal0 = texture(normalTex, uv0);
  const vec4 normal1 = texture(normalTex, uv0InverseAnim * 0.2);
  const vec4 noise = texture(noiseTex, uv0 * 2.0);
  const vec4 noise1 = texture(noiseTex, uv0);
  const vec4 noise2 = texture(noiseTex, uv0 * 4.0);

  const float blend = 0.5;
  const vec4 normal = mix(normal0, normal1, blend);

  const vec4 foam = texture(foamTex, uv0 * 15.0);

  // Shore waves fakery
  float waveFade = clamp(max(opaqueDepth * uboPerInstance.camParams.x - screenPos.z, 0.0) 
      * uboPerInstance.camParams.y / (uboPerMaterial.waterParams.x * 2.0), 0.0, 1.0);
  float wave = sin(1.0 - length(uv0Raw.xy * 2.0 - 1.0) * 64.0 + uboPerInstance.data0.w);
  wave = clamp(wave - noise2.r + 0.5, 0.0, 1.0);
  albedo.a *= mix(wave, 1.0, waveFade);

  // Foam
  float foamFade = 1.0 
    - clamp(max(opaqueDepth * uboPerInstance.camParams.x - screenPos.z, 0.0) 
      * uboPerInstance.camParams.y / uboPerMaterial.waterParams.x, 0.0, 1.0);

  foamFade += clamp(noise.r - 0.4, 0.0, 1.0);
  foamFade *= clamp(noise1.r * 2.0 - 0.2, 0.0, 1.0);
  foamFade = min(foamFade, albedo.a);

  // Smooth edges
  const float edgeFade = 1.0 
    - clamp(max(opaqueDepth * uboPerInstance.camParams.x - screenPos.z, 0.0) 
      * uboPerInstance.camParams.y / uboPerMaterial.waterParams.y, 0.0, 1.0);

  albedo.a = mix(albedo.a, 1.0, foamFade);    
  albedo.a = mix(albedo.a, 0.0, edgeFade);
  albedo.rgb = mix(albedo.rgb, foam.rgb, foamFade);
  vec2 metalRoughness = mix(vec2(1.0, 0.15), vec2(0.0, 1.0), foamFade);

  GBuffer gbuffer;
  {
    gbuffer.albedo = albedo;
    gbuffer.normal = normalize(TBN * (normal.xyz * 2.0 - 1.0));
    gbuffer.metalMask = metalRoughness.x;
    gbuffer.specular = 0.0;
    gbuffer.roughness = adjustRoughness(metalRoughness.y, uboPerMaterial.data1.x);
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.emissive = 0.0;
    gbuffer.occlusion = 1.0; 
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
