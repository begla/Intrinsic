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

#define PSSM_SPLIT_COUNT 4u

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

void initLightingData(inout LightingData d)
{
  d.diffuseColor = d.baseColor - d.baseColor * d.metalMask;
  d.specularColor = mix(0.08 * d.specular.xxx, d.baseColor, d.metalMask);

  d.H = normalize(d.L + d.V);
  d.NdL = clamp(dot(d.N, d.L), 0.0, 1.0); 
  d.NdV = clamp(abs(dot(d.N, d.V)) + 1.0e-5, 0.0, 1.0);
  d.NdH = clamp(dot(d.N, d.H), 0.0, 1.0);
  d.VdH = clamp(dot(d.V, d.H), 0.0, 1.0);
  d.roughness2 = d.roughness * d.roughness;
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

vec4 calcPosLS(vec3 posVS, uint shadowMapIdx, mat4 shadowViewProjMatrix[PSSM_SPLIT_COUNT])
{
  vec4 posLS = shadowViewProjMatrix[shadowMapIdx] * vec4(posVS, 1.0);

  posLS /= posLS.w;
  posLS.xy *= 0.5;
  posLS.xy += 0.5;

  const float splitBias[] = { 0.0001, 0.0001, 0.0005, 0.001 };
  posLS.z -= splitBias[shadowMapIdx];

  return posLS;
}

uint findBestFittingSplit(vec3 posVS, out vec4 posLS, mat4 shadowViewProjMatrix[PSSM_SPLIT_COUNT])
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

float calcShadowAttenuation(vec3 posVS, mat4 shadowViewProjMatrix[PSSM_SPLIT_COUNT], sampler2DArrayShadow shadowTex)
{
  float shadowAttenuation = 1.0;

  vec4 posLS;
  const uint shadowMapIdx = findBestFittingSplit(posVS, posLS, shadowViewProjMatrix);

  if (shadowMapIdx != uint(-1))
  {
    const float fadeDist = 0.05;
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
