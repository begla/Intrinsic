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

struct Light
{
  vec4 posAndRadius;
  vec4 colorAndIntensity;
  float temp;
};
struct IrradProbe
{
  vec4 posAndRadius;
  vec4 data0;

  vec4 shData[7];
};
struct Decal
{
  mat4 viewProjMatrix;
  vec4 posAndRadiusVS;
};

// Have to match the values on C++ side
const uint maxLightCountPerCluster = 256;
const uint maxIrradProbeCountPerCluster = 4;
const uint maxDecalCountPerCluster = 64;

const float gridDepth = 10000.0f;
const uvec3 gridRes = uvec3(16u, 8u, 24u);
const float gridDepthExp = 3.0;
const float gridDepthSliceScale =
    gridDepth / pow(gridRes.z - 1.0, gridDepthExp);
const float gridDepthSliceScaleRcp = 1.0f / gridDepthSliceScale;

uint calcClusterIndex(uvec3 gridPos, uint clusterSize)
{
  return gridPos.x * clusterSize +
         gridPos.y * gridRes.x * clusterSize +
         gridPos.z * gridRes.y * gridRes.x * clusterSize;
}

float calcGridDepthSlice(uint depthSliceIdx)
{
  return pow(depthSliceIdx, gridDepthExp) * gridDepthSliceScale;
}

uint calcGridDepthIndex(float depthVS)
{
  return uint(pow(depthVS * gridDepthSliceScaleRcp, 1.0 / gridDepthExp));
}

uvec3 clampGridPos(in uvec3 gridPos)
{
  return clamp(gridPos, uvec3(0), gridRes - 1u);
}

uvec3 calcGridPosForViewPos(vec3 posVS, vec4 nearFar, vec4 nearFarWidthHeight)
{
  const uint gridDepthIdx = calcGridDepthIndex(-posVS.z);
  const float gridStartDepth = calcGridDepthSlice(gridDepthIdx);
  const float gridEndDepth = calcGridDepthSlice(gridDepthIdx + 1);

  const float rayPos = (gridEndDepth - nearFar.x) / (nearFar.y - nearFar.x);
  const vec2 gridHalfWidthHeight = mix(nearFarWidthHeight.xy, nearFarWidthHeight.zw, rayPos) * 0.5;

  const vec2 localPos = posVS.xy / gridHalfWidthHeight.xy;
  const uvec3 gridPos = uvec3(uvec2((localPos.xy * 0.5 + 0.5) * gridRes.xy), gridDepthIdx);
  return clampGridPos(gridPos);
}
