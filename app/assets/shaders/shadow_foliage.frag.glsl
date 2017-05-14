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
layout (binding = 3) uniform sampler2D albedoTex;

// Input
layout (location = 0) in vec2 inUV0;

void main()
{ 
  const vec2 uv0 = UV0_TRANSFORMED_ANIMATED;

  vec4 albedo = texture(albedoTex, uv0);

  albedo.rgb /= albedo.a;
  if (albedo.a < 0.5)
  {
    discard;
  }
}
