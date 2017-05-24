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

#define BLUR_WIDTH 9
#define HALF_BLUR_WIDTH 4

#define BLUR_THREADS 128

const float blurWeights[] = {
    0.004815026,
    0.028716039,
    0.102818575,
    0.221024189,
    0.28525234,
    0.221024189,
    0.102818575,
    0.028716039,
    0.004815026
};

layout (binding = 0) uniform PerInstance
{
  ivec4 mipLevel;
} uboPerInstance;

layout (binding = 1, rgba16f) uniform image2D outTex;
layout (binding = 2) uniform sampler2D inTex;

shared vec4 temp[BLUR_THREADS];

layout (local_size_x = BLUR_THREADS, local_size_y = 1) in;
void main()
{
  ivec2 coord = ivec2(gl_LocalInvocationIndex - HALF_BLUR_WIDTH + (BLUR_THREADS - HALF_BLUR_WIDTH * 2) * gl_WorkGroupID.x, gl_WorkGroupID.y);
  temp[gl_LocalInvocationIndex] = texelFetch(inTex, coord, uboPerInstance.mipLevel.x);

  barrier();
  
  if (gl_LocalInvocationIndex >= HALF_BLUR_WIDTH &&
        gl_LocalInvocationIndex < (BLUR_THREADS - HALF_BLUR_WIDTH))
  {
    vec4 out0 = vec4(0.0);

    for (int i = -HALF_BLUR_WIDTH; i <= HALF_BLUR_WIDTH; ++i)
      out0 += temp[gl_LocalInvocationIndex + i] * blurWeights[i + HALF_BLUR_WIDTH];

    imageStore(outTex, ivec2(gl_LocalInvocationIndex - HALF_BLUR_WIDTH 
      + (BLUR_THREADS - HALF_BLUR_WIDTH * 2) * gl_WorkGroupID.x, gl_GlobalInvocationID.y), out0);
  }
}
