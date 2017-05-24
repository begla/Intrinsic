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

// Ubos
PER_MATERIAL_UBO;
PER_INSTANCE_UBO;

// Bindings
BINDINGS_TERRAIN;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;

// Output
OUTPUT

vec3 blend(vec3 grass0, vec3 stone0, vec3 stone1, vec3 blendMask, float noise)
{
  return mix(grass0, mix(stone0, stone1, 1.0 -noise), clamp(blendMask.b * 3.0, 0.0, 1.0));
}

vec3 contrast(vec3 color, float contrast)
{
  return ((color.rgb - 0.5) * max(contrast, 0)) + 0.5;
}

float contrast(float color, float contrast)
{
  return ((color - 0.5) * max(contrast, 0)) + 0.5;
}

void main()
{ 
  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);

  const vec2 uv0 = UV0_TRANSFORMED_ANIMATED;
  const vec2 uv0Raw = UV0;

  const vec4 albedo0 = texture(albedoTex0, uv0);
  const vec4 normal0 = texture(normalTex0, uv0);
  const vec4 pbr0 = texture(pbrTex0, uv0);

  const vec4 albedo1 = texture(albedoTex1, uv0);
  const vec4 normal1 = texture(normalTex1, uv0);
  const vec4 pbr1 = texture(pbrTex1, uv0);
  
  const vec4 albedo2 = texture(albedoTex2, uv0 * 0.1);
  const vec4 normal2 = texture(normalTex2, uv0 * 0.1);
  const vec4 pbr2 = texture(pbrTex2, uv0 * 0.1);

  float noise = clamp(texture(noiseTex, uv0Raw * 10.0).r, 0.0, 1.0);
  vec3 blendMask = texture(blendMaskTex, uv0Raw).rgb;

  vec3 albedo = blend(albedo0.rgb, albedo1.rgb, albedo2.rgb, blendMask, noise);
  vec3 normal = blend(normal0.rgb, normal1.rgb, normal2.rgb, blendMask, noise);
  vec3 pbr = blend(pbr0.rgb, pbr1.rgb, pbr2.rgb, blendMask, noise);

  float occlusion = clamp(mix(clamp(noise * 5.0, 0.0, 1.0) * blendMask.b, 1.0 - blendMask.r, 
  clamp((1.0 - blendMask.g) * 2.0 - 0.9, 0.0, 1.0)) * 2.0 + 0.2, 0.0, 1.0);
  albedo *= clamp(occlusion , 0.0, 1.0);

  GBuffer gbuffer;
  {
    gbuffer.albedo = vec4(albedo, 1.0) * uboPerInstance.colorTint;
    gbuffer.normal = normalize(TBN * (normal * 2.0 - 1.0));
    gbuffer.metalMask = pbr.r + uboPerMaterial.pbrBias.r;
    gbuffer.specular = pbr.g + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = pbr.b + uboPerMaterial.pbrBias.b;
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.occlusion = occlusion;
    gbuffer.emissive = 0.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
