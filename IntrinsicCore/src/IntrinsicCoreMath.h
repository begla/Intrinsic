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
namespace Math
{
template <class Type, uint32_t Count> struct Gradient
{
  Type _values[Count];
  float _keyPoints[Count];
};

struct AABB
{
  AABB() {}

  AABB(const glm::vec3& p_Min, const glm::vec3& p_Max) : min(p_Min), max(p_Max)
  {
  }

  glm::vec3 min;
  glm::vec3 max;
};

struct AABB2
{
  AABB2() {}

  AABB2(const glm::vec3& p_Center, const glm::vec3& p_HalfExtent)
      : center(p_Center), halfExtent(p_HalfExtent)
  {
  }

  glm::vec3 center;
  glm::vec3 halfExtent;
};

// <-

struct Ray
{
  glm::vec3 o;
  glm::vec3 d;
};

// <-

namespace FrustumPlane
{
enum Enum
{
  kNear = 0,
  kFar = 1,
  kLeft = 2,
  kRight = 3,
  kTop = 4,
  kBottom = 5,

  kCount
};
}

// <-

namespace FrustumCorner
{
enum Enum
{
  kNearTopRight = 0,
  kNearTopLeft = 1,
  kNearBottomLeft = 2,
  kNearBottomRight = 3,

  kFarTopRight = 4,
  kFarTopLeft = 5,
  kFarBottomLeft = 6,
  kFarBottomRight = 7,

