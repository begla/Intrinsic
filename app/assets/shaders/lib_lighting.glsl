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

#define PSSM_SPLIT_COUNT 4u
#define MAX_SHADOW_MAP_COUNT 16u

#define ESM_POSITIVE_EXPONENT 300.0
#define ESM_NEGATIVE_EXPONENT 80.0
#define ESM_EXPONENTS vec2(ESM_POSITIVE_EXPONENT, ESM_NEGATIVE_EXPONENT)

// Clustered lighting
// ->

// Have to match the values on C++ side
const uint maxLightCountPerCluster = 128;
const float gridDepth = 10000.0f;
const uvec3 gridRes = uvec3(16u, 8u, 24u);
const float gridDepthSliceScale =
    gridDepth * (1.0f / ((gridRes.z - 1u) * (gridRes.z - 1u)));
const float gridDepthSliceScaleRcp = 1.0f / gridDepthSliceScale;

uint calcClusterIndex(uvec3 gridPos)
{
  return gridPos.x * maxLightCountPerCluster +
         gridPos.y * gridRes.x * maxLightCountPerCluster +
         gridPos.z * gridRes.y * gridRes.x * maxLightCountPerCluster;
}

float calcGridDepthSlice(uint depthSliceIdx)
{
  return (depthSliceIdx * depthSliceIdx) * gridDepthSliceScale;
}

uint calcGridDepthIndex(float depthVS)
{
  return uint(sqrt(depthVS * gridDepthSliceScaleRcp));
}

uvec3 calcGridPosForViewPos(vec3 posVS, vec4 nearFar, vec4 nearFarWidthHeight)
{
  const uint gridDepthIdx = calcGridDepthIndex(-posVS.z);
  const float gridStartDepth = calcGridDepthSlice(gridDepthIdx);
  const float gridEndDepth = calcGridDepthSlice(gridDepthIdx + 1);

  const float rayPos = (gridEndDepth - nearFar.x) / (nearFar.y - nearFar.x);
  const vec2 gridHalfWidthHeight = mix(nearFarWidthHeight.xy, nearFarWidthHeight.zw, rayPos) * 0.5;

  const vec2 localPos = posVS.xy / gridHalfWidthHeight.xy;
  return uvec3(uvec2((localPos.xy * 0.5 + 0.5) * gridRes.xy), gridDepthIdx);
}

// <-

struct Light
{
  vec4 posAndRadius;
  vec4 color;
};

// <-
struct LightingData
{
  // Source
  vec3 L;
  vec3 V;
  vec3 N;
  vec3 energy;

  vec3 baseColor;
  float metalMask;
  float specular;
  float roughness;
  
  // Result
  vec3 diffuseColor;
  vec3 specularColor;

  vec3 H;
  float NdL;
  float NdV;
  float NdH;
  float VdH;
  float roughness2;
};

vec3 F_Schlick(vec3 specularColor, float VdH)
{
  float fc = pow(1.0 - VdH, 5.0);
  return clamp(50.0 * specularColor.g, 0.0, 1.0) * fc + (1.0 - fc) * specularColor;
}

float V_SmithJointApprox(float NdV, float NdL, float roughness2)
{
  const float Vis_SmithV = NdL * (NdV * (1.0 - roughness2) + roughness2);
  const float Vis_SmithL = NdV * (NdL * (1.0 - roughness2) + roughness2);
  return 0.5 * (1.0 / (Vis_SmithV + Vis_SmithL + 1.0e-6));
}

float D_GGX(float NdH, float roughness2)
{
  const float r = roughness2 * roughness2;
  const float f = (NdH * r - NdH) * NdH + 1.0;
  return r / (MATH_PI * f * f);
}

void initLightingDataFromGBuffer(inout LightingData d)
{
  d.diffuseColor = d.baseColor - d.baseColor * d.metalMask;
  d.specularColor = mix(0.08 * d.specular.xxx, d.baseColor, d.metalMask);
  d.roughness2 = d.roughness * d.roughness;
}

void calculateLightingData(inout LightingData d)
{
  d.H = normalize(d.L + d.V);
  d.NdL = clamp(dot(d.N, d.L), 0.0, 1.0); 
  d.NdV = clamp(abs(dot(d.N, d.V)) + 1.0e-5, 0.0, 1.0);
  d.NdH = clamp(dot(d.N, d.H), 0.0, 1.0);
  d.VdH = clamp(dot(d.V, d.H), 0.0, 1.0);
}

vec3 calcDiffuse(LightingData d)
{
  return d.energy.x * (d.diffuseColor / MATH_PI);
}

vec3 calcSpecular(LightingData d)
{
  const vec3 F = F_Schlick(d.specularColor, d.VdH);
  const float Vis = V_SmithJointApprox(d.NdV, d.NdL, d.roughness2);
  const float D = D_GGX(d.NdH, d.roughness2) * d.energy.y;
  
  return D * F * Vis;
}

vec3 calcLighting(LightingData d)
{
  return (calcDiffuse(d) + calcSpecular(d)) * d.NdL;
}

float calcInverseSqrFalloff(float lightRadius, float dist)
{
  float a0 = dist / lightRadius;
  a0 = a0 * a0 * a0 * a0;
  const float dist2 = dist * dist;
  const float a1 = clamp(1.0 - a0, 0.0, 1.0);
  return a1 * a1 / (dist2 + 1.0);
}

// Shadows
// ->

