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
#include "ubos.inc.glsl"

#define KERNEL_RADIUS uboPerInstance.blurParams.x

PER_INSTANCE_DATA_BLUR;

layout (binding = 1) uniform sampler2D inputTex;
layout (binding = 2) uniform sampler2D depthBufferTex;

layout (location = 0) in vec2 inUV0;
layout (location = 0) out vec4 outColor;

const float sharpness = 9000.0;

vec4 blur(vec2 uv, float r, vec4 center_c, float center_d, inout float w_total)
{
  vec4  c = texture(inputTex, uv);
  float d = texture(depthBufferTex, uv).x;
 	d = linearizeDepth(d, uboPerInstance.camParams.x, uboPerInstance.camParams.y);
  
  const float blurSigma = KERNEL_RADIUS * 0.5;
  const float blurFalloff = 1.0 / (2.0*blurSigma*blurSigma);
  
  float ddiff = (d - center_d) * sharpness;
  float w = exp2(-r*r*blurFalloff - ddiff*ddiff);
  w_total += w;

  return c*w;
}

void main()
{
	vec4  center_c = texture(inputTex, inUV0);
  float center_d = texture(depthBufferTex, inUV0).x;
  center_d = linearizeDepth(center_d, uboPerInstance.camParams.x, uboPerInstance.camParams.y);

  vec2 resDir = textureSize(inputTex, 0).xy;
  resDir = 1.0 / resDir * uboPerInstance.blurParams.zw;
  
  vec4  c_total = center_c;
  float w_total = 1.0;
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = inUV0 + resDir * r;
    c_total += blur(uv, r, center_c, center_d, w_total);  
  }
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = inUV0 - resDir * r;
    c_total += blur(uv, r, center_c, center_d, w_total);  
  }

  outColor = c_total/w_total;
}
