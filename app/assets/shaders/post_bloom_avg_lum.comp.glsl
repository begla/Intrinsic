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

#define AVG_LUM_THREADS 16u
#define LUM_AND_BRIGHT_THREADS 8u

layout (binding = 0) uniform PerInstance
{
  uvec4 dim;
  vec4 data;
} uboPerInstance;

layout (binding = 1, r32f) uniform image2D inOutLumTex;
layout (binding = 2) buffer AvgLum
{
  float avgLum[];
};

shared float temp0[AVG_LUM_THREADS];

layout (local_size_x = AVG_LUM_THREADS, local_size_y = 1) in;
void main()
{
  float totalLuminance = 0.0f;

  for(uint i=0; i<ceil(float(uboPerInstance.dim.x) / AVG_LUM_THREADS); ++i)
  {
    for(uint j=0; j<uboPerInstance.dim.y; ++j)
    {
      uint x = gl_GlobalInvocationID.x + AVG_LUM_THREADS * i;

      if (x < uboPerInstance.dim.x)
      {
        totalLuminance += imageLoad(inOutLumTex, ivec2(x, j)).x;   
      }
    }
  }

  temp0[gl_GlobalInvocationID.x] = totalLuminance;

  groupMemoryBarrier();
  barrier();

  if(gl_GlobalInvocationID.x == 0)
  {
    for(uint i=1; i<AVG_LUM_THREADS; ++i)
    {
      totalLuminance += temp0[i];
    }

    const float blendFactor = uboPerInstance.data.y > 0.0 ? uboPerInstance.data.y : uboPerInstance.data.x * 1.0;
    avgLum[0] = (1.0 - blendFactor) * avgLum[0] + blendFactor * (totalLuminance / (uboPerInstance.dim.x * uboPerInstance.dim.y));
  }
}
