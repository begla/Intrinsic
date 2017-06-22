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

#extension GL_ARB_separate_shader_objects : disable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "lib_buffers.glsl"
#include "lib_clustering.glsl"
#include "ubos.inc.glsl"
#include "gbuffer.inc.glsl"

const float alphaThreshold = 0.2;

layout(binding = 0) uniform PerInstance
{
  vec4 nearFarWidthHeight;
  vec4 nearFar;
}
uboPerInstance;

PER_FRAME_DATA(1);

layout(binding = 2) uniform sampler2D depthTex;

layout(std430, binding = 3) buffer readonly DecalBuffer { Decal decals[]; };
layout(std430, binding = 4) buffer readonly DecalIndexBuffer
{
  uint decalIndices[];
};
layout(set = 1, binding = 0) uniform sampler2D globalTextures[4095];

layout(location = 0) in vec2 inUV0;

OUTPUT

#include "decals.inc.glsl"

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

  const uvec3 gridPos = calcGridPosForViewPos(
      posVS, uboPerInstance.nearFar, uboPerInstance.nearFarWidthHeight);

  const uint clusterIdx =
      calcClusterIndex(gridPos, maxDecalCountPerCluster) / 2;
  const uint decalCount = decalIndices[clusterIdx];

  for (uint di = 0; di < decalCount; di += 2)
  {
    uint packedDecalIndices = decalIndices[clusterIdx + di / 2 + 1];

    Decal decal = decals[packedDecalIndices & 0xFFFF];
    calcDecal(decal, posVS, globalTextures, albedo, normal, pbr, 1.0);
    decal = decals[packedDecalIndices >> 16];
    calcDecal(decal, posVS, globalTextures, albedo, normal, pbr,
              float(di + 1 < decalCount));
  }

  if (albedo.a < alphaThreshold)
  {
    discard;
  }

  GBuffer gbuffer;
  {
    gbuffer.albedo = albedo;
    gbuffer.normal = normal;
    gbuffer.metalMask = pbr.r;
    gbuffer.specular = pbr.g;
    gbuffer.roughness = pbr.b;
    gbuffer.materialBufferIdx = 0u;
    gbuffer.emissive = 0.0;
    gbuffer.occlusion = 1.0;
  }
  writeGBuffer(gbuffer, outAlbedo, outNormal, outParameter0);
}
