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

#define VOLUME_DEPTH 128
#define VOLUME_DIMENSIONS vec3(160.0, 90.0, VOLUME_DEPTH)
#define VOLUMETRIC_LIGHTING_RANGE_NEAR 1.0
#define VOLUMETRIC_LIGHTING_RANGE_FAR 1024.0
#define VOLUMETRIC_LIGHTING_RANGE (VOLUMETRIC_LIGHTING_RANGE_FAR - VOLUMETRIC_LIGHTING_RANGE_NEAR)
#define DEPTH_EXP 2.0

vec3 screenSpacePosToCellIndex(vec3 ssPos)
{
  return clamp(ssPos, 0.0, 1.0) * vec3((VOLUME_DIMENSIONS.x - 1) / VOLUME_DIMENSIONS.x, 
    (VOLUME_DIMENSIONS.y - 1) / VOLUME_DIMENSIONS.y, (VOLUME_DIMENSIONS.z - 1) / VOLUME_DIMENSIONS.z) 
  + vec3(0.5 / VOLUME_DIMENSIONS.x, 0.5 / VOLUME_DIMENSIONS.y, 0.5 / VOLUME_DIMENSIONS.z);
}

vec3 screenToViewSpacePos(vec2 ssPos, vec3 eyeX, vec3 eyeY, vec3 eyeZ, float linDepth, mat4 projMatrix)
{
  const vec3 eyeRay = eyeX.xyz * ssPos.xxx + eyeY.xyz * ssPos.yyy + eyeZ.xyz;
  return eyeRay * linDepth;
}

vec3 cellIndexToScreenSpacePos(vec3 cellIdx)
{
  return cellIdx * vec3(2.0 / VOLUME_DIMENSIONS.x, 2.0 / VOLUME_DIMENSIONS.y, 1.0 / VOLUME_DIMENSIONS.z) + vec3(-1.0, -1.0, 0.0);
}

float volumeZToDepth(float volumePosZ)
{
    return pow(abs(volumePosZ), DEPTH_EXP) * VOLUMETRIC_LIGHTING_RANGE + VOLUMETRIC_LIGHTING_RANGE_NEAR;
}

float depthToVolumeZ(float depth)
{
    return pow(abs((depth - VOLUMETRIC_LIGHTING_RANGE_NEAR) / VOLUMETRIC_LIGHTING_RANGE), 1.0 / DEPTH_EXP);
}
