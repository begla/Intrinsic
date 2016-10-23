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
namespace Core
{
struct World
{
  typedef _INTR_ARRAY(Components::NodeRef) NodeArray;

  static void init();
  static void destroy();

  static void destroyNodeFull(Components::NodeRef p_Ref);
  static Components::NodeRef cloneNodeFull(Components::NodeRef p_Ref);

  static void save(const _INTR_STRING& p_FilePath);
  static void load(const _INTR_STRING& p_FilePath);

  _INTR_INLINE static Components::NodeRef getRootNode() { return _rootNode; }
  _INTR_INLINE static void setRootNode(Components::NodeRef p_Node)
  {
    _rootNode = p_Node;
  }

  _INTR_INLINE static const Components::CameraRef getActiveCamera()
  {
    return _activeCamera;
  }
  _INTR_INLINE static void setActiveCamera(Components::CameraRef p_Camera)
  {
    _activeCamera = p_Camera;
  }

protected:
  static Components::NodeRef _rootNode;
  static Components::CameraRef _activeCamera;
};
}
}
