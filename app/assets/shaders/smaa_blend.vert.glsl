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

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec4 outOffset;

#include "ubos.inc.glsl"

PER_INSTANCE_DATA_SMAA_VERT;

#define SMAA_INCLUDE_PS 0
#define SMAA_INCLUDE_VS 1
#include "SMAA.h"

void main()
{
  outUV0 = vec2(float(gl_VertexIndex / 2) * 2.0, float(gl_VertexIndex % 2) * 2.0);
  gl_Position = vec4(float(gl_VertexIndex / 2) * 4.0 - 1.0, float(gl_VertexIndex % 2) 
  	* 4.0 - 1.0, 0.0, 1.0);

  SMAANeighborhoodBlendingVS(outUV0, outOffset);
}
