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

#define UV0_TRANSFORM_ANIMATED(_uv0) vec2(_uv0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
    (1.0 - _uv0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y) \
    + uboPerMaterial.uvAnimation.xy * vec2(uboPerInstance.data0.w)
#define UV0_TRANSFORM_ANIMATED_FACTOR(_uv0, _fact) vec2(_uv0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
    (1.0 - _uv0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y) \
    + _fact * uboPerMaterial.uvAnimation.xy * vec2(uboPerInstance.data0.w)
#define UV0_TRANSFORM(_uv0) vec2(_uv0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
  (1.0 - _uv0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y)
#define UV0(_uv0) vec2(_uv0.x, (1.0 - _uv0.y))

#define PER_INSTANCE_UBO layout (binding = 1) uniform PerInstance \
{ \
  vec4 colorTint; \
  vec4 camParams; \
  vec4 data0; \
} uboPerInstance

#define PER_MATERIAL_UBO layout (binding = 2) uniform PerMaterial \
{ \
  vec4 uvOffsetScale; \
  vec4 uvAnimation; \
  vec4 pbrBias; \
  vec4 waterParams; \
  \
  uvec4 data0; \
  vec4 data1; \
} uboPerMaterial

#define BINDINGS_GBUFFER layout (binding = 3) uniform sampler2D albedoTex; \
layout (binding = 4) uniform sampler2D normalTex; \
layout (binding = 5) uniform sampler2D pbrTex

#define BINDINGS_TERRAIN layout (binding = 3) uniform sampler2D albedoTex0; \
layout (binding = 4) uniform sampler2D normalTex0; \
layout (binding = 5) uniform sampler2D pbrTex0; \
layout (binding = 6) uniform sampler2D albedoTex1; \
layout (binding = 7) uniform sampler2D normalTex1; \
layout (binding = 8) uniform sampler2D pbrTex1; \
layout (binding = 9) uniform sampler2D albedoTex2; \
layout (binding = 10) uniform sampler2D normalTex2; \
layout (binding = 11) uniform sampler2D pbrTex2; \
layout (binding = 12) uniform sampler2D blendMaskTex; \
layout (binding = 13) uniform sampler2D noiseTex

#define OUTPUT \
layout (location = 0) out vec4 outAlbedo; \
layout (location = 1) out vec4 outNormal; \
layout (location = 2) out vec4 outParameter0;

struct GBuffer
{
  vec4 albedo;
  vec3 normal;
  float roughness;
  float specular;
  float metalMask;
  uint materialBufferIdx;
  float emissive;
  float occlusion;
};

void writeGBuffer(in GBuffer gbuffer, inout vec4 outAlbedo, inout vec4 outNormal, inout vec4 outParameter0)
{
  outAlbedo = gbuffer.albedo; 
  outNormal.rg = encodeNormal(gbuffer.normal);
  outNormal.b = clamp(gbuffer.specular, 0.0, 1.0);
  outNormal.a = clamp(gbuffer.roughness, 0.05, 1.0);
  outParameter0.rgba = vec4(clamp(gbuffer.metalMask, 0.0, 1.0), gbuffer.materialBufferIdx, gbuffer.occlusion, gbuffer.emissive);
}

// Specular AA
// http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr.pdf
float adjustRoughness(float inputRoughness , float avgNormalLength)
{
  if (avgNormalLength < 1.0)
  {
    float avgNormLen2 = avgNormalLength * avgNormalLength;
    float kappa = (3.0 * avgNormalLength - avgNormalLength * avgNormLen2) / (1.0 - avgNormLen2);
    float variance = 1.0 / (2.0 * kappa);

    return sqrt(inputRoughness * inputRoughness + variance);
  }

  return inputRoughness;
}