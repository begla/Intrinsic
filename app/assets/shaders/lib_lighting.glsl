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

#define PSSM_SPLIT_COUNT 4u
#define MAX_SHADOW_MAP_COUNT 4u
#define EPSILON 1.0e-6
#define IBL_MIN_ROUGHNESS 0.05
#define IBL_MIP_COUNT 9

#define ESM_POSITIVE_EXPONENT 300.0
#define ESM_NEGATIVE_EXPONENT 20.0
#define ESM_EXPONENTS vec2(ESM_POSITIVE_EXPONENT, ESM_NEGATIVE_EXPONENT)

// <-

const float translDistortion = 0.2;
const float translPower = 8.0;
const float translScale = 1.0;
const vec3 translAmbient = vec3(0.0);

// <-

struct SH9
{
  vec3 L[9];
};

SH9 decodeSH9(vec4 data[7])
{
  SH9 result;
  result.L[0] = data[0].xyz;

  result.L[1] = vec3(data[0].w, data[1].xy);
  result.L[2] = vec3(data[1].zw, data[2].x);
  result.L[3] = data[2].yzw;

  result.L[4] = data[3].xyz;
  result.L[5] = vec3(data[3].w, data[4].xy);
  result.L[6] = vec3(data[4].zw, data[5].x);
  result.L[7] = data[5].yzw;
  result.L[8] = data[6].xyz;

  return result;
}

// <-

struct LightingData
{
  // Source
  vec3 L;
  vec3 LSpec;
  vec3 V;
  vec3 N;
  float energy;
  vec3 posVS;

  vec3 baseColor;
  float metalMask;
  float specular;
  float roughness;

  // Result
  float invDistToLight;

  vec3 diffuseColor;
  vec3 specularColor;
  vec2 lobeEnergy;

  vec3 R;
  vec3 H;
  float NdL;
  float NdV;
  float NdH;
  float VdH;
  float LdH;
  float roughness2;
};

float F_Schlick(float value, float VdH)
{
  const float fc = pow(1.0 - VdH, 5.0);
  return fc + value * (1.0 - fc);
}

vec3 F_Schlick(vec3 value, float VdH)
{
  const float fc = pow(1.0 - VdH, 5.0);
  return fc + value * (1.0 - fc);
}

float V_GGX(float NdL, float NdV, float roughness2)
{
  float k = roughness2 * 0.5;
  float V = NdV * (1.0 - k) + k;
  float L = NdL * (1.0 - k) + k;
  return 0.25 / (V * L);
}

float D_GGX(float NdH, float roughness2)
{
  const float roughness4 = roughness2 * roughness2;
  const float denom = NdH * NdH * (roughness4 - 1.0) + 1.0;
  return roughness4 / (MATH_PI * denom * denom);
}

// IBL
// ->

float roughnessToMipIdx(float roughness)
{
  return max(roughness - IBL_MIN_ROUGHNESS, 0.0) / (1.0 - IBL_MIN_ROUGHNESS) *
         (IBL_MIP_COUNT - 1);
}

// Irradiance / SH

vec3 shDotProduct(in SH9 a, in SH9 b)
{
  vec3 result = vec3(0.0);
  result += a.L[0] * b.L[0];
  result += a.L[1] * b.L[1];
  result += a.L[2] * b.L[2];
  result += a.L[3] * b.L[3];
  result += a.L[4] * b.L[4];
  result += a.L[5] * b.L[5];
  result += a.L[6] * b.L[6];
  result += a.L[7] * b.L[7];
  result += a.L[8] * b.L[8];

  return result;
}

SH9 projectOntoSH9(in vec3 n, in vec3 color, in float A0, in float A1,
                   in float A2)
{
  SH9 sh;

  // Band 0
  sh.L[0] = 0.282095 * A0 * color;

  // Band 1
  sh.L[1] = 0.488603 * n.y * A1 * color;
  sh.L[2] = 0.488603 * n.z * A1 * color;
  sh.L[3] = 0.488603 * n.x * A1 * color;

  // Band 2
  sh.L[4] = 1.092548 * n.x * n.y * A2 * color;
  sh.L[5] = 1.092548 * n.y * n.z * A2 * color;
  sh.L[6] = 0.315392 * (3.0 * n.z * n.z - 1.0) * A2 * color;
  sh.L[7] = 1.092548 * n.x * n.z * A2 * color;
  sh.L[8] = 0.546274 * (n.x * n.x - n.y * n.y) * A2 * color;

  return sh;
}

