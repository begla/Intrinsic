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
layout (binding = 2) uniform sampler2D bloomTex;
layout (binding = 3) uniform sampler2D lensDirtTex;
layout (binding = 4) uniform sampler2D filmGrainTex;
layout (binding = 5) uniform sampler2D lensFlareTex;
layout (binding = 6) buffer AvgLum
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

void main()
{
  const vec2 framebufferSize = vec2(textureSize(sceneTex, 0));

  const vec4 scene = textureLod(sceneTex, inUV0, 0.0).rgba;
  const vec4 bloom = textureLod(bloomTex, inUV0, 0.0).rgba;
  const vec4 lensDirt = textureLod(lensDirtTex, inUV0, 0.0).rgba;
  const vec4 lensFlare = textureLod(lensFlareTex, inUV0, 0.0).rgba;

  const float lensDirtLumThreshold = 0.3;
  const float lensDirtIntens = 0.75;
  const float toneMappingLumTarget = 0.4;
  const float toneMappingMaxExposure = 3.0;
  const float bloomFactor = 0.5;
  const float filmGrainBias = 0.0;
  const float filmGrainMax = 0.5;
  const float lensFlareFactor = 0.5;

  const float exposure = min(toneMappingLumTarget / avgLum[0], toneMappingMaxExposure);

  // Bloom
  outColor = vec4(scene.rgb + bloomFactor * bloom.rgb, 1.0);
  // Lens flares
  outColor.rgb += lensFlare.rgb * lensFlareFactor;
  // Lens dirt
  outColor.rgb += lensDirtIntens * lensDirt.rgb * clamp(max(bloom.a - lensDirtLumThreshold, 0.0), 0.0, 1.0);

  // Film grain
  const vec4 grain = texelFetch(filmGrainTex, 
    (ivec2(inUV0 * framebufferSize.xy) + uboPerInstance.data0.xy) & 255, 0).rgba;
  outColor.rgb = grain.rgb * min(outColor.rgb + filmGrainBias, filmGrainMax) + outColor.rgb;

  // Tonemapping
  outColor.rgb *= exposure;
  outColor.rgb = tonemap(outColor.rgb);

  vec3 whiteScale = 1.0/tonemap(vec3(W, W, W));
  outColor.rgb *= whiteScale;
}
 