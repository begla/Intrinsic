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

#define BLUR_WIDTH 9
#define HALF_BLUR_WIDTH 4

#define BLUR_THREADS_X 64
#define BLUR_THREADS_Y 4

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

layout (local_size_x = BLUR_THREADS_X, local_size_y = 1) in;
void main()
{
  vec4 in0[BLUR_WIDTH + BLUR_THREADS_Y];
  ivec2 base = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y * BLUR_THREADS_Y); 
  
  for(int i = 0; i < BLUR_WIDTH + BLUR_THREADS_Y; i++)
  {
    in0[i] = texelFetch(inTex, base + ivec2(0, i-HALF_BLUR_WIDTH), uboPerInstance.mipLevel.x);
  }
  
  for(int y = 0; y < BLUR_THREADS_Y; y++)
  {
    vec4 out0 = vec4(0.0);

    for(int i = 0; i < BLUR_WIDTH; i++)
    {
      out0 += in0[y+i] * blurWeights[i];
    }

    imageStore(outTex, base.xy + ivec2(0, y), out0);
  } 
}
