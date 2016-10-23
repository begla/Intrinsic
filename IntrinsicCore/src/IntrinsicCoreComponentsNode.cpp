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
    _normalMatrix(nodeRef) = rot;

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
  }
}
}
}
}
