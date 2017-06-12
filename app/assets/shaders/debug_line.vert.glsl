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

out gl_PerVertex
{
  vec4 gl_Position;
};

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;

layout (binding = 0) uniform PerInstance
{
  mat4 worldViewProjMatrix;
  mat4 viewMatrix;
} uboPerInstance;

void main()
{
  gl_Position = uboPerInstance.worldViewProjMatrix * vec4(inPosition.xyz, 1.0);
  outNormal = (uboPerInstance.viewMatrix * vec4(0.0, 1.0, 0.0, 0.0)).xyz;
  outColor = inColor.xyz;
}
