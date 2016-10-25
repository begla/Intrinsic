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

#define LUM_WEIGHTS vec3(0.27, 0.67, 0.06)
#define DELTA 0.00001
#define LUM_AND_BRIGHT_THREADS 8u

layout (binding = 0) uniform PerInstance
{
  uvec4 dim;
  vec4 bloomThreshold;
} uboPerInstance;

layout (binding = 1, rgba16f) uniform image2D output0Tex;
layout (binding = 2, rgba16f) uniform image2D output1Tex;
layout (binding = 3, rgba16f) uniform image2D output2Tex;
layout (binding = 4, rgba16f) uniform image2D output3Tex;
layout (binding = 5, r32f) uniform image2D outputLumTex;

layout (binding = 6) uniform sampler2D input0Tex;

shared vec4 temp0[LUM_AND_BRIGHT_THREADS][LUM_AND_BRIGHT_THREADS];
shared vec4 temp1[LUM_AND_BRIGHT_THREADS][LUM_AND_BRIGHT_THREADS];

vec4 brightClampAndLuminance(vec4 s)
{
  const float maxLum = 500.0;
  s.rgb = min(s.rgb, maxLum);
  s.a = dot(s.rgb, LUM_WEIGHTS) + DELTA;
  s.rgb = max(s.rgb - uboPerInstance.bloomThreshold.x, vec3(0.0));
  return s;
}

layout (local_size_x = LUM_AND_BRIGHT_THREADS, local_size_y = LUM_AND_BRIGHT_THREADS) in;
void main()
{
  const vec2 samplePoint = (2.0 * vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(uboPerInstance.dim.x, uboPerInstance.dim.y);
  vec4 bright = vec4(0.0);
  const ivec2 outPos = 2 * ivec2(gl_GlobalInvocationID.xy);

  vec4 b = brightClampAndLuminance(textureLodOffset(input0Tex, samplePoint, 0.0, ivec2(0, 0)));
  imageStore(output0Tex, outPos, b);
  bright += b;

  b = brightClampAndLuminance(textureLodOffset(input0Tex, samplePoint, 0.0, ivec2(0, 2)));
  imageStore(output0Tex, outPos + ivec2(0, 1), b);
  bright += b;
  
  b = brightClampAndLuminance(textureLodOffset(input0Tex, samplePoint, 0.0, ivec2(2, 0)));
  imageStore(output0Tex, outPos + ivec2(1, 0), b);
  bright += b;
  
  b = brightClampAndLuminance(textureLodOffset(input0Tex, samplePoint, 0.0, ivec2(2, 2)));
  imageStore(output0Tex, outPos + ivec2(1, 1), b);
  bright += b;

  bright = bright / 4;
  temp0[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = bright;

  imageStore(output1Tex, ivec2(gl_LocalInvocationID.xy + gl_WorkGroupID.xy * LUM_AND_BRIGHT_THREADS), bright);
  
  groupMemoryBarrier();
  barrier();

  if(gl_LocalInvocationID.x < LUM_AND_BRIGHT_THREADS / 2 && gl_LocalInvocationID.y < LUM_AND_BRIGHT_THREADS / 2) 
  {
    vec4 nextLevel;
    nextLevel =  temp0[gl_LocalInvocationID.x * 2][gl_LocalInvocationID.y * 2];
    nextLevel += temp0[gl_LocalInvocationID.x * 2 + 1][gl_LocalInvocationID.y * 2];
    nextLevel += temp0[gl_LocalInvocationID.x * 2][gl_LocalInvocationID.y * 2 + 1];
    nextLevel += temp0[gl_LocalInvocationID.x * 2 + 1][gl_LocalInvocationID.y * 2 + 1];
    nextLevel = nextLevel / 4;
    
    temp1[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = nextLevel;
    imageStore(output2Tex, ivec2(gl_WorkGroupID.xy * LUM_AND_BRIGHT_THREADS / 2 + gl_LocalInvocationID.xy), nextLevel);
  }

  groupMemoryBarrier();
  barrier();

  if(gl_LocalInvocationID.x < LUM_AND_BRIGHT_THREADS / 4 && gl_LocalInvocationID.y < LUM_AND_BRIGHT_THREADS / 4)
  {
    vec4 nextLevel;
    nextLevel =  temp1[gl_LocalInvocationID.x * 2][gl_LocalInvocationID.y * 2];
    nextLevel += temp1[gl_LocalInvocationID.x * 2 + 1][gl_LocalInvocationID.y * 2];
    nextLevel += temp1[gl_LocalInvocationID.x * 2][gl_LocalInvocationID.y * 2 + 1];
    nextLevel += temp1[gl_LocalInvocationID.x * 2 + 1][gl_LocalInvocationID.y * 2 + 1];
    nextLevel = nextLevel / 4;

    temp0[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = nextLevel;
    imageStore(output3Tex, ivec2(gl_WorkGroupID.xy * LUM_AND_BRIGHT_THREADS / 4 + gl_LocalInvocationID.xy), nextLevel);
  }

  groupMemoryBarrier();
  barrier();

  if(gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0)
  {
    vec4 nextLevel;
    nextLevel =  temp0[0][0];
    nextLevel += temp0[1][0];
    nextLevel += temp0[0][1];
    nextLevel += temp0[1][1];
    nextLevel = nextLevel / 4;

    imageStore(outputLumTex, ivec2(gl_WorkGroupID.xy), vec4(nextLevel.a));
  }
}
