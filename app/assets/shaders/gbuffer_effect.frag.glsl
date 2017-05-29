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
  screenPos.z = linearizeDepth(screenPos.z, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

  const float opaqueDepth = linearizeDepth(textureLod(gbufferDepthTex, screenPos.xy, 0.0).r, 
    uboPerInstance.camParams.x, 
    uboPerInstance.camParams.y);
  if (opaqueDepth < screenPos.z)
  {
    discard;
  }

  vec4 albedo;
  albedo.a = clamp(sin(inNormal.x + uboPerInstance.data0.w * 2.0), 0.0, 1.0);
  albedo.rgb = vec3(1.0, 0.7, 0.7) * trunc(inNormal.xyz * 10.0 * 0.5 + 0.5) * 0.1;

  GBuffer gbuffer;
  {
    gbuffer.albedo = albedo;
    gbuffer.normal = normalize(inNormal);
    gbuffer.metalMask = uboPerMaterial.pbrBias.r;
    gbuffer.specular = 0.5 + uboPerMaterial.pbrBias.g;
    gbuffer.roughness = adjustRoughness(0.5 + uboPerMaterial.pbrBias.b, 1.0);
    gbuffer.materialBufferIdx = uboPerMaterial.data0.x;
    gbuffer.occlusion = 1.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
