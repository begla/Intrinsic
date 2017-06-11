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
struct World
{
  typedef _INTR_ARRAY(Components::NodeRef) NodeArray;

  static void init();
  static void destroy();

  static void destroyNodeFull(Components::NodeRef p_Ref);
  static Components::NodeRef cloneNodeFull(Components::NodeRef p_Ref);
  static void alignNodeWithGround(Components::NodeRef p_NodeRef);

  static void save(const _INTR_STRING& p_FilePath);

  static void load(const _INTR_STRING& p_FilePath);

  static void saveNodeHierarchy(const _INTR_STRING& p_FilePath,
                                Components::NodeRef p_RootNodeRef);
  static Components::NodeRef loadNodeHierarchy(const _INTR_STRING& p_FilePath);
  static void loadNodeResources(Components::NodeRef p_RootNodeRef);

  static void updateDayNightCycle(float p_DeltaT);

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

  static float _currentTime;
  static float _currentDayNightFactor;

  static glm::quat _currentSunLightOrientation;
  static glm::vec4 _currentSunLightColorAndIntensity;

  static _INTR_STRING _filePath;

protected:
  static Components::NodeRef _rootNode;
  static Components::CameraRef _activeCamera;
};
}
}