  kCount
};
}

// <-

struct FrustumPlanes
{
  glm::vec3 n[FrustumPlane::kCount];
  float d[FrustumPlane::kCount];
};

// <-

struct FrustumCorners
{
  glm::vec3 c[FrustumCorner::kCount];
};

// <-

struct Sphere
{
  glm::vec3 p;
  float r;
};

// <-

// djb2 hash function
_INTR_INLINE uint32_t hash(const char* p_Data, std::size_t p_Size)
{
  uint32_t hash = 0u;

  for (uint32_t i = 0u; i < p_Size; ++i)
  {
    hash = ((hash << 5) + hash) + p_Data[i];
  }

  return hash;
}

// <-

_INTR_INLINE uint32_t calcRandomNumber()
{
  static uint32_t y = 2463534242u;
  y ^= (y << 13);
  y ^= (y >> 17);
  return (y ^= (y << 15));
}

// <-

_INTR_INLINE float calcRandomFloat()
{
  return (float)calcRandomNumber() / UINT_MAX;
}

// <-

_INTR_INLINE float calcRandomFloatMinMax(float p_Min, float p_Max)
{
  _INTR_ASSERT(p_Min <= p_Max);
  return calcRandomFloat() * (p_Max - p_Min) + p_Min;
}

// <-

template <class T>
_INTR_INLINE void dampSimple(T& p_Value, float p_Damping, float p_DeltaT)
{
  p_Value = p_Value * glm::pow(p_Damping, p_DeltaT);
}

// <-

_INTR_INLINE float calcScreenSpaceScale(const glm::vec3& p_WorldPosition,
                                        const glm::mat4& p_ViewProjMatrix,
                                        float p_Height)
{
  glm::vec4 p0Proj = p_ViewProjMatrix * glm::vec4(p_WorldPosition, 1.0f);
  p0Proj /= p0Proj.w;

  glm::vec4 p1Proj =
      glm::vec4(p0Proj.x, p0Proj.y + p_Height * 2.0f, p0Proj.z, 1.0f);
  p1Proj /= p1Proj.w;

  const glm::mat4 invViewProj = glm::inverse(p_ViewProjMatrix);

  glm::vec4 p0 = invViewProj * p0Proj;
  p0 /= p0.w;

  glm::vec4 p1 = invViewProj * p1Proj;
  p1 /= p1.w;

  return glm::length(p1 - p0);
}

// <-

_INTR_INLINE Ray calcMouseRay(const glm::vec3& p_WorldRayOrigin,
                              const glm::vec2& p_RelMousePos,
                              const glm::mat4x4& p_InverseMatrix)
{
  glm::vec4 screenPos =
      glm::vec4(glm::vec3(p_RelMousePos * 2.0f - 1.0f, 1.0f), 1.0f);
  glm::vec4 worldPos0 = p_InverseMatrix * screenPos;
  worldPos0 /= worldPos0.w;

  glm::vec3 worldPos1 = glm::vec3(worldPos0);
  Ray ray = {p_WorldRayOrigin, glm::normalize(worldPos1 - p_WorldRayOrigin)};
  return ray;
}

// <-

_INTR_INLINE bool calcIntersectRayPlane(const Ray& p_Ray,
                                        const glm::vec3& p_PlaneNormal,
                                        const glm::vec3& p_PointOnPlane,
                                        glm::vec3& p_IntersectionPoint)
{
  float denom = glm::dot(p_PlaneNormal, p_Ray.d);
  if (glm::abs(denom) > _INTR_EPSILON)
  {
    float t = glm::dot(p_PointOnPlane - p_Ray.o, p_PlaneNormal) / denom;
    if (t >= 0)
    {
      p_IntersectionPoint = p_Ray.o + t * p_Ray.d;
      return true;
    }
  }

  return false;
}

// <-

_INTR_INLINE bool calcIntersectPointAABB(const glm::vec3& p_Point,
                                         const AABB& p_AABB)
{
  if (p_Point.x > p_AABB.min.x && p_Point.x < p_AABB.max.x &&
      p_Point.y > p_AABB.min.y && p_Point.y < p_AABB.max.y &&
      p_Point.z > p_AABB.min.z && p_Point.z < p_AABB.max.z)
  {
    return true;
  }

  return false;
}

// <-

_INTR_INLINE bool calcIntersectRayAABB(const Ray& p_Ray, const AABB& p_AABB,
                                       glm::vec3& p_IntersectionPoint0,
                                       glm::vec3& p_IntersectionPoint1)
{
  const glm::vec3 invRayDir = 1.0f / p_Ray.d;

  float t1 = (p_AABB.min[0] - p_Ray.o[0]) * invRayDir[0];
  float t2 = (p_AABB.max[0] - p_Ray.o[0]) * invRayDir[0];

  float tmin = glm::min(t1, t2);
  float tmax = glm::max(t1, t2);

  for (uint32_t i = 1u; i < 3u; ++i)
  {
    t1 = (p_AABB.min[i] - p_Ray.o[i]) * invRayDir[i];
    t2 = (p_AABB.max[i] - p_Ray.o[i]) * invRayDir[i];

    tmin = glm::max(tmin, glm::min(t1, t2));
    tmax = glm::min(tmax, glm::max(t1, t2));
  }

  p_IntersectionPoint0 = p_Ray.o + tmin * p_Ray.d;
  p_IntersectionPoint1 = p_Ray.o + tmax * p_Ray.d;
  return tmax > glm::max(tmin, 0.0f);
}

// <-

_INTR_INLINE glm::vec3 calcAABBHalfExtent(const AABB& p_AABB)
{
  return (p_AABB.max - p_AABB.min) * 0.5f;
}

// <-

_INTR_INLINE glm::vec3 calcAABBCenter(const AABB& p_AABB)
{
  return (p_AABB.min + p_AABB.max) * 0.5f;
}

// <-

_INTR_INLINE bool calcIntersectSphereAABB(const Sphere& p_Sphere,
                                          const AABB2& p_AABB)
{
  const glm::vec3 d0 =
      glm::max(glm::vec3(0.0f),
               glm::abs(p_AABB.center - p_Sphere.p) - p_AABB.halfExtent);
  const float d1 = glm::dot(d0, d0);

  return d1 <= p_Sphere.r * p_Sphere.r;
}

// <-

_INTR_INLINE void scaleAABB(AABB& p_AABB, const glm::vec3& p_Scale)
{
  p_AABB.min *= p_Scale;
  p_AABB.max *= p_Scale;
}

// <-

_INTR_INLINE void extractFrustumsCorners(const glm::mat4& p_InverseMatrix,
                                         FrustumCorners& p_Corners)
{
  p_Corners.c[FrustumCorner::kNearBottomLeft] = glm::vec3(-1.0f, -1.0f, 0.0f);
  p_Corners.c[FrustumCorner::kNearTopLeft] = glm::vec3(-1.0f, 1.0f, 0.0f);
  p_Corners.c[FrustumCorner::kNearTopRight] = glm::vec3(1.0f, 1.0f, 0.0f);
  p_Corners.c[FrustumCorner::kNearBottomRight] = glm::vec3(1.0f, -1.0f, 0.0f);

  p_Corners.c[FrustumCorner::kFarBottomLeft] = glm::vec3(-1.0f, -1.0f, 1.0f);
  p_Corners.c[FrustumCorner::kFarTopLeft] = glm::vec3(-1.0f, 1.0f, 1.0f);
  p_Corners.c[FrustumCorner::kFarTopRight] = glm::vec3(1.0f, 1.0f, 1.0f);
  p_Corners.c[FrustumCorner::kFarBottomRight] = glm::vec3(1.0f, -1.0f, 1.0f);

  for (uint32_t i = 0u; i < FrustumCorner::kCount; ++i)
  {
    glm::vec4 corner = p_InverseMatrix * glm::vec4(p_Corners.c[i], 1.0f);
    p_Corners.c[i] = corner / corner.w;
  }
}

// <-

_INTR_INLINE glm::vec3 calcVecMax(const glm::vec3& p_V0, const glm::vec3& p_V1)
{
  glm::vec3 result;
  result.x = p_V0.x > p_V1.x ? p_V0.x : p_V1.x;
  result.y = p_V0.y > p_V1.y ? p_V0.y : p_V1.y;
  result.z = p_V0.z > p_V1.z ? p_V0.z : p_V1.z;

  return result;
}

// <-

_INTR_INLINE glm::vec3 calcVecMin(const glm::vec3& p_V0, const glm::vec3& p_V1)
{
  glm::vec3 result;
  result.x = p_V0.x < p_V1.x ? p_V0.x : p_V1.x;
  result.y = p_V0.y < p_V1.y ? p_V0.y : p_V1.y;
  result.z = p_V0.z < p_V1.z ? p_V0.z : p_V1.z;

  return result;
}

// <-

_INTR_INLINE void initAABB(AABB& p_AABB)
{
  p_AABB.min = glm::vec3(FLT_MAX);
  p_AABB.max = glm::vec3(-FLT_MAX);
}

// <-

_INTR_INLINE void setAABBInfinite(AABB& p_AABB)
{
  p_AABB.min = glm::vec3(-FLT_MAX);
  p_AABB.max = glm::vec3(FLT_MAX);
}

// <-

_INTR_INLINE void setAABBZero(AABB& p_AABB)
{
  p_AABB.min = glm::vec3(0.0f);
  p_AABB.max = glm::vec3(0.0f);
}

// <-

_INTR_INLINE bool isAABBInfinite(const AABB& p_AABB)
{
  return p_AABB.min == glm::vec3(-FLT_MAX) && p_AABB.max == glm::vec3(FLT_MAX);
}

// <-

_INTR_INLINE bool isAABBZero(const AABB& p_AABB)
{
  return p_AABB.min == glm::vec3(0.0f) && p_AABB.max == glm::vec3(0.0f);
}

// <-

_INTR_INLINE bool isAABBInit(const AABB& p_AABB)
{
  return p_AABB.min == glm::vec3(FLT_MAX) && p_AABB.max == glm::vec3(-FLT_MAX);
}

// <-

_INTR_INLINE bool isAABBValid(const AABB& p_AABB)
{
  return !isAABBInit(p_AABB) && !isAABBZero(p_AABB) && !isAABBInfinite(p_AABB);
}

// <-

_INTR_INLINE void calcAABBCorners(const AABB& p_AABB, glm::vec3* p_Corners)
{
  p_Corners[0] = p_AABB.min;
  p_Corners[1].x = p_AABB.min.x;
  p_Corners[1].y = p_AABB.max.y;
  p_Corners[1].z = p_AABB.min.z;
  p_Corners[2].x = p_AABB.max.x;
  p_Corners[2].y = p_AABB.max.y;
  p_Corners[2].z = p_AABB.min.z;
  p_Corners[3].x = p_AABB.max.x;
  p_Corners[3].y = p_AABB.min.y;
  p_Corners[3].z = p_AABB.min.z;

  p_Corners[4] = p_AABB.max;
  p_Corners[5].x = p_AABB.min.x;
  p_Corners[5].y = p_AABB.max.y;
  p_Corners[5].z = p_AABB.max.z;
  p_Corners[6].x = p_AABB.min.x;
  p_Corners[6].y = p_AABB.min.y;
  p_Corners[6].z = p_AABB.max.z;
  p_Corners[7].x = p_AABB.max.x;
  p_Corners[7].y = p_AABB.min.y;
  p_Corners[7].z = p_AABB.max.z;
}

// <-

_INTR_INLINE void mergePointToAABB(AABB& p_AABB, const glm::vec3& p_Point)
{
  p_AABB.min = calcVecMin(p_Point, p_AABB.min);
  p_AABB.max = calcVecMax(p_Point, p_AABB.max);
}

// <-

_INTR_INLINE void transformAABBAffine(AABB& p_AABB,
                                      const glm::mat4& p_Transform)
{
  glm::vec3 center = calcAABBCenter(p_AABB);
  glm::vec3 halfSize = calcAABBHalfExtent(p_AABB);

  glm::vec3 newCentre = p_Transform * glm::vec4(center, 1.0f);

  glm::vec3 newHalfSize =
      glm::vec3(glm::abs(p_Transform[0][0]) * halfSize.x +
                    glm::abs(p_Transform[0][1]) * halfSize.y +
                    glm::abs(p_Transform[0][2]) * halfSize.z,
                glm::abs(p_Transform[1][0]) * halfSize.x +
                    glm::abs(p_Transform[1][1]) * halfSize.y +
                    glm::abs(p_Transform[1][2]) * halfSize.z,
                glm::abs(p_Transform[2][0]) * halfSize.x +
                    glm::abs(p_Transform[2][1]) * halfSize.y +
                    glm::abs(p_Transform[2][2]) * halfSize.z);

  p_AABB.min = newCentre - newHalfSize;
  p_AABB.max = newCentre + newHalfSize;
}

// <-

_INTR_INLINE float calcHaltonSequence(uint32_t p_Idx, uint32_t p_Base)
{
  float result = 0.0f;
  float frac = 1.0f / p_Base;
  uint32_t i = p_Idx + 1u;

  while (i > 0u)
  {
    result += frac * (i % p_Base);
    i = (uint32_t)glm::floor((float)i / p_Base);
    frac = frac / p_Base;
  }

  return result;
}

// <-

_INTR_INLINE void extractFrustumPlanes(FrustumPlanes& p_FrustumPlanes,
                                       const glm::mat4& p_ViewProjMatrix)
{
  p_FrustumPlanes.n[FrustumPlane::kLeft].x =
      p_ViewProjMatrix[0].w + p_ViewProjMatrix[0].x;
  p_FrustumPlanes.n[FrustumPlane::kLeft].y =
      p_ViewProjMatrix[1].w + p_ViewProjMatrix[1].x;
  p_FrustumPlanes.n[FrustumPlane::kLeft].z =
      p_ViewProjMatrix[2].w + p_ViewProjMatrix[2].x;
  p_FrustumPlanes.d[FrustumPlane::kLeft] =
      p_ViewProjMatrix[3].w + p_ViewProjMatrix[3].x;

  p_FrustumPlanes.n[FrustumPlane::kRight].x =
      p_ViewProjMatrix[0].w - p_ViewProjMatrix[0].x;
  p_FrustumPlanes.n[FrustumPlane::kRight].y =
      p_ViewProjMatrix[1].w - p_ViewProjMatrix[1].x;
  p_FrustumPlanes.n[FrustumPlane::kRight].z =
      p_ViewProjMatrix[2].w - p_ViewProjMatrix[2].x;
  p_FrustumPlanes.d[FrustumPlane::kRight] =
      p_ViewProjMatrix[3].w - p_ViewProjMatrix[3].x;

  p_FrustumPlanes.n[FrustumPlane::kTop].x =
      p_ViewProjMatrix[0].w - p_ViewProjMatrix[0].y;
  p_FrustumPlanes.n[FrustumPlane::kTop].y =
      p_ViewProjMatrix[1].w - p_ViewProjMatrix[1].y;
  p_FrustumPlanes.n[FrustumPlane::kTop].z =
      p_ViewProjMatrix[2].w - p_ViewProjMatrix[2].y;
  p_FrustumPlanes.d[FrustumPlane::kTop] =
      p_ViewProjMatrix[3].w - p_ViewProjMatrix[3].y;

  p_FrustumPlanes.n[FrustumPlane::kBottom].x =
      p_ViewProjMatrix[0].w + p_ViewProjMatrix[0].y;
  p_FrustumPlanes.n[FrustumPlane::kBottom].y =
      p_ViewProjMatrix[1].w + p_ViewProjMatrix[1].y;
  p_FrustumPlanes.n[FrustumPlane::kBottom].z =
      p_ViewProjMatrix[2].w + p_ViewProjMatrix[2].y;
  p_FrustumPlanes.d[FrustumPlane::kBottom] =
      p_ViewProjMatrix[3].w + p_ViewProjMatrix[3].y;

  p_FrustumPlanes.n[FrustumPlane::kFar].x =
      p_ViewProjMatrix[0].w + p_ViewProjMatrix[0].z;
  p_FrustumPlanes.n[FrustumPlane::kFar].y =
      p_ViewProjMatrix[1].w + p_ViewProjMatrix[1].z;
  p_FrustumPlanes.n[FrustumPlane::kFar].z =
      p_ViewProjMatrix[2].w + p_ViewProjMatrix[2].z;
  p_FrustumPlanes.d[FrustumPlane::kFar] =
      p_ViewProjMatrix[3].w + p_ViewProjMatrix[3].z;

  p_FrustumPlanes.n[FrustumPlane::kNear].x =
      p_ViewProjMatrix[0].w - p_ViewProjMatrix[0].z;
  p_FrustumPlanes.n[FrustumPlane::kNear].y =
      p_ViewProjMatrix[1].w - p_ViewProjMatrix[1].z;
  p_FrustumPlanes.n[FrustumPlane::kNear].z =
      p_ViewProjMatrix[2].w - p_ViewProjMatrix[2].z;
  p_FrustumPlanes.d[FrustumPlane::kNear] =
      p_ViewProjMatrix[3].w - p_ViewProjMatrix[3].z;

  for (int planeIdx = 0; planeIdx < FrustumPlane::kCount; ++planeIdx)
  {
    const float len = glm::length(p_FrustumPlanes.n[planeIdx]);
    p_FrustumPlanes.n[planeIdx] /= len;
    p_FrustumPlanes.d[planeIdx] /= len;
  }
}

_INTR_INLINE float noiseHash(float n, float p_Seed)
{
  return glm::fract(sin(n) * p_Seed);
}

// <-

_INTR_INLINE float noise(const glm::vec3& p_X, float p_Seed = 753.5453123f)
{
  glm::vec3 p = glm::floor(p_X);
  glm::vec3 f = glm::fract(p_X);
  f = f * f * (3.0f - 2.0f * f);

  float n = p.x + p.y * 157.0f + 113.0f * p.z;
  return glm::mix(glm::mix(glm::mix(noiseHash(n + 0.0f, p_Seed),
                                    noiseHash(n + 1.0f, p_Seed), f.x),
                           glm::mix(noiseHash(n + 157.0f, p_Seed),
                                    noiseHash(n + 158.0f, p_Seed), f.x),
                           f.y),
                  glm::mix(glm::mix(noiseHash(n + 113.0f, p_Seed),
                                    noiseHash(n + 114.0f, p_Seed), f.x),
                           glm::mix(noiseHash(n + 270.0f, p_Seed),
                                    noiseHash(n + 271.0f, p_Seed), f.x),
                           f.y),
                  f.z);
}

// <-

_INTR_INLINE float noise(const glm::vec3& p_X, uint32_t p_Octaves,
                         float p_Seed = 753.5453123f)
{
  float n = 0.0f;
  for (uint32_t i = 0u; i < p_Octaves; ++i)
  {
    n += noise(p_X * (float)p_Octaves) / (float)p_Octaves;
  }
  return n;
}

// <-

_INTR_INLINE glm::vec3 calcBaryCoords(const glm::vec3& p_V0,
                                      const glm::vec3& p_V1,
                                      const glm::vec3& p_V2)
{
  return glm::vec3(glm::length(p_V0 - p_V1), glm::length(p_V1 - p_V2),
                   glm::length(p_V0 - p_V1));
}

// <-

_INTR_INLINE glm::vec3 calcRandomBaryCoords()
{
  const glm::vec3 b = glm::vec3(Math::calcRandomFloatMinMax(0.0, 1.0),
                                Math::calcRandomFloatMinMax(0.0, 1.0),
                                Math::calcRandomFloatMinMax(0.0, 1.0));
  return b / (b.x + b.y + b.z);
}

// <-

_INTR_INLINE glm::vec3 baryInterpolate(glm::vec3 p_B, const glm::vec3& p_V0,
                                       const glm::vec3& p_V1,
                                       const glm::vec3& p_V2)
{
  return glm::vec3(p_B[0] * p_V0.x + p_B[1] * p_V1.x + p_B[2] * p_V2.x,
                   p_B[0] * p_V0.y + p_B[1] * p_V1.y + p_B[2] * p_V2.y,
                   p_B[0] * p_V0.z + p_B[1] * p_V1.z + p_B[2] * p_V2.z);
}

// <-

template <typename T0, typename T1>
_INTR_INLINE T0 roundToNextMultiple(const T0& p_V0, const T1& p_V1)
{
  return ((p_V0 - 1) / p_V1 + 1) * p_V1;
}

// <-

template <typename T>
_INTR_INLINE T divideByMultiple(const T& p_Val, uint32_t p_Alignment)
{
  return ((p_Val + p_Alignment - 1) / p_Alignment);
}

// <-

_INTR_INLINE float projectSphere(const Sphere& p_S,
                                 const glm::mat4& p_ViewMatrix, float p_Fov)
{
  glm::vec3 o = glm::vec3(p_ViewMatrix * glm::vec4(p_S.p, 1.0));

  const float r2 = p_S.r * p_S.r;
  const float z2 = o.z * o.z;
  const float l2 = glm::dot(o, o);

  return -glm::pi<float>() * p_Fov * p_Fov * r2 *
         glm::sqrt(glm::abs((l2 - r2) / (r2 - z2))) / (r2 - z2);
}

// <-

_INTR_INLINE uint32_t convertColorToBGRA(const glm::vec4& p_Color)
{
  uint32_t color = (uint8_t)(p_Color.b * 255.0f);
  color |= ((uint8_t)(p_Color.g * 255.0f)) << 8u;
  color |= ((uint8_t)(p_Color.r * 255.0f)) << 16u;
  color |= ((uint8_t)(p_Color.a * 255.0f)) << 24u;
  return color;
}

// <-

_INTR_INLINE float bytesToMegaBytes(uint32_t p_Bytes)
{
  return p_Bytes * 0.00000095367431640625f;
}

// <-

_INTR_INLINE uint32_t megaBytesToBytes(float p_MegaBytes)
{
  return (uint32_t)(p_MegaBytes * 1024.0f * 1024.0f);
}

// <-

_INTR_INLINE glm::vec3 wrapEuler(const glm::vec3& p_Euler)
{
  return glm::mod(p_Euler * 0.5f + glm::half_pi<float>(),
                  glm::pi<float>() + _INTR_EPSILON) *
             2.0f -
         glm::pi<float>();
}

// <-

_INTR_INLINE glm::vec3 bezierQuadratic(const _INTR_ARRAY(glm::vec3) p_Points,
                                       float p_Perc)
{
  _INTR_ASSERT(p_Points.size() >= 3u);

  _INTR_ARRAY(glm::vec3) interpPoints;
  interpPoints.resize(p_Points.size());
  memcpy(interpPoints.data(), p_Points.data(),
         sizeof(glm::vec3) * p_Points.size());
  uint32_t numPoints = (uint32_t)p_Points.size() - 1u;

  while (numPoints > 0u)
  {
    for (uint32_t i = 0u; i < numPoints; ++i)
    {
      interpPoints[i] = glm::mix(interpPoints[i], interpPoints[i + 1], p_Perc);
    }
    --numPoints;
  }

  return interpPoints[0];
}

// <-

template <class Type, uint32_t Count>
_INTR_INLINE Type interpolateGradient(const Gradient<Type, Count>& p_Gradient,
                                      float p_KeyValue)
{
  _INTR_ASSERT(Count >= 2u);
  _INTR_ASSERT(p_KeyValue >= 0.0f && p_KeyValue <= 1.0f);

  for (uint32_t i = 0u; i < Count - 1u; ++i)
  {
    const float currentPerc = p_Gradient._keyPoints[i];
    const float nextPerc = p_Gradient._keyPoints[i + 1];
    _INTR_ASSERT(currentPerc <= nextPerc);

    if (p_KeyValue >= currentPerc && p_KeyValue < nextPerc)
    {
      Type currentValue = p_Gradient._values[i];
      Type nextValue = p_Gradient._values[i + 1];

      const float interp =
          (p_KeyValue - currentPerc) / (nextPerc - currentPerc);
      return glm::mix(currentValue, nextValue, interp);
    }
  }

  _INTR_ASSERT(false && "Invalid values provided");
  return glm::vec4(0.0f);
}

_INTR_INLINE float radicalInverse(uint32_t p_Bits)
{
  p_Bits = (p_Bits << 16u) | (p_Bits >> 16u);
  p_Bits = ((p_Bits & 0x55555555u) << 1u) | ((p_Bits & 0xAAAAAAAAu) >> 1u);
  p_Bits = ((p_Bits & 0x33333333u) << 2u) | ((p_Bits & 0xCCCCCCCCu) >> 2u);
  p_Bits = ((p_Bits & 0x0F0F0F0Fu) << 4u) | ((p_Bits & 0xF0F0F0F0u) >> 4u);
  p_Bits = ((p_Bits & 0x00FF00FFu) << 8u) | ((p_Bits & 0xFF00FF00u) >> 8u);
  return float(p_Bits) * 2.3283064365386963e-10f; // / 0x100000000
}

_INTR_INLINE glm::vec2 hammersley(uint32_t p_I, uint32_t p_N)
{
  return glm::vec2((float)p_I / p_N, radicalInverse(p_I));
}
}
}
}
