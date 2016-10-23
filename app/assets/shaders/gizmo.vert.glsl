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

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBinormal;
layout (location = 5) in vec4 inColor;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;

layout (binding = 0) uniform PerInstance
{
  mat4 worldMatrix;
  mat4 worldViewProjMatrix;
  mat4 normalMatrix;

  vec4 colorTintX;
  vec4 colorTintY;
  vec4 colorTintZ;
} uboPerInstance;

void main()
{
  vec3 localPos = inPosition.xyz;

  outColor = inColor.xyz;

  if (abs(localPos.x) > 0.1
    && abs(localPos.y) < 0.25
    && abs(localPos.y) < 0.25)
  {
    outColor *= uboPerInstance.colorTintX.xyz;
  }
  else if (abs(localPos.y) > 0.1
    && abs(localPos.x) < 0.25
    && abs(localPos.z) < 0.25)
  {
    outColor *= uboPerInstance.colorTintY.xyz;
  }
  else if (abs(localPos.z) > 0.1
    && abs(localPos.x) < 0.25
    && abs(localPos.y) < 0.25)
  {
    outColor *= uboPerInstance.colorTintZ.xyz;
  }

  vec3 worldPos = (uboPerInstance.worldMatrix * vec4(inPosition.xyz, 1.0)).xyz;

  gl_Position = uboPerInstance.worldViewProjMatrix * vec4(localPos, 1.0);
  gl_Position.z *= 0.001;

  outNormal = (uboPerInstance.normalMatrix * vec4(inNormal, 0.0)).xyz;
}
