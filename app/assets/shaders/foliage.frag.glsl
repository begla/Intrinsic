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
#include "surface_fragment.inc.glsl"

// Ubos
PER_MATERIAL_UBO();
PER_INSTANCE_UBO();

// Bindings
BINDINGS();

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;

// Output
OUTPUT();

void main()
{ 
  float ns = 1.0;
  if (!gl_FrontFacing)
  {
    ns = -1.0;
  }

  const mat3 TBN = mat3(inTangent, inBinormal, ns * inNormal);

  const vec2 uv0 = clamp(UV0_TRANSFORMED, 0.001, 0.999);

  const vec4 albedo = texture(albedoTex, uv0);
  const vec4 normal = texture(normalTex, uv0);
  const vec4 roughness = texture(roughnessTex, uv0);

  if (albedo.a < 0.3)
  {
    discard;
  }

  outAlbedo = vec4(albedo.rgb * uboPerInstance.data0.x, 1.0); // Albedo
  outNormal.rg = encodeNormal(normalize(TBN * (normal.xyz * 2.0 - 1.0)));
  outNormal.b = roughness.g + uboPerMaterial.pbrBias.g; // Specular
  outNormal.a = max(roughness.b + uboPerMaterial.pbrBias.b, 0.05); // Roughness
  outParameter0.rgba = vec4(roughness.r + uboPerMaterial.pbrBias.r, uboPerMaterial.data0.x, 0.0, 0.0); // Metal Mask / Material Buffer Idx
}
