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

// Precompiled header file
#include "stdafx.h"

namespace Intrinsic
{
namespace Core
{
namespace Components
{
// Static members
NodeRefArray NodeManager::_rootNodes;
NodeRefArray NodeManager::_sortedNodes;

void NodeManager::init()
{
  _INTR_LOG_INFO("Inititializing Node Component Manager...");

  Dod::Components::ComponentManagerBase<
      NodeData, _INTR_MAX_NODE_COMPONENT_COUNT>::_initComponentManager();

  _sortedNodes.reserve(_INTR_MAX_NODE_COMPONENT_COUNT);
  _rootNodes.reserve(_INTR_MAX_NODE_COMPONENT_COUNT);

  Dod::Components::ComponentManagerEntry nodeEntry;
  {
    nodeEntry.createFunction = Components::NodeManager::createNode;
    nodeEntry.destroyFunction = Components::NodeManager::destroyNode;
    nodeEntry.getComponentForEntityFunction =
        Components::NodeManager::getComponentForEntity;
    nodeEntry.onPropertyUpdateFinishedFunction =
        Components::NodeManager::updateTransforms;
    nodeEntry.onInsertionDeletionFinishedAction =
        Components::NodeManager::rebuildTreeAndUpdateTransforms;

    Application::_componentManagerMapping[_N(Node)] = nodeEntry;
    Application::_orderedComponentManagers.push_back(nodeEntry);
  }

  Dod::PropertyCompilerEntry propCompilerNode;
  {
    propCompilerNode.compileFunction =
        Components::NodeManager::compileDescriptor;
    propCompilerNode.initFunction = Components::NodeManager::initFromDescriptor;
    propCompilerNode.ref = Dod::Ref();

    Application::_componentPropertyCompilerMapping[_N(Node)] = propCompilerNode;
  }
}

void NodeManager::updateTransforms(const NodeRefArray& p_Nodes)
{
  for (uint32_t nodeIdx = 0u; nodeIdx < p_Nodes.size(); ++nodeIdx)
  {
    NodeRef nodeRef = p_Nodes[nodeIdx];
    NodeRef parentNodeRef = _parent(nodeRef);

    if (!parentNodeRef.isValid())
    {
      _worldPosition(nodeRef) = _position(nodeRef);
      _worldOrientation(nodeRef) = _orientation(nodeRef);
      _worldSize(nodeRef) = _size(nodeRef);
    }
    else
    {
      const glm::vec3& parentPos = _worldPosition(parentNodeRef);
      const glm::quat& parentOrient = _worldOrientation(parentNodeRef);
      const glm::vec3& parentSize = _worldSize(parentNodeRef);

      const glm::vec3& localPos = _position(nodeRef);
      const glm::quat& localOrient = _orientation(nodeRef);
      const glm::vec3& localSize = _size(nodeRef);

      const glm::vec3 worldPos = parentPos + (parentOrient * localPos);
      const glm::quat worldOrient = parentOrient * localOrient;
      const glm::vec3 worldSize = parentSize * localSize;

      _worldPosition(nodeRef) = worldPos;
      _worldOrientation(nodeRef) = worldOrient;
      _worldSize(nodeRef) = worldSize;
    }

    glm::mat4 rot = glm::mat4_cast(NodeManager::_worldOrientation(nodeRef));
    glm::mat4 trans =
        glm::translate(glm::mat4(1.0f), NodeManager::_worldPosition(nodeRef));
    glm::mat4 scale =
        glm::scale(glm::mat4(1.0f), NodeManager::_worldSize(nodeRef));

    _worldMatrix(nodeRef) = trans * rot * scale;
    _inverseWorldMatrix(nodeRef) = glm::inverse(_worldMatrix(nodeRef));

    // Update AABB
    // TODO: Merge sub meshes
    Components::MeshRef meshCompRef =
        Components::MeshManager::getComponentForEntity(_entity(nodeRef));
    if (meshCompRef.isValid())
    {
      Name& meshName = Components::MeshManager::_descMeshName(meshCompRef);
      Resources::MeshRef meshRef =
          Resources::MeshManager::_getResourceByName(meshName);

      if (meshRef.isValid())
      {
        const uint32_t aabbCount =
            (uint32_t)Resources::MeshManager::_aabbPerSubMesh(meshRef).size();

        if (aabbCount > 0u)
        {
          _localAABB(nodeRef) =
              Resources::MeshManager::_aabbPerSubMesh(meshRef)[0u];
          _worldAABB(nodeRef) = _localAABB(nodeRef);
          Math::transformAABBAffine(_worldAABB(nodeRef), _worldMatrix(nodeRef));

          _worldBoundingSphere(nodeRef) = {
              Math::calcAABBCenter(_worldAABB(nodeRef)),
              glm::length(Math::calcAABBHalfExtent(_worldAABB(nodeRef)))};
        }
      }
    }
    else
    {
      _worldAABB(nodeRef) =
          Math::AABB(_worldPosition(nodeRef) - glm::vec3(0.5f),
                     _worldPosition(nodeRef) + glm::vec3(0.5f));
    }
  }
}
}
}
}
