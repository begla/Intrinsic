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

#include "lib_noise.glsl"
#include "lib_math.glsl"

layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outParameter0;

layout (binding = 1) uniform PerInstance
{
  mat4 invWorldRotMatrix;
  vec4 invWorldPos;
  mat4 viewProjMatrix;
  mat4 normalMatrix;

  vec4 planeNormal;

  float gridSize;
  float fade;

} uboPerInstance;

#define GRID_RANGE 50.0
#define GRID_FADE_RANGE 40.0

#define BIG_GRID_COLOR vec3(0.1, 0.5, 0.75)
#define TINY_GRID_COLOR BIG_GRID_COLOR * 0.75

bool isGridVisible(vec2 localPos, float lineWidth, float cellSize)
{
  return mod(localPos.x + lineWidth * 0.5, cellSize) <= lineWidth 
    || mod(localPos.y + lineWidth * 0.5, cellSize) <= lineWidth;
}

void main()
{
  const vec3 localPos = inPosition.xyz + uboPerInstance.invWorldPos.xyz;
  const vec3 worldPos = inPosition.xyz;

  vec2 projLocalPos = localPos.xz;
  vec2 projWorldPos = worldPos.xz;

  if (abs(uboPerInstance.planeNormal.xyz) == vec3(1.0, 0.0, 0.0))
  {
    projLocalPos = localPos.yz;
    projWorldPos = worldPos.yz;
  }
  else if (abs(uboPerInstance.planeNormal.xyz) == vec3(0.0, 0.0, 1.0))
  {
    projLocalPos = localPos.xy;
    projWorldPos = worldPos.xy;
  }

  const float xzDist = length(projLocalPos);

  const float gridRange = uboPerInstance.gridSize * uboPerInstance.fade * GRID_RANGE;
  const float fadeRange = uboPerInstance.gridSize * uboPerInstance.fade * GRID_FADE_RANGE ;
  const float fadeInterval = gridRange - fadeRange;
  const float noiseScale = 1.0 / uboPerInstance.gridSize * 0.4;

  if (xzDist > fadeInterval)
  {
    const float fadePerc = (gridRange - xzDist) / fadeInterval;

    if (abs(noise(vec3(projLocalPos * noiseScale, 0.0))) > fadePerc)
    {
      discard;
    }
  }

  const float bigGridCellSize = 5.0 * uboPerInstance.gridSize;
  const float tinyGridCellSize = uboPerInstance.gridSize;
  const float bigGridLineWidth = calcScreenSpaceScale(inPosition.xyz, uboPerInstance.viewProjMatrix, 0.004);
  const float tinyGridLineWidth = calcScreenSpaceScale(inPosition.xyz, uboPerInstance.viewProjMatrix, 0.0025);

  const bool tinyGridVisible = isGridVisible(projWorldPos, tinyGridLineWidth, tinyGridCellSize);
  const bool bigGridVisible = isGridVisible(projWorldPos, bigGridLineWidth, bigGridCellSize);

  if (!tinyGridVisible && !bigGridVisible)
  {
    discard;
  }

  vec3 color = BIG_GRID_COLOR;
  if (tinyGridVisible && !bigGridVisible)
  {
    color = TINY_GRID_COLOR;
  }

  outAlbedo = vec4(color, 1.0);
  outNormal = vec4(encodeNormal((uboPerInstance.normalMatrix * vec4(uboPerInstance.planeNormal.xyz, 0.0)).xyz), 1.0, 0.5);
  outParameter0 = vec4(0.0, 1.0, 1.0, 0.0);
}
