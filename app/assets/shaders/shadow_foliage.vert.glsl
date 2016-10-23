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

#include "lib_noise.glsl"
#include "surface_vertex.inc.glsl"

out gl_PerVertex
{
  vec4 gl_Position;
};

// Ubos
PER_INSTANCE_UBO();

// Input
INPUT();

// Output
layout (location = 0) out vec2 outUV0;

void main()
{
  vec3 localPos = inPosition.xyz;
  const vec3 worldPos = (uboPerInstance.worldMatrix * vec4(inPosition.xyz, 1.0)).xyz;
  const vec3 worldNormal = normalize((uboPerInstance.worldMatrix * vec4(inNormal.xyz, 0.0)).xyz);

  applyWind(worldPos, worldNormal, inColor.r, uboPerInstance.data0.w, localPos);

  gl_Position = uboPerInstance.worldViewProjMatrix * vec4(localPos, 1.0);

  outUV0 = inUV0; 
}
