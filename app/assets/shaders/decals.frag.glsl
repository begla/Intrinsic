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
#include "lib_noise.glsl"
#include "lib_lighting.glsl"
#include "lib_buffers.glsl"
#include "lib_clustering.glsl"
#include "ubos.inc.glsl"

layout (binding = 0) uniform PerInstance
{
  vec4 nearFarWidthHeight;
  vec4 nearFar;
} uboPerInstance;

PER_FRAME_DATA(1);

layout (binding = 2) uniform sampler2D depthTex;
layout (binding = 3) uniform sampler2D testTex;

layout (std430, binding = 4) buffer readonly DecalBuffer
{
  Decal decals[];
};
layout (std430, binding = 5) buffer readonly DecalIndexBuffer
{
  uint decalIndices[];
};

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

void main()
{
  const float depth = textureLod(depthTex, inUV0, 0.0).x;

  // Ignore sky
  if (depth >= 1.0)
  { 
    discard;
  }

  vec3 posVS = unproject(inUV0, depth, uboPerFrame.invProjMatrix);

  outColor.rgba = vec4(0.0);

  const uvec3 gridPos = calcGridPosForViewPos(posVS, uboPerInstance.nearFar, 
    uboPerInstance.nearFarWidthHeight);

  const uint clusterIdx = calcClusterIndex(gridPos, maxDecalCountPerCluster);
  const uint decalCount = decalIndices[clusterIdx];

  for (uint di=0; di<decalCount; ++di)
  {
    Decal decal = decals[decalIndices[clusterIdx + di + 1]];

    vec4 posDecalSS = decal.viewProjMatrix * vec4(posVS, 1.0);
    posDecalSS /= posDecalSS.w;

    if (all(lessThanEqual(posDecalSS.xyz, vec3(1.0, 1.0, 1.0)))
      && all(greaterThanEqual(posDecalSS.xyz, vec3(-1.0, -1.0, 0.0))))
    {
      const vec2 decalUV = posDecalSS.xy * 0.5 + 0.5;
      outColor = texture(testTex, decalUV);
    }
  }

  if (outColor.a < EPSILON)
  {
    discard;
  }
}
