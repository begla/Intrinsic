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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Irradiance
{
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

_INTR_INLINE Irradiance::SH9 blend(const Irradiance::SH9& p_Left,
                                   const Irradiance::SH9& p_Right,
                                   float p_Blend)
{
  Irradiance::SH9 result;
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

  return dir;
}

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
}
}
}
