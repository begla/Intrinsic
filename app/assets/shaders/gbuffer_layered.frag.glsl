// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
BINDINGS_LAYERED();

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
  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);

  const vec2 uv0 = UV0_TRANSFORMED_ANIMATED;
  const vec2 uv0Raw = UV0;

  const vec4 albedo0 = texture(albedoTex0, uv0);
  const vec4 normal0 = texture(normalTex0, uv0);
  const vec4 roughness0 = texture(roughnessTex0, uv0);

  const vec4 albedo1 = texture(albedoTex1, uv0);
  const vec4 normal1 = texture(normalTex1, uv0);
  const vec4 roughness1 = texture(roughnessTex1, uv0);
  
  const vec4 albedo2 = texture(albedoTex2, uv0);
  const vec4 normal2 = texture(normalTex2, uv0);
  const vec4 roughness2 = texture(roughnessTex2, uv0);

  vec4 blendMask = texture(blendMaskTex, uv0Raw);

  vec3 albedo = min(albedo0.rgb * blendMask.r + albedo1.rgb * blendMask.g + albedo2.rgb * blendMask.b, vec3(1.0));
  vec3 normal = min(normal0.rgb * blendMask.r + normal1.rgb * blendMask.g + normal2.rgb * blendMask.b, vec3(1.0));
  vec3 roughness = min(roughness0.rgb * blendMask.r + roughness1.rgb * blendMask.g + roughness2.rgb * blendMask.b, vec3(1.0));

  outAlbedo = vec4(albedo.rgb * uboPerInstance.data0.x, 1.0); // Albedo
  outNormal.rg = encodeNormal(normalize(TBN * (normal.xyz * 2.0 - 1.0)));
  outNormal.b = roughness.g + uboPerMaterial.pbrBias.g; // Specular
  outNormal.a = max(roughness.b + uboPerMaterial.pbrBias.b, 0.01); // Roughness
  outParameter0.rgba = vec4(roughness.r + uboPerMaterial.pbrBias.r, uboPerMaterial.data0.x, 0.0, 0.0); // Metal Mask / Material Buffer Index;
}