const float cosineA0 = MATH_PI;
const float cosineA1 = (2.0 * MATH_PI) / 3.0;
const float cosineA2 = (0.25 * MATH_PI);

vec3 sampleSH(vec4 data[7], vec3 dir)
{
  dir = normalize(dir);
  SH9 sh = decodeSH9(data);
  SH9 dirSH = projectOntoSH9(dir, vec3(1.0), cosineA0, cosineA1, cosineA2);
  return max(shDotProduct(dirSH, sh), 0.0);
}

// <-

void initLightingDataFromGBuffer(inout LightingData d)
{
  const float adjRough =
      d.roughness * (1.0 - IBL_MIN_ROUGHNESS) + IBL_MIN_ROUGHNESS;
  d.roughness2 = adjRough * adjRough;

  d.diffuseColor = d.baseColor - d.baseColor * d.metalMask;
  d.specularColor = mix(0.08 * d.specular.xxx, d.baseColor, d.metalMask);
}

void calculateLightingDataBase(inout LightingData d)
{
  d.V = -normalize(d.posVS);
  d.R = reflect(-d.V, d.N);
  d.lobeEnergy = vec2(d.energy);
}

void calculateLightingData(inout LightingData d)
{
  d.H = normalize(d.L + d.V);
  d.NdL = clamp(dot(d.N, d.L), 0.0, 1.0);
  d.NdV = clamp(abs(dot(d.N, d.V)) + 1.0e-5, 0.0, 1.0);
  d.NdH = clamp(dot(d.N, d.H), 0.0, 1.0);
  d.VdH = clamp(dot(d.V, d.H), 0.0, 1.0);
  d.LdH = clamp(dot(d.L, d.H), 0.0, 1.0);
}

void calculateDiscL(inout LightingData d, float discRadius)
{
  const float discR = sin(discRadius);
  const float discD = cos(discRadius);

  const float DdR = dot(d.L, d.R);
  const vec3 S = d.R - DdR * d.L;

  d.L = DdR < discD ? normalize(discD * d.L + normalize(S) * discR) : d.R;
  d.invDistToLight = 1.0 / discD;
}

void calculateSpehereL(inout LightingData d, float sphereRadius,
                       vec3 lightDistVec)
{
  const vec3 closestPointRay = dot(lightDistVec, d.R) * d.R;
  const vec3 centerToRay = closestPointRay - lightDistVec;
  const vec3 cloestPointSphere =
      lightDistVec +
      centerToRay *
          clamp(sphereRadius * (1.0 / sqrt(dot(centerToRay, centerToRay))), 0.0,
                1.0);

  d.L = normalize(cloestPointSphere);
  d.invDistToLight = 1.0 / length(lightDistVec);
}

void calculateLobeEnergySphere(inout LightingData d, float sphereRadius)
{
  const float sphereAngle = clamp(sphereRadius * d.invDistToLight, 0.0, 1.0);
  float energ =
      d.roughness2 / clamp(d.roughness2 + 0.5 * sphereAngle, 0.0, 1.0);
  energ *= energ;
  d.lobeEnergy *= energ;
}

vec3 calcDiffuse(LightingData d)
{
  return d.lobeEnergy.x * d.diffuseColor * (1.0 / MATH_PI);
}