vec4 calcPosLS(vec3 posVS, uint shadowMapIdx, mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT])
{
  vec4 posLS = shadowViewProjMatrix[shadowMapIdx] * vec4(posVS, 1.0);

  posLS /= posLS.w;
  posLS.xy *= 0.5;
  posLS.xy += 0.5;

  const float splitBias[] = { 0.000025, 0.0002, 0.001, 0.0025 };
  posLS.z -= splitBias[shadowMapIdx];

  return posLS;
}

uint findBestFittingSplit(vec3 posVS, out vec4 posLS, mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT])
{
  for (uint i=0u; i<PSSM_SPLIT_COUNT; ++i)
  {
    posLS = calcPosLS(posVS, i, shadowViewProjMatrix);

    if (posLS.x <= 1.0 && posLS.x >= 0.0
      && posLS.y <= 1.0 && posLS.y >= 0.0)
    {
      return i;
    }
  }

  return uint(-1);
}

// Ordinary shadow mapping / PCF
// ->

float sampleShadowMap(vec2 base_uv, float u, float v, vec2 shadowMapSizeInv, float depth, uint shadowMapIdx, sampler2DArrayShadow shadowTex) 
{
  const vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
  return texture(shadowTex, vec4(uv, shadowMapIdx, depth));
} 

float witnessPCF(vec3 shadowPos, uint shadowMapIdx, sampler2DArrayShadow shadowTex)
{
  float lightDepth = shadowPos.z;

  const vec2 shadowMapSize = vec2(textureSize(shadowTex, 0));
  const vec2 shadowMapSizeInv = 1.0 / shadowMapSize;

  vec2 uv = shadowPos.xy * shadowMapSize;

  vec2 base_uv;
  base_uv.x = floor(uv.x + 0.5);
  base_uv.y = floor(uv.y + 0.5);

  float s = (uv.x + 0.5 - base_uv.x);
  float t = (uv.y + 0.5 - base_uv.y);

  base_uv -= vec2(0.5, 0.5);
  base_uv *= shadowMapSizeInv;

  float sum = 0;

  float uw0 = (4 - 3 * s);
  float uw1 = 7;
  float uw2 = (1 + 3 * s);

  float u0 = (3 - 2 * s) / uw0 - 2;
  float u1 = (3 + s) / uw1;
  float u2 = s / uw2 + 2;

  float vw0 = (4 - 3 * t);
  float vw1 = 7;
  float vw2 = (1 + 3 * t);

  float v0 = (3 - 2 * t) / vw0 - 2;
  float v1 = (3 + t) / vw1;
  float v2 = t / vw2 + 2;

  sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);

  sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);

  sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv, lightDepth, shadowMapIdx, shadowTex);

  return sum * 1.0 / 144.0;
}

float calcShadowAttenuation(vec3 posVS, mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT], sampler2DArrayShadow shadowTex)
{
  float shadowAttenuation = 1.0;

  vec4 posLS;
  const uint shadowMapIdx = findBestFittingSplit(posVS, posLS, shadowViewProjMatrix);

  if (shadowMapIdx != uint(-1))
  {
    const float fadeDist = 0.1;
    const float fadeStart = 1.0 - fadeDist;

    const float fade = max(clamp(length(posLS.xy * 2.0 -1.0), 0.0, 1.0) - fadeStart, 0.0) / fadeDist;
    float shadowAttenuation2 = 1.0;

    if (shadowMapIdx < PSSM_SPLIT_COUNT - 1)
    {
      shadowAttenuation2 = witnessPCF(calcPosLS(posVS, shadowMapIdx + 1, shadowViewProjMatrix).xyz, shadowMapIdx + 1, shadowTex);
    }

    shadowAttenuation = mix(
      witnessPCF(posLS.xyz, shadowMapIdx, shadowTex), 
      shadowAttenuation2, fade);
  }

  return shadowAttenuation;
}

// Exponential shadow maps
// ->

vec2 warpDepth(float depth)
{
  depth = 2.0 * depth - 1.0;
  const float pos = exp(ESM_POSITIVE_EXPONENT * depth);
  const float neg = -exp(-ESM_NEGATIVE_EXPONENT * depth);
  return vec2(pos, neg);
}

float chebyshev(vec2 moments, float mean, float minVariance)
{
  // Compute variance
  float variance = moments.y - (moments.x * moments.x);
  variance = max(variance, minVariance);

  // Compute probabilistic upper bound
  float d = mean - moments.x;
  float pMax = variance / (variance + (d * d));

  // One-tailed Chebyshev
  return (mean <= moments.x ? 1.0f : pMax);
}

float calculateShadowEVSM(vec4 moments, float shadowDepth)
{
  vec2 posMoments = vec2(moments.x, moments.z);
  vec2 negMoments = vec2(moments.y, moments.w);
  vec2 warpedDepth = warpDepth(shadowDepth);

  vec2 depthScale = 0.0001 * ESM_EXPONENTS * warpedDepth;
  vec2 minVariance = depthScale * depthScale;
  float posResult = chebyshev(posMoments, warpedDepth.x, minVariance.x);
  float negResult = chebyshev(negMoments, warpedDepth.y, minVariance.y);
  return min(posResult, negResult);
}

float calculateShadowESM(vec4 moments, float shadowDepth)
{
  const vec2 warpedDepth = warpDepth(shadowDepth);
  return clamp(moments.x / warpedDepth.x, 0.0, 1.0);
}
