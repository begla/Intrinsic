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
  const float windStrength = clamp(noise(vec3(worldPos.xz * 0.001, time * 0.5)) , 0.1, 1.0);
  localPos += windStrength * worldNormal * intensity * vec3(
      noise(worldPos.xyz + 0.17281 + vec3(time * freq)) * 2.0 - 1.0
    , noise(worldPos.xyz + 0.38791 + vec3(time * freq)) * 2.0 - 1.0
    , noise(worldPos.xyz + 0.83911 + vec3(time * freq)) * 2.0 - 1.0
    );
} 
