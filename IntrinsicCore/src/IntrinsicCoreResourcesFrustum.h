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
namespace Resources
{
typedef Dod::Ref FrustumRef;
typedef _INTR_ARRAY(FrustumRef) FrustumRefArray;

namespace ProjectionType
{
enum Enum
{
  kPerspective,
  kOrthographic
};
}

struct FrustumData : Dod::Resources::ResourceDataBase
{
  FrustumData() : Dod::Resources::ResourceDataBase(_INTR_MAX_FRUSTUM_COUNT)
  {
    descProjectionType.resize(_INTR_MAX_FRUSTUM_COUNT);
    descNearFarPlaneDistances.resize(_INTR_MAX_FRUSTUM_COUNT);

    descViewMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);
    invViewMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);

    descProjectionMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);
    invProjectionMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);

    viewProjectionMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);
    invViewProjectionMatrix.resize(_INTR_MAX_FRUSTUM_COUNT);

    frustumWorldPosition.resize(_INTR_MAX_FRUSTUM_COUNT);
    frustumPlanesViewSpace.resize(_INTR_MAX_FRUSTUM_COUNT);
    frustumCornersViewSpace.resize(_INTR_MAX_FRUSTUM_COUNT);
    frustumCornersWorldSpace.resize(_INTR_MAX_FRUSTUM_COUNT);
  }

  // <-

  _INTR_ARRAY(uint8_t) descProjectionType;
  _INTR_ARRAY(glm::vec2) descNearFarPlaneDistances;

  _INTR_ARRAY(glm::mat4) descViewMatrix;
  _INTR_ARRAY(glm::mat4) invViewMatrix;

  _INTR_ARRAY(glm::mat4) descProjectionMatrix;
  _INTR_ARRAY(glm::mat4) invProjectionMatrix;

  _INTR_ARRAY(glm::mat4) viewProjectionMatrix;
  _INTR_ARRAY(glm::mat4) invViewProjectionMatrix;

  _INTR_ARRAY(glm::vec3) frustumWorldPosition;
  _INTR_ARRAY(Math::FrustumPlanes) frustumPlanesViewSpace;
  _INTR_ARRAY(Math::FrustumCorners) frustumCornersViewSpace;
  _INTR_ARRAY(Math::FrustumCorners) frustumCornersWorldSpace;
};

struct FrustumManager
    : Dod::Resources::ResourceManagerBase<FrustumData, _INTR_MAX_FRUSTUM_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static FrustumRef createFrustum(const Name& p_Name)
  {
    FrustumRef ref = Dod::Resources::ResourceManagerBase<
        FrustumData, _INTR_MAX_FRUSTUM_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void destroyFrustum(FrustumRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        FrustumData, _INTR_MAX_FRUSTUM_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void prepareForRendering(FrustumRefArray p_Refs)
  {
    _INTR_PROFILE_CPU("General", "Prepare Frustums For Rendering");

    for (uint32_t i = 0u; i < p_Refs.size(); ++i)
    {
      FrustumRef ref = p_Refs[i];

      _viewProjectionMatrix(ref) =
          _descProjectionMatrix(ref) * _descViewMatrix(ref);
      _frustumWorldPosition(ref) = _descViewMatrix(ref)[3];

      Math::FrustumPlanes& frustumPlanesVS = _frustumPlanesViewSpace(ref);
      Math::extractFrustumPlanes(frustumPlanesVS, _viewProjectionMatrix(ref));

      _invViewMatrix(ref) = glm::inverse(_descViewMatrix(ref));
      _invProjectionMatrix(ref) = glm::inverse(_descProjectionMatrix(ref));
      _invViewProjectionMatrix(ref) = glm::inverse(_viewProjectionMatrix(ref));

      Math::FrustumCorners& frustumCornersVS = _frustumCornersViewSpace(ref);
      Math::FrustumCorners& frustumCornersWS = _frustumCornersWorldSpace(ref);
      Math::extractFrustumsCorners(_invProjectionMatrix(ref), frustumCornersVS);
      Math::extractFrustumsCorners(_invViewProjectionMatrix(ref),
                                   frustumCornersWS);
    }
  }

  // <-

  static void cullNodes(const FrustumRefArray& p_ActiveFrustums);

  // <-

  // Member refs.
  // ->

  _INTR_INLINE static uint8_t& _descProjectionType(FrustumRef p_Ref)
  {
    return _data.descProjectionType[p_Ref._id];
  }
  _INTR_INLINE static glm::vec2& _descNearFarPlaneDistances(FrustumRef p_Ref)
  {
    return _data.descNearFarPlaneDistances[p_Ref._id];
  }
  _INTR_INLINE static glm::mat4& _descViewMatrix(FrustumRef p_Ref)
  {
    return _data.descViewMatrix[p_Ref._id];
  }
  _INTR_INLINE static glm::mat4& _descProjectionMatrix(FrustumRef p_Ref)
  {
    return _data.descProjectionMatrix[p_Ref._id];
  }

  _INTR_INLINE static glm::mat4& _invViewMatrix(FrustumRef p_Ref)
  {
    return _data.invViewMatrix[p_Ref._id];
  }
  _INTR_INLINE static glm::mat4& _invProjectionMatrix(FrustumRef p_Ref)
  {
    return _data.invProjectionMatrix[p_Ref._id];
  }
  _INTR_INLINE static glm::mat4& _viewProjectionMatrix(FrustumRef p_Ref)
  {
    return _data.viewProjectionMatrix[p_Ref._id];
  }
  _INTR_INLINE static glm::mat4& _invViewProjectionMatrix(FrustumRef p_Ref)
  {
    return _data.invViewProjectionMatrix[p_Ref._id];
  }
  _INTR_INLINE static glm::vec3& _frustumWorldPosition(FrustumRef p_Ref)
  {
    return _data.frustumWorldPosition[p_Ref._id];
  }
  _INTR_INLINE static Math::FrustumPlanes&
  _frustumPlanesViewSpace(FrustumRef p_Ref)
  {
    return _data.frustumPlanesViewSpace[p_Ref._id];
  }
  _INTR_INLINE static Math::FrustumCorners&
  _frustumCornersViewSpace(FrustumRef p_Ref)
  {
    return _data.frustumCornersViewSpace[p_Ref._id];
  }
  _INTR_INLINE static Math::FrustumCorners&
  _frustumCornersWorldSpace(FrustumRef p_Ref)
  {
    return _data.frustumCornersWorldSpace[p_Ref._id];
  }

  // <-
};
}
}
}
