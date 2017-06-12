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

// Output
OUTPUT

void main()
{ 
  GBuffer gbuffer;
  {
    gbuffer.albedo = vec4(0.0, 0.5, 0.5, 1.0);
    gbuffer.normal = normalize(vec3(0.0, 1.0, 0.0));
    gbuffer.metalMask = 0.0;
    gbuffer.specular = 0.5;
    gbuffer.roughness = 1.0;
    gbuffer.materialBufferIdx = 0;
    gbuffer.emissive = 0.0;
    gbuffer.occlusion = 1.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
