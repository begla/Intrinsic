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
