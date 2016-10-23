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
BINDINGS();
layout (binding = 6) uniform sampler2D gbufferDepthTex;
layout (binding = 7) uniform sampler2D foamTex;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec4 inPosition;

// Output
OUTPUT();

void main()
{ 
  vec3 screenPos = inPosition.xyz / inPosition.w;
  screenPos.xy = screenPos.xy * 0.5 + 0.5;

  const vec4 camParams = vec4(1.0, 5000.0, 1.0, 1.0 / 5000.0);
  screenPos.z = linearizeDepth(screenPos.z, camParams.x, camParams.y);

  const float opaqueDepth = linearizeDepth(textureLod(gbufferDepthTex, screenPos.xy, 0.0).r, camParams.x, camParams.y);
  if (opaqueDepth < screenPos.z)
  {
    discard;
  }

  const mat3 TBN = mat3(inTangent, inBinormal, inNormal);

  const vec2 uv0 = UV0_TRANSFORMED_ANIMATED;
  const vec2 uv0InverseAnim = UV0_TRANSFORMED_ANIMATED_FACTOR(vec2(0.4, 0.3));

  const vec4 albedo0 = texture(albedoTex, uv0);
  const vec4 normal0 = texture(normalTex, uv0);

  const vec4 albedo1 = texture(albedoTex, uv0InverseAnim * 0.4);
  const vec4 normal1 = texture(normalTex, uv0InverseAnim * 0.4);

  const float blend = 0.5;
  vec4 albedo = mix(albedo0, albedo1, blend);
  const vec4 normal = mix(normal0, normal1, blend);

  const vec4 foam = texture(foamTex, uv0);
  vec4 roughness = texture(roughnessTex, uv0);

  const float baseAlpha = 0.5;
  const float edgeFadeDistance = 0.0;

  const float foamFade = 1.0 
    - clamp(max(opaqueDepth * camParams.x - screenPos.z, 0.0) * camParams.y / uboPerMaterial.pbrBias.w / foam.r, 0.0, 1.0);
  const float edgeFade = 1.0 
    - clamp(max(opaqueDepth * camParams.x - screenPos.z, 0.0) * camParams.y / edgeFadeDistance, 0.0, 1.0);

  float alpha = mix(baseAlpha, 0.0, foamFade);
  alpha = mix(alpha, 1.0, edgeFade);

  albedo.rgb = mix(albedo.rgb, foam.rgb, foamFade);

  roughness.xz = mix(roughness.xz, vec2(0.0, 1.0), foamFade);

  outAlbedo = vec4(albedo.rgb * uboPerInstance.data0.x, alpha); // Albedo
  outNormal.rg = encodeNormal(normalize(TBN * (normal.xyz * 2.0 - 1.0)));
  outNormal.b = roughness.g + uboPerMaterial.pbrBias.g; // Specular
  outNormal.a = max(roughness.b + uboPerMaterial.pbrBias.b, 0.01); // Roughness
  outParameter0.rgba = vec4(1.0, uboPerMaterial.data0.x, 0.0, 0.0); // Metal Mask / Material Buffer Idx
}
