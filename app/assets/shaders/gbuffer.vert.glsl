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

#include "surface_vertex.inc.glsl"

out gl_PerVertex
{
  vec4 gl_Position;
};

PER_INSTANCE_UBO();
INPUT();

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outTangent;
layout (location = 2) out vec3 outBinormal;
layout (location = 3) out vec3 outColor;
layout (location = 4) out vec2 outUV0;

void main()
{
  gl_Position = uboPerInstance.worldViewProjMatrix * vec4(inPosition.xyz, 1.0);

  outColor = inColor.xyz;
  outNormal = normalize(uboPerInstance.normalMatrix * vec4(inNormal, 0.0)).xyz;
  outTangent = normalize(uboPerInstance.normalMatrix * vec4(inTangent, 0.0)).xyz;
  outBinormal = normalize(uboPerInstance.normalMatrix * vec4(inBinormal, 0.0)).xyz;
  outUV0 = inUV0;
}
