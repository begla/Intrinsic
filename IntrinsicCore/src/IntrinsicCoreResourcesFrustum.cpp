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

// Precompiled header file
#include "stdafx.h"

//#define USE_NAIVE_CULLING

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
struct CullingParallelTaskSet : enki::ITaskSet
{
  virtual ~CullingParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Frustum", "Cull Nodes");

    glm::mat4 mainFrustumViewMatrix =
        FrustumManager::_descViewMatrix(_frustums[0u]);

    for (uint32_t frustIdx = 0u; frustIdx < _frustums.size(); ++frustIdx)
    {
      Resources::FrustumRef frustumRef = _frustums[frustIdx];
      const uint32_t frustumMask = 1u << frustIdx;

      const Math::FrustumPlanes& frustumPlanes =
          Resources::FrustumManager::_frustumPlanesViewSpace(frustumRef);

#if !defined(USE_NAIVE_CULLING)
      const __m128 simdFrustumPlanes[] = {
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kNear].x,
                        -frustumPlanes.n[Math::FrustumPlane::kFar].x,
                        -frustumPlanes.n[Math::FrustumPlane::kLeft].x,
                        -frustumPlanes.n[Math::FrustumPlane::kRight].x),
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kNear].y,
                        -frustumPlanes.n[Math::FrustumPlane::kFar].y,
                        -frustumPlanes.n[Math::FrustumPlane::kLeft].y,
                        -frustumPlanes.n[Math::FrustumPlane::kRight].y),
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kNear].z,
                        -frustumPlanes.n[Math::FrustumPlane::kFar].z,
                        -frustumPlanes.n[Math::FrustumPlane::kLeft].z,
                        -frustumPlanes.n[Math::FrustumPlane::kRight].z),
          Simd::simdSet(-frustumPlanes.d[Math::FrustumPlane::kNear],
                        -frustumPlanes.d[Math::FrustumPlane::kFar],
                        -frustumPlanes.d[Math::FrustumPlane::kLeft],
                        -frustumPlanes.d[Math::FrustumPlane::kRight]),
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kTop].x,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].x,
                        -frustumPlanes.n[Math::FrustumPlane::kTop].x,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].x),
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kTop].y,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].y,
                        -frustumPlanes.n[Math::FrustumPlane::kTop].y,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].y),
          Simd::simdSet(-frustumPlanes.n[Math::FrustumPlane::kTop].z,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].z,
                        -frustumPlanes.n[Math::FrustumPlane::kTop].z,
                        -frustumPlanes.n[Math::FrustumPlane::kBottom].z),
          Simd::simdSet(-frustumPlanes.d[Math::FrustumPlane::kTop],
                        -frustumPlanes.d[Math::FrustumPlane::kBottom],
                        -frustumPlanes.d[Math::FrustumPlane::kTop],
                        -frustumPlanes.d[Math::FrustumPlane::kBottom])};
#endif // USE_NAIVE_CULLING

      for (uint32_t nodeIdx = p_Range.start; nodeIdx < p_Range.end; ++nodeIdx)
      {
        Components::NodeRef nodeRef =
            Components::NodeManager::getActiveResourceAtIndex(nodeIdx);
        const Math::Sphere& cullingSphere =
            Components::NodeManager::_worldBoundingSphere(nodeRef);
        uint32_t& visibilityMask =
            Components::NodeManager::_visibilityMask(nodeRef);

#if !defined(USE_NAIVE_CULLING)
        const __m128 s = Simd::simdSet(cullingSphere.p.x, cullingSphere.p.y,
                                       cullingSphere.p.z, cullingSphere.r);
        const __m128 xxxx = Simd::simdSplatX(s);
        const __m128 yyyy = Simd::simdSplatY(s);
        const __m128 zzzz = Simd::simdSplatZ(s);
        const __m128 rrrr = Simd::simdSplatW(s);

        __m128 v, r;
        v = Simd::simdMadd(xxxx, simdFrustumPlanes[0], simdFrustumPlanes[3]);
        v = Simd::simdMadd(yyyy, simdFrustumPlanes[1], v);
        v = Simd::simdMadd(zzzz, simdFrustumPlanes[2], v);

        r = _mm_cmpgt_ps(v, rrrr);

        v = Simd::simdMadd(xxxx, simdFrustumPlanes[4], simdFrustumPlanes[7]);
        v = Simd::simdMadd(yyyy, simdFrustumPlanes[5], v);
        v = Simd::simdMadd(zzzz, simdFrustumPlanes[6], v);

        r = _mm_or_ps(r, _mm_cmpgt_ps(v, rrrr));
        r = _mm_or_ps(r, _mm_movehl_ps(r, r));
        r = _mm_or_ps(r, Simd::simdSplatY(r));

        uint32_t result;
        _mm_store_ss((float*)&result, r);
#else
        uint32_t result = (uint32_t)-1;
        for (int i = 0; i < Math::FrustumPlane::kCount; ++i)
        {
          if (glm::dot(frustumPlanes.n[i], cullingSphere.p) +
                  frustumPlanes.d[i] <
              -cullingSphere.r)
          {
            result = 0x0;
            break;
          }
        }
#endif // USE_NAIVE_CULLING

        if ((result & 1u) > 0u)
        {
          visibilityMask &= ~frustumMask;
        }
        else
        {
          visibilityMask |= frustumMask;
        }
      }
    }
  }

  FrustumRefArray _frustums;
} _cullingParallelTaskSet;

void FrustumManager::init()
{
  _INTR_LOG_INFO("Inititializing Frustum Manager...");

  Dod::Resources::ResourceManagerBase<
      FrustumData, _INTR_MAX_FRUSTUM_COUNT>::_initResourceManager();
}

// <-

void FrustumManager::cullNodes(const FrustumRefArray& p_ActiveFrustums)
{
  _INTR_PROFILE_CPU("Frustum", "Culling");

  _cullingParallelTaskSet._frustums = p_ActiveFrustums;
  _cullingParallelTaskSet.m_SetSize =
      Components::NodeManager::getActiveResourceCount();

  Application::_scheduler.AddTaskSetToPipe(&_cullingParallelTaskSet);
  Application::_scheduler.WaitforTaskSet(&_cullingParallelTaskSet);
}
}
}
}
