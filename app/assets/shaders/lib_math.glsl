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

#define MATH_PI 3.14159

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

vec3 unproject(vec2 uv0, float depth, mat4 invProjMatrix)
{
  vec4 unprojPos = invProjMatrix * vec4(uv0.xy * 2.0 - 1.0, depth, 1.0);
  unprojPos /= unprojPos.w;
  return unprojPos.xyz;
}

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
