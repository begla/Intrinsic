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
namespace Renderer
{
namespace RenderProcess
{
struct Default
{
  static void loadRendererConfig();
  static void renderFrame(float p_DeltaT);

  // Static members
  // ->

  static Core::Dod::RefArray _activeFrustums;
  static _INTR_HASH_MAP(Components::CameraRef,
                        _INTR_ARRAY(Dod::Ref)) _shadowFrustums;
  static _INTR_HASH_MAP(CResources::FrustumRef, uint8_t) _cameraToIdMapping;

  static _INTR_INLINE const
      Containers::LockFreeStack<Core::Dod::Ref, _INTR_MAX_DRAW_CALL_COUNT>&
      getVisibleDrawCalls(Components::CameraRef p_CameraRef,
                          uint32_t p_FrustumIdx, uint32_t p_MaterialPassIdx)
  {
    return _visibleDrawCallsPerMaterialPass[_cameraToIdMapping[p_CameraRef] +
                                            p_FrustumIdx][p_MaterialPassIdx];
  }

  static _INTR_INLINE const
      Containers::LockFreeStack<Core::Dod::Ref, _INTR_MAX_MESH_COMPONENT_COUNT>&
      getVisibleMeshComponents(Components::CameraRef p_CameraRef,
                               uint32_t p_FrustumIdx)
  {
    return _visibleMeshComponents[_cameraToIdMapping[p_CameraRef] +
                                  p_FrustumIdx];
  }

  static Containers::LockFreeStack<Core::Dod::Ref, _INTR_MAX_DRAW_CALL_COUNT>
      _visibleDrawCallsPerMaterialPass[_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT]
                                      [_INTR_MAX_MATERIAL_PASS_COUNT];
  static Containers::LockFreeStack<Core::Dod::Ref,
                                   _INTR_MAX_MESH_COMPONENT_COUNT>
      _visibleMeshComponents[_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT];

  // <-
};
}
}
}
