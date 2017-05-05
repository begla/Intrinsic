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
layout (binding = 6) uniform sampler2D gbufferDepthTex;

// Input
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec4 inPosition;

// Output
OUTPUT

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

  vec4 albedo = vec4(0.0, 0.5, 0.8, abs(sin(inNormal.x + uboPerInstance.data0.w * 2.0)));
  albedo.rgba *= 1.0;

  outAlbedo = vec4(albedo.rgb * uboPerInstance.data0.x, albedo.a); // Albedo
  outNormal.rg = encodeNormal(inNormal);
  outNormal.b = uboPerMaterial.pbrBias.g; // Specular
  outNormal.a = max(uboPerMaterial.pbrBias.b, 0.01); // Roughness
  outParameter0.rgba = vec4(uboPerMaterial.pbrBias.r, uboPerMaterial.data0.x, 1.0, 0.0); // Metal Mask / Material Buffer Idx
}
