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

layout(binding = 2) uniform sampler2D inputTex;
layout(binding = 3) uniform sampler2D areaTex;
layout(binding = 4) uniform sampler2D searchTex;

layout(location = 0) in vec2 inUV0;
layout(location = 1) in vec2 inPixelPos;
layout(location = 2) in vec4 inOffsets[3];
layout(location = 0) out vec4 outColor;

#include "ubos.inc.glsl"

PER_INSTANCE_DATA_SMAA_FRAG;

#define SMAA_INCLUDE_PS 1
#define SMAA_INCLUDE_VS 0
#include "SMAA.h"

void main()
{
  outColor = SMAABlendingWeightCalculationPS(
      inUV0, inPixelPos, inOffsets, inputTex, areaTex, searchTex, int4(0));
}
