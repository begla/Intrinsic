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

/* __PREPROCESSOR DEFINES__ */

#include "lib_math.glsl"
#include "gbuffer.inc.glsl"

#if defined (PRE_PASS)
# undef OUTPUT
# define OUTPUT
#endif // PRE_PASS

// Ubos
PER_MATERIAL_UBO;
PER_INSTANCE_UBO;

// Bindings
BINDINGS_GBUFFER;
layout (binding = 6) uniform sampler2D blendMaskTex;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec3 inWorldPosition;

// Output
OUTPUT

void main()
{ 
  float ns = 1.0;
  if (!gl_FrontFacing)
  {
    ns = -1.0;
  }

  const mat3 TBN = mat3(inTangent, inBinormal, ns * inNormal);

  const vec2 uv0 = clamp(UV0_TRANSFORMED, 0.001, 0.999);

  const vec2 blendScale = vec2(25.0);
  const vec2 blendLookupUV = mod(abs(inWorldPosition.xz), blendScale) / blendScale;
  const vec4 blendMask = texture(blendMaskTex, blendLookupUV);

  vec4 albedo = texture(albedoTex, uv0);
  albedo.rgb *= blendMask.rgb;

  const vec3 normal = texture(normalTex, uv0).rgb;
  const vec3 pbr = texture(pbrTex, uv0).rgb;

  if (albedo.a < 0.3)
  {
    discard;
  }

#if !defined (PRE_PASS)
  GBuffer gbuffer;
  {
    gbuffer.albedo = albedo * uboPerInstance.colorTint;
    gbuffer.normal = normalize(TBN * (normal * 2.0 - 1.0));
    gbuffer.metalMask = pbr.r + uboPerMaterial.pbrBias.r;
    gbuffer.specular = pbr.g + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = pbr.b + uboPerMaterial.pbrBias.b;
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.occlusion = 1.0;
    gbuffer.emissive = 0.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
#endif // PRE_PASS
}
