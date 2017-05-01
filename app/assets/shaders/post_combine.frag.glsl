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

layout (binding = 0) uniform PerInstance
{
  ivec4 data0;
} uboPerInstance;

layout (binding = 1) uniform sampler2D sceneTex;
layout (binding = 2) uniform sampler2D sceneBlurredTex;
layout (binding = 3) uniform sampler2D bloomTex;
layout (binding = 4) uniform sampler2D lensDirtTex;
layout (binding = 5) uniform sampler2D filmGrainTex;
layout (binding = 6) uniform sampler2D lensFlareTex;
layout (binding = 7) uniform sampler2D depthBufferTex;
layout (binding = 8) buffer AvgLum
{
  float avgLum[];
};

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

float A = 0.22;
float B = 0.3;
float C = 0.10;
float D = 0.20;
float E = 0.01;
float F = 0.33;
float W = 11.2;

vec3 tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 sampleColorOffsets(sampler2D scene, vec2 uv0, vec3 offsets, vec2 framebufferSize)
{
  const vec2 direction = uv0 * 2.0 - 1.0;
  offsets *= clamp(length(direction) - 0.3, 0.0, 1.0);

  const vec2 pixelSize = 1.0 / framebufferSize;

  return vec3(textureLod(scene, uv0 - direction * offsets.x * pixelSize, 0.0).r, 
    textureLod(scene, uv0 - direction * offsets.y * pixelSize, 0.0).g, 
    textureLod(scene, uv0 - direction * offsets.z * pixelSize, 0.0).b);
}

void main()
{
  const vec2 framebufferSize = vec2(textureSize(sceneTex, 0));

  // Chromatic abberation
  const vec3 colorOffsets = 5.0 * vec3(1, 2, 3);
  vec3 scene = sampleColorOffsets(sceneTex, inUV0, colorOffsets, framebufferSize);

  const vec3 sceneBlurred = textureLod(sceneBlurredTex, inUV0, 0.0).rgb;
  const float depth = textureLod(depthBufferTex, inUV0, 0.0).r;

  // TODO
  const vec4 camParams = vec4(1.0, 10000.0, 1.0, 1.0 / 10000.0);
  const float linDepth = linearizeDepth(depth, camParams.x, camParams.y) * camParams.y;

  // DoF fake
  const float blurFactor = clamp((linDepth - 500.0) / 50.0, 0.0, 1.0);
  scene = mix(scene, sceneBlurred, blurFactor);

  const vec4 bloom = textureLod(bloomTex, inUV0, 0.0).rgba;
  const vec4 lensDirt = textureLod(lensDirtTex, inUV0, 0.0).rgba;
  const vec4 lensFlare = textureLod(lensFlareTex, inUV0, 0.0).rgba;

  const float lensDirtLumThreshold = 0.2;
  const float lensDirtIntens = 0.9;
  const float toneMappingLumTarget = 0.7;
  const float toneMappingMaxExposure = 3.0;
  const float bloomFactor = 1.0;
  const float filmGrainBias = 0.0;
  const float filmGrainMax = 1.0;
  const float lensFlareFactor = 1.0;

  const float exposure = min(toneMappingLumTarget / avgLum[0], toneMappingMaxExposure);

  // Bloom
  outColor = vec4(scene.rgb + bloomFactor * bloom.rgb, 1.0);
  // Lens flares
  outColor.rgb += lensFlare.rgb * lensFlareFactor * lensDirt.rgb;
  // Lens dirt
  outColor.rgb += lensDirtIntens * lensDirt.rgb * clamp(max(bloom.a - lensDirtLumThreshold, 0.0), 0.0, 1.0);
  // Vignette
  outColor.rgb *= 1.0 - clamp(length(inUV0 * 2.0 - 1.0) - 0.5, 0.0, 1.0);

  // Film grain
  const vec4 grain = texelFetch(filmGrainTex, 
    (ivec2(inUV0 * framebufferSize.xy) + uboPerInstance.data0.xy) & 255, 0).rgba;
  outColor.rgb = grain.rgb * min(outColor.rgb + filmGrainBias, filmGrainMax) + outColor.rgb;

  // Tonemapping
  outColor.rgb *= exposure;
  outColor.rgb = tonemap(outColor.rgb);
  vec3 whiteScale = 1.0/tonemap(vec3(W));
  outColor.rgb *= whiteScale;

  // Bleach Bypass
  {
    vec3 lumCoeff = vec3(0.25, 0.65, 0.1);
    float lum = dot(lumCoeff, outColor.rgb);
    vec3 blend = vec3(lum);
    float L = min(1.0, max(0.0, 10.0 * (lum - 0.45)));
    vec3 result1 = 2.0 * outColor.rgb * blend;
    vec3 result2 = 1.0 - 2.0 * (1.0 - blend) * (1.0 - outColor.rgb);

    const float strength = 0.5;
    outColor.rgb = mix(outColor.rgb, mix(result1, result2, L), strength);
  }
}
 