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
BINDINGS_GBUFFER;
layout (binding = 6) uniform sampler2D emissiveTex;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;

// Output
OUTPUT

void main()
{ 
  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);
  const vec2 uv0 = UV0_TRANSFORM_ANIMATED(inUV0);

  GBuffer gbuffer;
  {
    gbuffer.albedo = texture(albedoTex, uv0) * uboPerInstance.colorTint;
    gbuffer.normal = normalize(TBN * (texture(normalTex, uv0).xyz * 2.0 - 1.0));
    const vec4 pbr = texture(pbrTex, uv0);
    gbuffer.metalMask = pbr.r + uboPerMaterial.pbrBias.r;
    gbuffer.specular = pbr.g + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = adjustRoughness(pbr.b + uboPerMaterial.pbrBias.b, 1.0);
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.emissive = texture(emissiveTex, uv0).r;
    gbuffer.occlusion = 1.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
