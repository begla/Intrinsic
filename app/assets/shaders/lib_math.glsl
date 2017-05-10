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

#define MATH_PI 3.14159
#define LUM_WEIGHTS vec3(0.27, 0.67, 0.06)

// https://github.com/playdeadgames/temporal/blob/master/Assets/Shaders/IncNoise.cginc
// Normalized random: [0,1[
float PDnrand(vec2 n) 
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}
vec2 PDnrand2(vec2 n) 
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* vec2(43758.5453, 28001.8384));
}
vec3 PDnrand3(vec2 n) 
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* vec3(43758.5453, 28001.8384, 50849.4141));
}
vec4 PDnrand4(vec2 n) 
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* vec4(43758.5453, 28001.8384, 50849.4141, 12996.89));
}

// <-

float calcScreenSpaceScale(vec3 worldPosition, mat4 viewProjMatrix, float height)
{
  vec4 p0Proj = viewProjMatrix * vec4(worldPosition, 1.0f);
  p0Proj /= p0Proj.w;

  vec4 p1Proj = vec4(p0Proj.x, p0Proj.y + height * 2.0f, p0Proj.z, 1.0f);
  p1Proj /= p1Proj.w;

  const mat4 invViewProj = inverse(viewProjMatrix);

  vec4 p0 = invViewProj * p0Proj;
  p0 /= p0.w;

  vec4 p1 = invViewProj * p1Proj;
  p1 /= p1.w;

  return length(p1 - p0);
}

// <-

vec2 encodeNormal(vec3 n)
{
  float p = sqrt(n.z * 8.0 + 8.0);
  return vec2(n.xy / p + 0.5);
}

vec3 decodeNormal(vec2 enc)
{
  vec2 fenc = enc * 4.0 - 2.0;
  float f = dot(fenc, fenc);
  float g = sqrt(1.0 - f / 4.0);
  vec3 n;
  n.xy = fenc * g;
  n.z = 1.0 - f / 2.0;
  return n;
}

// <-

vec3 unproject(vec2 uv0, float depth, mat4 invProjMatrix)
{
  vec4 unprojPos = invProjMatrix * vec4(uv0.xy * 2.0 - 1.0, depth, 1.0);
  return unprojPos.xyz / unprojPos.www;
}

// <-

float linearizeDepth(float depth, float n, float f)
{
  return (2.0 * n) / (f + n - depth * (f - n));
}

float depthToVSDepth(float depth, float n, float f)
{
  const float bias = (f - n) / (-f * n);
  const float scale = f / (f * n);

  return 1.0 / (depth * scale + bias);
}
