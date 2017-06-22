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

// ->
// Based on HBAO by Louis Bavoil and Miguel Sainz / NVIDIA
// <-

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "ubos.inc.glsl"

PER_INSTANCE_DATA_SSAO_HBAO;

layout(binding = 1) uniform sampler2D depthBufferTex;
layout(binding = 2) uniform sampler2D noiseTex;

layout(location = 0) in vec2 inUV0;
layout(location = 0) out vec4 outColor;

const uint numDirections = 6;
const uint numSamples = 4;
const float strength = 1.5;
const float minAo = 0.0;
const float R = 4.0;
const float R2 = R * R;
const float negInvR2 = -1.0 / R2;
const float maxRadiusPixels = 5.0;
const float tanBias = tan(5.0 * MATH_PI / 180.0);

// TODO
const float focalLen = 1.0 / tan(MATH_PI * 0.25) * 0.5625;

vec3 fetchPosVS(vec2 uv0)
{
  return unproject(uv0, texture(depthBufferTex, uv0).r,
                   uboPerInstance.invProjMatrix);
}

float length2(vec3 V) { return dot(V, V); }

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (length2(V1) < length2(V2)) ? V1 : V2;
}

float tanToSin(float x) { return x * inversesqrt(x * x + 1.0); }

void computeSteps(inout vec2 stepSizeUv, inout float numSteps,
                  float rayRadiusPix, float rand, vec4 res)
{
  // Avoid oversampling if numSteps is greater than the kernel radius in pixels
  numSteps = min(numSamples, rayRadiusPix);

  // Divide by Ns+1 so that the farthest samples are not fully attenuated
  float stepSizePix = rayRadiusPix / (numSteps + 1);

  // Clamp numSteps if it is greater than the max kernel footprint
  float maxNumSteps = maxRadiusPixels / stepSizePix;
  if (maxNumSteps < numSteps)
  {
    // Use dithering to avoid AO discontinuities
    numSteps = floor(maxNumSteps + rand);
    numSteps = max(numSteps, 1);
    stepSizePix = maxRadiusPixels / numSteps;
  }

  // Step size in uv space
  stepSizeUv = stepSizePix * res.zw;
}

vec2 rotateDirections(vec2 dir, vec2 cosSin)
{
  return vec2(dir.x * cosSin.x - dir.y * cosSin.y,
              dir.x * cosSin.y + dir.y * cosSin.x);
}

vec2 snapUVOffset(vec2 uv, vec4 res) { return round(uv * res.xy) * res.zw; }

float invLength(vec2 V) { return inversesqrt(dot(V, V)); }

float biasedTangent(vec3 V) { return V.z * invLength(V.xy) + tanBias; }

float tangent(vec3 V) { return V.z * invLength(V.xy); }

float tangent(vec3 P, vec3 S) { return -(P.z - S.z) * invLength(S.xy - P.xy); }

float falloff(float d2) { return d2 * negInvR2 + 1.0f; }

float horizonOcclusion(vec2 deltaUV, vec3 P, vec3 dPdu, vec3 dPdv,
                       float randstep, float numSamples, vec4 res)
{
  float ao = 0;

  // Offset the first coord with some noise
  vec2 uv = inUV0 + snapUVOffset(randstep * deltaUV, res);
  deltaUV = snapUVOffset(deltaUV, res);

  // Calculate the tangent vector
  vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

  // Get the angle of the tangent vector from the viewspace axis
  float tanH = biasedTangent(T);
  float sinH = tanToSin(tanH);

  float tanS;
  float d2;
  vec3 S;

  // Sample to find the maximum angle
  for (float s = 1; s <= numSamples; ++s)
  {
    uv += deltaUV;
    S = fetchPosVS(uv);
    tanS = tangent(P, S);
    d2 = length2(S - P);

    // Is the sample within the radius and the angle greater?
    if (d2 < R2 && tanS > tanH)
    {
      float sinS = tanToSin(tanS);
      // Apply falloff based on the distance
      ao += falloff(d2) * (sinS - sinH);

      tanH = tanS;
      sinH = sinS;
    }
  }

  return ao;
}

void main()
{
  // TODO: Hardcoded half size target
  vec4 res = vec4(0.5 * textureSize(depthBufferTex, 0).xy, 0.0, 0.0);
  res.zw = 1.0 / res.xy;
  const vec2 noiseScale = textureSize(noiseTex, 0).xy / res.xy;

  vec3 P, Pr, Pl, Pt, Pb;
  P = fetchPosVS(inUV0);

  // Sample neighboring pixels
  Pr = fetchPosVS(inUV0 + vec2(res.z, 0.0));
  Pl = fetchPosVS(inUV0 + vec2(-res.z, 0));
  Pt = fetchPosVS(inUV0 + vec2(0.0, res.w));
  Pb = fetchPosVS(inUV0 + vec2(0.0, -res.w));

  // Calculate tangent basis vectors using the minimu difference
  vec3 dPdu = minDiff(P, Pr, Pl);
  vec3 dPdv = minDiff(P, Pt, Pb) * (res.y * res.z);

  // Get the random samples from the noise texture
  vec3 random = texture(noiseTex, inUV0.xy / noiseScale).rgb;

  // Calculate the projected size of the hemisphere
  float rayRadiusUV = 0.5 * R * focalLen / -P.z;
  float rayRadiusPx = rayRadiusUV * res.x;

  float ao = 1.0;

  // Make sure the radius of the evaluated hemisphere is more than a pixel
  if (rayRadiusPx > 1.0)
  {
    ao = 0.0;
    float numSteps;
    vec2 stepSizeUV;

    // Compute the number of steps
    computeSteps(stepSizeUV, numSteps, rayRadiusPx, random.z, res);

    float alpha = 2.0 * MATH_PI / numDirections;

    // Calculate the horizon occlusion of each direction
    for (float d = 0; d < numDirections; ++d)
    {
      float theta = alpha * d;

      // Apply noise to the direction
      vec2 dir = rotateDirections(vec2(cos(theta), sin(theta)), random.xy);
      vec2 deltaUV = dir * stepSizeUV;

      // Sample the pixels along the direction
      ao += horizonOcclusion(deltaUV, P, dPdu, dPdv, random.z, numSteps, res);
    }

    // Average the results and produce the final AO
    ao = 1.0 - ao / numDirections * strength;
  }

  outColor = vec4(clamp(ao, minAo, 1.0));
}
