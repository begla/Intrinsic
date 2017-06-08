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

#extension GL_ARB_separate_shader_objects : disable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "lib_buffers.glsl"
#include "lib_clustering.glsl"
#include "ubos.inc.glsl"
#include "gbuffer.inc.glsl"

layout (binding = 0) uniform PerInstance
{
  vec4 nearFarWidthHeight;
  vec4 nearFar;
} uboPerInstance;

PER_FRAME_DATA(1);

layout (binding = 2) uniform sampler2D depthTex;

layout (std430, binding = 3) buffer readonly DecalBuffer
{
  Decal decals[];
};
layout (std430, binding = 4) buffer readonly DecalIndexBuffer
{
  uint decalIndices[];
};
layout (set = 1, binding = 0) uniform sampler2D textures0[4095];

layout (location = 0) in vec2 inUV0;

OUTPUT

void main()
{
  const float depth = textureLod(depthTex, inUV0, 0.0).x;

  // Ignore sky
  if (depth >= 1.0)
  { 
    discard;
  }

  vec3 posVS = unproject(inUV0, depth, uboPerFrame.invProjMatrix);

  vec4 albedo = vec4(0.0);
  vec3 normal = vec3(0.0);
  vec4 pbr = vec4(0.0);

  const uvec3 gridPos = calcGridPosForViewPos(posVS, uboPerInstance.nearFar, 
    uboPerInstance.nearFarWidthHeight);

  const uint clusterIdx = calcClusterIndex(gridPos, maxDecalCountPerCluster);
  const uint decalCount = decalIndices[clusterIdx];

  vec3 ddxPosVS = dFdx(posVS);
  vec3 ddyPosVS = -dFdy(posVS);
  //albedo = vec4(normalize(cross(ddxPosVS, ddyPosVS)), 1.00);

  const vec3 normVS = normalize(cross(ddxPosVS, ddyPosVS));
  const vec3 binormVS = normalize(ddxPosVS);
  const vec3 tangentVS = normalize(ddyPosVS);
  const mat3 TBN = mat3(tangentVS, binormVS, normVS);

  for (uint di=0; di<decalCount; ++di)
  {
    Decal decal = decals[decalIndices[clusterIdx + di + 1]];

    vec4 posDecalSS = decal.viewProjMatrix * vec4(posVS, 1.0);
    posDecalSS /= posDecalSS.w;

    if (all(lessThanEqual(posDecalSS.xyz, vec3(1.0, 1.0, 1.0)))
      && all(greaterThanEqual(posDecalSS.xyz, vec3(-1.0, -1.0, 0.0))))
    {
      const vec2 decalUV = posDecalSS.xy * 0.5 + 0.5;
      albedo = texture(textures0[decal.textureIds.x], decalUV);
      normal = texture(textures0[decal.textureIds.y], decalUV).xyz * 2.0 - 1.0;
      pbr = texture(textures0[decal.textureIds.z], decalUV);
    }
  }

  if (albedo.a < 0.2)
  {
    discard;
  }

  GBuffer gbuffer;
  {
    gbuffer.albedo = albedo;
    gbuffer.normal = normalize(TBN * normal);
    gbuffer.metalMask = pbr.r;
    gbuffer.specular = pbr.g;
    gbuffer.roughness = pbr.b;
    gbuffer.materialBufferIdx = 0u;
    gbuffer.emissive = 0.0;
    gbuffer.occlusion = 1.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
