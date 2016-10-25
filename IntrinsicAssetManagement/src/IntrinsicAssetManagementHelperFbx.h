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
namespace AssetManagement
{
namespace HelperFbx
{
template <typename T>
int getVertexIndex(const FbxLayerElementTemplate<T>* p_LayerElement,
                   FbxMesh* p_Mesh, int p_PolygonIndex, int p_PolygonVertex,
                   int p_MeshVertex)
{
  int index = 0;
  switch (p_LayerElement->GetMappingMode())
  {
  case FbxLayerElement::eByControlPoint:
    index = p_Mesh->GetPolygonVertex(p_PolygonIndex, p_PolygonVertex);
    break;
  case FbxLayerElement::eByPolygonVertex:
    index = p_MeshVertex;
    break;
  case FbxLayerElement::eByPolygon:
    index = p_PolygonIndex;
    break;
  }

  if (p_LayerElement->GetReferenceMode() != FbxLayerElement::eDirect)
  {
    if (index < p_LayerElement->GetIndexArray().GetCount())
      index = p_LayerElement->GetIndexArray().GetAt(index);
  }

  return index;
}

// <-

template <typename T>
T getElement(const FbxLayerElementTemplate<T>* p_LayerElement, FbxMesh* p_Mesh,
             int p_PolygonIndex, int p_PolygonVertex, int p_MeshVertex)
{
  if (!p_LayerElement)
  {
    return T();
  }

  return p_LayerElement->GetDirectArray().GetAt(getVertexIndex(
      p_LayerElement, p_Mesh, p_PolygonIndex, p_PolygonVertex, p_MeshVertex));
}
}
}
}