vec3 calcSpecular(LightingData d)
{
  const float D = D_GGX(d.NdH, d.roughness2) * d.lobeEnergy.y;
  const float V = V_GGX(d.NdL, d.NdV, d.roughness2);
  const float FHelper = F_Schlick(d.roughness2, d.VdH);
  const vec3 F = clamp(50.0 * d.specularColor.g, 0.0, 1.0) * FHelper +
                 (1.0 - FHelper) * d.specularColor;

  return D * F * V;
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

vec3 kelvinToRGB(float kelvin, sampler2D kelvinLutTex)
{
  return texture(kelvinLutTex, vec2(max(kelvin - 1000.0, 0.0) / 30000.0, 0.0))
      .rgb;
}

// Shadows
// ->

vec4 calcPosLS(vec3 posVS, uint shadowMapIdx,
               in mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT])
{
  vec4 posLS = shadowViewProjMatrix[shadowMapIdx] * vec4(posVS, 1.0);

  posLS.xyz /= posLS.www;
  posLS.xy = posLS.xy * 0.5 + 0.5;

  return posLS;
}

uint findBestFittingSplit(vec3 posVS, out vec4 posLS,
                          in mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT])
{
  for (uint i = 0u; i < PSSM_SPLIT_COUNT; ++i)
  {
    posLS = calcPosLS(posVS, i, shadowViewProjMatrix);

    if (posLS.x <= 1.0 && posLS.x >= 0.0 && posLS.y <= 1.0 && posLS.y >= 0.0)
    {
      return i;
    }
  }

  return uint(-1);
}

// Ordinary shadow mapping / PCF
// ->

float sampleShadowMap(vec2 base_uv, float u, float v, vec2 shadowMapSizeInv,
                      float depth, uint shadowMapIdx,
                      sampler2DArrayShadow shadowTex)
{
  const vec2 uv = base_uv + vec2(u, v) * shadowMapSizeInv;
  return texture(shadowTex, vec4(uv, shadowMapIdx, depth));
}

float witnessPCF(vec3 shadowPos, uint shadowMapIdx,
                 sampler2DArrayShadow shadowTex)
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

  sum += uw0 * vw0 * sampleShadowMap(base_uv, u0, v0, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw0 * sampleShadowMap(base_uv, u1, v0, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw0 * sampleShadowMap(base_uv, u2, v0, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);

  sum += uw0 * vw1 * sampleShadowMap(base_uv, u0, v1, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw1 * sampleShadowMap(base_uv, u1, v1, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw1 * sampleShadowMap(base_uv, u2, v1, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);

  sum += uw0 * vw2 * sampleShadowMap(base_uv, u0, v2, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw1 * vw2 * sampleShadowMap(base_uv, u1, v2, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);
  sum += uw2 * vw2 * sampleShadowMap(base_uv, u2, v2, shadowMapSizeInv,
                                     lightDepth, shadowMapIdx, shadowTex);

  return sum * 1.0 / 144.0;
}

float calcShadowAttenuation(vec3 posVS,
                            in mat4 shadowViewProjMatrix[MAX_SHADOW_MAP_COUNT],
                            sampler2DArrayShadow shadowTex)
{
  float shadowAttenuation = 1.0;

  vec4 posLS;
  const uint shadowMapIdx =
      findBestFittingSplit(posVS, posLS, shadowViewProjMatrix);

  if (shadowMapIdx != uint(-1))
  {
    const float fadeDist = 0.1;
    const float fadeStart = 1.0 - fadeDist;

    const float fade =
        max(clamp(length(posLS.xy * 2.0 - 1.0), 0.0, 1.0) - fadeStart, 0.0) /
        fadeDist;
    float shadowAttenuation2 = 1.0;

    if (shadowMapIdx < PSSM_SPLIT_COUNT - 1)
    {
      shadowAttenuation2 = witnessPCF(
          calcPosLS(posVS, shadowMapIdx + 1, shadowViewProjMatrix).xyz,
          shadowMapIdx + 1, shadowTex);
    }

    shadowAttenuation = mix(witnessPCF(posLS.xyz, shadowMapIdx, shadowTex),
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

float calculateShadowESM(vec4 moments, float shadowDepth)
{
  const vec2 warpedDepth = warpDepth(shadowDepth);
  return clamp(moments.x / warpedDepth.x, 0.0, 1.0);
}
