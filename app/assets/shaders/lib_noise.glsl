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

float hash(float n) { return fract(sin(n) * 753.5453123); }

float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix(hash(n +  0.0), hash(n +  1.0),f.x),
                   mix(hash(n + 157.0), hash(n + 158.0),f.x),f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0),f.x),
                   mix(hash(n + 270.0), hash(n + 271.0),f.x),f.y),f.z);
}

void applyWind(vec3 worldPos, vec3 worldNormal, float intensity, float time, inout vec3 localPos)
{
  const float freq = 2.0;
  const float windStrength = clamp(noise(vec3(worldPos.xz * 0.001, time * 0.1)) * 2.0 - 1.0, 0.1, 1.0);
  localPos += windStrength * worldNormal * intensity * vec3(
      noise(worldPos.xyz + 0.17281 + vec3(time * freq)) * 2.0 - 1.0
    , noise(worldPos.xyz + 0.38791 + vec3(time * freq)) * 2.0 - 1.0
    , noise(worldPos.xyz + 0.83911 + vec3(time * freq)) * 2.0 - 1.0
    );
} 
