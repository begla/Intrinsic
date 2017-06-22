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

void calcDecal(in Decal decal, in vec3 posVS, in sampler2D[4095] globalTextures,
               inout vec4 albedo, inout vec3 normal, inout vec4 pbr,
               float alphaFactor)
{
  vec4 posDecalSS = decal.viewProjMatrix * vec4(posVS, 1.0);
  posDecalSS /= posDecalSS.w;

  if (all(lessThanEqual(posDecalSS.xyz, vec3(1.0, 1.0, 1.0))) &&
      all(greaterThanEqual(posDecalSS.xyz, vec3(-1.0, -1.0, 0.0))))
  {
    const mat3 TBN =
        mat3(-decal.tangentVS.xyz, decal.binormalVS.xyz, decal.normalVS.xyz);

    vec2 decalUV = posDecalSS.xy * vec2(0.5, 0.5) + 0.5;
    decalUV = decalUV * decal.uvTransform.xy + decal.uvTransform.zw;

    vec4 albedoSample = texture(globalTextures[decal.textureIds.x], decalUV);
    albedoSample.a *= alphaFactor;

    if (albedoSample.a >= alphaThreshold)
    {
      albedo = albedoSample;
      normal = normalize(
          TBN *
          (texture(globalTextures[decal.textureIds.y], decalUV).xyz * 2.0 -
           1.0));
      pbr = texture(globalTextures[decal.textureIds.z], decalUV);
    }
  }
}
