// Copyright 2017 Benjamin Glatzel
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

void calcPointLight(in Light light, in vec3 posVS, inout vec3 lighting)
{
  const vec3 lightDistVec = light.posAndRadius.xyz - posVS;
  const float dist = length(lightDistVec);
  const float att = calcInverseSqrFalloff(light.posAndRadius.w, dist);

  lighting += att * light.colorAndIntensity.rgb * light.colorAndIntensity.a *
              kelvinToRGB(light.temp.r, kelvinLutTex) / MATH_PI;
}

void calcLocalIrradiance(in IrradProbe probe, in vec3 posVS, in vec3 rayWS,
                         inout vec3 irrad, float fadeFactor)
{
  const float distToProbe = distance(posVS, probe.posAndRadius.xyz);
  if (distToProbe < probe.posAndRadius.w)
  {
    const float fadeRange = probe.posAndRadius.w * probe.data0.x;
    const float fadeStart = probe.posAndRadius.w - fadeRange;
    const float fade =
        pow(1.0 - max(distToProbe - fadeStart, 0.0) / fadeRange, probe.data0.y);

    irrad =
        mix(irrad, sampleSH(probe.shData, rayWS) / MATH_PI, fade * fadeFactor);
  }
}
