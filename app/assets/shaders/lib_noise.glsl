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

const float windStrengthFreq = 0.25;
const float grassFreq = 5.0;
const float foliageFreq = 2.0;
const float grassIntensFact = 0.1;
const float foliageIntensFact = 0.5;
const float minWindStrength = 0.2;

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

vec4 triangleWave(vec4 x) 
{  
  return abs(fract(x + 0.5) * 2.0 - 1.0);  
}

vec4 smoothTriangleWave(vec4 x) 
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
  float fDetailPhase,
  float fBranchPhase,
  float fTime,
  float fEdgeAtten,
  float fBranchAtten,
  float fBranchAmp,
  float fSpeed,
  float fDetailFreq,
  float fDetailAmp)
{
  float fObjPhase = dot(objectPosition.xyz, vec3(1.0));  
  fBranchPhase += fObjPhase;
  float fVtxPhase = dot(vPos.xyz, vec3(fDetailPhase + fBranchPhase));  

  vec2 vWavesIn = fTime + vec2(fVtxPhase, fBranchPhase);
  vec4 vWaves = (fract(vWavesIn.xxyy *
             vec4(SIDE_TO_SIDE_FREQ1, SIDE_TO_SIDE_FREQ2, 
                  UP_AND_DOWN_FREQ1, UP_AND_DOWN_FREQ2)) *
             2.0 - 1.0) * fSpeed * fDetailFreq;
  vWaves = smoothTriangleWave(vWaves);
  vec2 vWavesSum = vWaves.xz + vWaves.yw;  

  vPos.xyz += vWavesSum.x * vec3(fEdgeAtten * fDetailAmp * vNormal.xyz);
  vPos.y += vWavesSum.y * fBranchAtten * fBranchAmp;
}

void applyMainBending(inout vec3 pos, vec2 wind, float bendScale)
{
  float length = length(pos);
  float bf = pos.y * bendScale;
  bf += 1.0;
  bf *= bf;
  bf = bf * bf - bf;
  vec3 newPos = pos;
  newPos.xz += wind.xy * bf;
  pos.xyz = normalize(newPos.xyz) * length;
}
// <-

vec2 calcWindStrength(float time)
{
  return max(
    vec2(noise(vec3(time * windStrengthFreq, 0.0, 0.0)),
      noise(vec3(time * windStrengthFreq + 0.5891, 0.0, 0.0)))
      * 2.0 - 1.0,
    vec2(minWindStrength));
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
    * vec2(sin(worldPos.x * 0.05 + time * foliageFreq));
  const float windS = length(windStrength);

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
  const vec2 wind = grassIntensFact
    * windStrength
    * clamp(sin(worldPos.x * 0.1 + time * grassFreq) * 0.5 + 0.5, 0.0, 1.0);

  applyMainBending(localPos, wind, 0.8);
} 
