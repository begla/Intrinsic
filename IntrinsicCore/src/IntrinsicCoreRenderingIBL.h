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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Rendering
{
namespace IBL
{
void initCubemapProcessing();

// <-

// Struct defining a third order SH
struct SH9
{
  SH9& operator+=(const SH9& p_Other)
  {
    for (uint32_t i = 0; i < 9; ++i)
      ((glm::vec3*)this)[i] += ((glm::vec3*)&p_Other)[i];
    return *this;
  }

  SH9& operator*=(float p_Scale)
  {
    for (uint32_t i = 0; i < 9; ++i)
      ((glm::vec3*)this)[i] *= p_Scale;
    return *this;
  }

  SH9 operator*(float p_Scale)
  {
    SH9 result;
    for (uint32_t i = 0; i < 9; ++i)
      ((glm::vec3*)&result)[i] = ((glm::vec3*)this)[i] * p_Scale;
    return result;
  }

  // Band 0
  glm::vec3 L0;

  // Band 1
  glm::vec3 L10;
  glm::vec3 L11;
  glm::vec3 L12;

  // Band 3
  glm::vec3 L20;
  glm::vec3 L21;
  glm::vec3 L22;
  glm::vec3 L23;
  glm::vec3 L24;
};

// <-

_INTR_INLINE Rendering::IBL::SH9 blend(const Rendering::IBL::SH9& p_Left,
                                       const Rendering::IBL::SH9& p_Right,
                                       float p_Blend)
{
  Rendering::IBL::SH9 result;
  {
    glm::vec3* rawResult = (glm::vec3*)&result;

    glm::vec3* rawLeft = (glm::vec3*)&p_Left;
    glm::vec3* rawRight = (glm::vec3*)&p_Right;

    for (uint32_t i = 0u; i < 9u; ++i)
    {
      rawResult[i] = glm::mix(rawLeft[i], rawRight[i], p_Blend);
    }
  }
  return result;
}

// <-

// https://en.wikipedia.org/wiki/Cube_mapping
_INTR_INLINE glm::vec3 mapXYSToDirection(const glm::uvec3& p_PixelPos,
                                         const glm::uvec2& p_Extent)
{
  glm::vec2 uv0 =
      (glm::vec2(p_PixelPos) + 0.5f) / glm::vec2(p_Extent) * 2.0f - 1.0f;
  uv0.y *= -1.0f;

  glm::vec3 dir = glm::vec3(0.0f);

  // +x, -x, +y, -y, +z, -z
  switch (p_PixelPos.z)
  {
  case 0:
    dir = glm::normalize(glm::vec3(1.0f, uv0.y, -uv0.x));
    break;
  case 1:
    dir = glm::normalize(glm::vec3(-1.0f, uv0.y, uv0.x));
    break;
  case 2:
    dir = glm::normalize(glm::vec3(uv0.x, 1.0f, -uv0.y));
    break;
  case 3:
    dir = glm::normalize(glm::vec3(uv0.x, -1.0f, uv0.y));
    break;
  case 4:
    dir = glm::normalize(glm::vec3(uv0.x, uv0.y, 1.0f));
    break;
  case 5:
    dir = glm::normalize(glm::vec3(-uv0.x, uv0.y, -1.0f));
    break;
  }

  // TODO: Not sure why the cubemaps are mirrored...
  dir *= glm::vec3(-1.0f, 1.0f, 1.0f);
  return dir;
}

// <-

// https://en.wikipedia.org/wiki/Cube_mapping
_INTR_INLINE glm::vec3 mapDirectionToUVS(const glm::vec3& p_Direction)
{
  glm::vec3 absDir = glm::abs(p_Direction);
  glm::vec3 isPositive = glm::greaterThan(p_Direction, glm::vec3(0.0f));

  glm::vec3 uvs;
  float maxAxis;

  // POSITIVE X
  if (isPositive.x > 0.0f && absDir.x >= absDir.y && absDir.x >= absDir.z)
  {
    maxAxis = absDir.x;
    uvs.x = -p_Direction.z;
    uvs.y = p_Direction.y;
    uvs.z = 0;
  }
  // NEGATIVE X
  if (!(isPositive.x > 0.0f) && absDir.x >= absDir.y && absDir.x >= absDir.z)
  {
    // u (0 to 1) goes from -z to +z
    // v (0 to 1) goes from -y to +y
    maxAxis = absDir.x;
    uvs.x = p_Direction.z;
    uvs.y = p_Direction.y;
    uvs.z = 1;
  }
  // POSITIVE Y
  if (isPositive.y > 0.0f && absDir.y >= absDir.x && absDir.y >= absDir.z)
  {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from +z to -z
    maxAxis = absDir.y;
    uvs.x = p_Direction.x;
    uvs.y = -p_Direction.z;
    uvs.z = 2;
  }
  // NEGATIVE Y
  if (!(isPositive.y > 0.0f) && absDir.y >= absDir.x && absDir.y >= absDir.z)
  {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -z to +z
    maxAxis = absDir.y;
    uvs.x = p_Direction.x;
    uvs.y = p_Direction.z;
    uvs.z = 3;
  }
  // POSITIVE Z
  if (isPositive.z > 0.0f && absDir.z >= absDir.x && absDir.z >= absDir.y)
  {
    // u (0 to 1) goes from -x to +x
    // v (0 to 1) goes from -y to +y
    maxAxis = absDir.z;
    uvs.x = p_Direction.x;
    uvs.y = p_Direction.y;
    uvs.z = 4;
  }
  // NEGATIVE Z
  if (!(isPositive.z > 0.0f) && absDir.z >= absDir.x && absDir.z >= absDir.y)
  {
    // u (0 to 1) goes from +x to -x
    // v (0 to 1) goes from -y to +y
    maxAxis = absDir.z;
    uvs.x = -p_Direction.x;
    uvs.y = p_Direction.y;
    uvs.z = 5;
  }

  // Convert range from -1 to 1 to 0 to 1
  uvs.x = uvs.x / maxAxis * 0.5f + 0.5f;
  uvs.y = 1.0f - (uvs.y / maxAxis * 0.5f + 0.5f);

  return uvs;
}

// <-

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
_INTR_INLINE glm::vec3 importanceSampleGGX(glm::vec2 p_Xi, float p_Roughness,
                                           glm::vec3 p_N)
{
  float a = p_Roughness * p_Roughness;
  float Phi = 2.0f * glm::pi<float>() * p_Xi.x;
  float cosTheta = sqrt((1.0f - p_Xi.y) / (1.0f + (a * a - 1.0f) * p_Xi.y));
  float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
  glm::vec3 H;
  H.x = sinTheta * cos(Phi);
  H.y = sinTheta * sin(Phi);
  H.z = cosTheta;
  glm::vec3 up = abs(p_N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f)
                                     : glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 tanX = normalize(cross(up, p_N));
  glm::vec3 tanY = cross(p_N, tanX);

  // Tangent to world space
  return tanX * H.x + tanY * H.y + p_N * H.z;
}

// <-

void preFilterGGX(const gli::texture_cube& p_Input, gli::texture_cube& p_Output,
                  const uint32_t* p_SampleCounts, float p_MinRoughness = 0.05f);

_INTR_INLINE void _preFilterGGX(const gli::texture_cube& p_Input,
                                gli::texture_cube& p_Output, uint32_t p_FaceIdx,
                                uint32_t p_MipIdx, glm::uvec2 p_RangeX,
                                glm::uvec2 p_RangeY,
                                const uint32_t* p_SampleCounts,
                                float p_MinRoughness)
{
  gli::fsamplerCube sourceSampler =
      gli::fsamplerCube(p_Input, gli::WRAP_CLAMP_TO_EDGE);
  gli::fsamplerCube targetSampler =
      gli::fsamplerCube(p_Output, gli::WRAP_CLAMP_TO_EDGE);

  glm::uvec2 extent = p_Output.extent(p_MipIdx);
  const float roughness =
      p_MinRoughness +
      float(p_MipIdx) / p_Output.max_level() * (1.0f - p_MinRoughness);
  const uint32_t sampleCount = p_SampleCounts[p_MipIdx];

  for (uint32_t y = p_RangeY.x; y < p_RangeY.y; ++y)
  {
    for (uint32_t x = p_RangeX.x; x < p_RangeX.y; ++x)
    {
      const glm::uvec3 pixelPos = glm::uvec3(x, y, p_FaceIdx);
      const glm::vec2 uv = (glm::vec2(pixelPos) + 0.5f) / glm::vec2(extent);
      const glm::vec3 R = Rendering::IBL::mapXYSToDirection(pixelPos, extent);

      glm::vec3 prefilteredColor = glm::vec3(0.0f);

      float totalWeight = 0.0f;

      for (uint32_t i = 0; i < sampleCount; i++)
      {
        glm::vec2 Xi = Math::hammersley(i, sampleCount);
        glm::vec3 H = importanceSampleGGX(Xi, roughness, R);
        glm::vec3 L = 2.0f * glm::dot(R, H) * H - R;
        float NdL = glm::clamp(glm::dot(R, L), 0.0f, 1.0f);

        if (NdL > 0.0f)
        {
          const glm::vec3 uvs = mapDirectionToUVS(L);
          const glm::vec2 pixelPosSource =
              glm::vec2(uvs) * glm::vec2(p_Output.extent(0u)) - 0.5f;

          prefilteredColor += glm::vec3(sourceSampler.texel_fetch(
                                  pixelPosSource, (uint32_t)uvs.z, 0u)) *
                              NdL;
          totalWeight += NdL;
        }
      }

      targetSampler.texel_write(
          pixelPos, p_FaceIdx, p_MipIdx,
          glm::vec4(prefilteredColor / totalWeight, 1.0f));
    }
  }
}

// <-

_INTR_INLINE SH9 project(const glm::vec3& p_Direction, const glm::vec3& p_Color)
{

  SH9 sh;

  // Band 0
  sh.L0 = p_Color * 0.282095f;

  // Band 1
  sh.L10 = p_Color * 0.488603f * p_Direction.y;
  sh.L11 = p_Color * 0.488603f * p_Direction.z;
  sh.L12 = p_Color * 0.488603f * p_Direction.x;

  // Band 2
  sh.L20 = p_Color * 1.092548f * p_Direction.x * p_Direction.y;
  sh.L21 = p_Color * 1.092548f * p_Direction.y * p_Direction.z;
  sh.L22 = p_Color * 0.315392f * (3.0f * p_Direction.z * p_Direction.z - 1.0f);
  sh.L23 = p_Color * 1.092548f * p_Direction.x * p_Direction.z;
  sh.L24 = p_Color * 0.546274f *
           (p_Direction.x * p_Direction.x - p_Direction.y * p_Direction.y);

  return sh;
}

// <-

_INTR_INLINE SH9 project(const gli::texture_cube& p_CubeMap)
{
  gli::texture_cube decCube =
      gli::convert(p_CubeMap, gli::FORMAT_RGB32_SFLOAT_PACK32);

  SH9 result;
  float weightSum = 0.0f;
  for (uint32_t face = 0; face < 6; ++face)
  {
    for (int32_t y = 0; y < decCube.extent().y; ++y)
    {
      for (int32_t x = 0; x < decCube.extent().x; ++x)
      {
        const glm::vec3 sample =
            decCube.load<glm::vec3>(gli::uvec2(x, y), face, 0);
        glm::vec2 uv0 =
            (glm::vec2(x, y) + 0.5f) / glm::vec2(decCube.extent()) * 2.0f -
            1.0f;

        const float temp = 1.0f + uv0.x * uv0.x + uv0.y * uv0.y;
        const float weight = 4.0f / (std::sqrt(temp) * temp);

        const glm::vec3 dir =
            mapXYSToDirection(glm::uvec3(x, y, face), decCube.extent());
        result += project(dir, sample) * weight;
        weightSum += weight;
      }
    }
  }

  result *= (4.0f * glm::pi<float>()) / weightSum;
  return result;
}

// <-

void captureProbes(const Dod::RefArray& p_NodeRefs, bool p_Clear, float p_Time);
}
}
}
}