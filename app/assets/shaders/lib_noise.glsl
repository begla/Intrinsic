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

const float windStrengthFreq = 0.1;
const float grassFreq = 3.0;
const float foliageFreq = 0.25;
const float grassIntensFact = 0.6;
const float foliageIntensFact = 1.0;

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

// Crytek vegetation animation
// ->
vec4 smoothCurve(vec4 x)
{  
  return x * x * (3.0 - 2.0 * x);  
}

float smoothCurve(float x)
{  
  return x * x * (3.0 - 2.0 * x);  
}

vec4 triangleWave(vec4 x) 
{  
  return abs(fract(x + 0.5) * 2.0 - 1.0);  
}

float triangleWave(float x) 
{  
  return abs(fract(x + 0.5) * 2.0 - 1.0);  
}

vec4 smoothTriangleWave(vec4 x) 
{  
  return smoothCurve(triangleWave(x));  
}

float smoothTriangleWave(float x) 
{  
  return smoothCurve(triangleWave(x));  
} 

#define SIDE_TO_SIDE_FREQ1 1.975
#define SIDE_TO_SIDE_FREQ2 0.793
#define UP_AND_DOWN_FREQ1 0.375
#define UP_AND_DOWN_FREQ2 0.193

void applyDetailBending(
  inout vec3 vPos,
  vec3 vNormal,
  vec3 objectPosition,
  float detailPhase,
  float branchPhase,
  float time,
  float edgeAtten,
  float branchAtten,
  float branchAmp,
  float speed,
  float detailFreq,
  float detailAmp)
{
  const float objPhase = dot(objectPosition.xyz, vec3(1.0));  
  branchPhase += objPhase;
  float vtxPhase = dot(vPos.xyz, vec3(detailPhase + branchPhase));  

  const vec2 wavesIn = time + vec2(vtxPhase, branchPhase);
  vec4 waves = (fract(wavesIn.xxyy *
             vec4(SIDE_TO_SIDE_FREQ1, SIDE_TO_SIDE_FREQ2, 
                  UP_AND_DOWN_FREQ1, UP_AND_DOWN_FREQ2)) *
             2.0 - 1.0) * speed * detailFreq;
  waves = smoothTriangleWave(waves);
  const vec2 wavesSum = waves.xz + waves.yw;  

  vPos.xyz += wavesSum.x * vec3(edgeAtten * detailAmp * vNormal.xyz);
  vPos.y += wavesSum.y * branchAtten * branchAmp;
}

void applyMainBending(inout vec3 pos, vec2 wind, float bendScale)
{
  const float length = length(pos);
  float bf = pos.y * bendScale;
  bf += 1.0;
  bf *= bf;
  bf = bf * bf - bf;
  vec3 newPos = pos;
  newPos.xz += wind.xy * bf;
  pos.xyz = normalize(newPos.xyz) * length;
}
// <-

void applyGrassBending(inout vec3 pos, vec2 wind)
{
  const float falloff = clamp(pos.y * pos.y, 0.0, 1.0);
  const float windLength = length(wind);
  const vec3 windDir = vec3(wind.x, -windLength * windLength, wind.y);
  pos.xyz += falloff * windDir;
}

vec2 calcWindStrength(float time)
{
  return vec2(
      noise(vec3(time * windStrengthFreq, 0.0, 0.0)) * 2.0 - 1.0, 
      noise(vec3(0.0, time * windStrengthFreq, 0.0)) * 2.0 - 1.0
    );
}

void applyTreeWind(
  inout vec3 localPos, 
  vec3 worldPos,
  vec3 worldNormal,
  float intensity,
  float time,
  vec2 windStrength)
{
  const vec2 wind = foliageIntensFact
    * windStrength
    * clamp(vec2(smoothTriangleWave(time * foliageFreq)) * 0.4 + 0.6, 0.0, 1.0);
  const float windS = clamp(length(windStrength), 0.0, 1.0);

  applyDetailBending(localPos, worldNormal, worldPos,
    0.0, 0.0, time, intensity, intensity, windS * 5.0, 1.0, 0.1, 
    windS * 2.0);
  applyMainBending(localPos, wind, 0.025);
}

void applyGrassWind(
  inout vec3 localPos, 
  vec3 worldPos,
  float time,
  vec2 windStrength)
{
  const vec2 detailStrength = clamp(
      vec2(
        noise(vec3(worldPos.x * 2.0 + time * grassFreq, 0.0, 0.0)) * 0.5 + 0.5,
        noise(vec3(worldPos.z * 2.0 + time * grassFreq, 0.0, 0.0)) * 0.5 + 0.5
      )
      , vec2(0.0), vec2(1.0));
  const vec2 mainStrength = clamp(
    vec2(
        (smoothTriangleWave(worldPos.x * 0.05 + time * grassFreq * 0.25) * 0.5 + 0.5) * 0.8 + 0.2,
        (smoothTriangleWave(worldPos.z * 0.05 + time * grassFreq * 0.25) * 0.5 + 0.5) * 0.8 + 0.2
      )
      , vec2(0.0), vec2(1.0));

  const vec2 wind = grassIntensFact
    * windStrength
    * ((mainStrength + detailStrength) * 0.5);

  applyGrassBending(localPos, wind);
} 
