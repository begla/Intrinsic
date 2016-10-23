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

/** \file
* Contains the Node Component Manager.
*/

namespace Intrinsic
{
namespace Core
{
namespace Components
{
typedef Dod::Ref NodeRef;
typedef _INTR_ARRAY(NodeRef) NodeRefArray;

///
/// Stores the Node Component Data in a data oriented fashion.
///
struct NodeData : Dod::Components::ComponentDataBase
{
  NodeData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_NODE_COMPONENT_COUNT)
  {
    position.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    orientation.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    size.resize(_INTR_MAX_NODE_COMPONENT_COUNT);

    worldPosition.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    worldOrientation.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    worldSize.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    worldMatrix.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    normalMatrix.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    inverseWorldMatrix.resize(_INTR_MAX_NODE_COMPONENT_COUNT);

    localAABB.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    worldAABB.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    worldBoundingSphere.resize(_INTR_MAX_NODE_COMPONENT_COUNT);

    visibilityMask.resize(_INTR_MAX_NODE_COMPONENT_COUNT);

    parent.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    firstChild.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    prevSibling.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
    nextSibling.resize(_INTR_MAX_NODE_COMPONENT_COUNT);
  }

  _INTR_ARRAY(glm::vec3) position;
  _INTR_ARRAY(glm::quat) orientation;
  _INTR_ARRAY(glm::vec3) size;

  _INTR_ARRAY(glm::vec3) worldPosition;
  _INTR_ARRAY(glm::quat) worldOrientation;
  _INTR_ARRAY(glm::vec3) worldSize;
  _INTR_ARRAY(glm::mat4x4) worldMatrix;
  _INTR_ARRAY(glm::mat4x4) normalMatrix;
  _INTR_ARRAY(glm::mat4x4) inverseWorldMatrix;

  _INTR_ARRAY(Math::AABB) localAABB;
  _INTR_ARRAY(Math::AABB) worldAABB;
  _INTR_ARRAY(Math::Sphere) worldBoundingSphere;

  _INTR_ARRAY(uint32_t) visibilityMask;

  _INTR_ARRAY(NodeRef) parent;
  _INTR_ARRAY(NodeRef) firstChild;
  _INTR_ARRAY(NodeRef) prevSibling;
  _INTR_ARRAY(NodeRef) nextSibling;
};

///
/// The manager for all Node Components.
///
struct NodeManager
    : Dod::Components::ComponentManagerBase<NodeData,
                                            _INTR_MAX_NODE_COMPONENT_COUNT>
{
  /// Init. the Node Component Manager.
  static void init();

  // <-

  /// Init. the provided Node with default values.
  _INTR_INLINE static void initNode(NodeRef p_Ref)
  {
    _parent(p_Ref) = NodeRef();
    _firstChild(p_Ref) = NodeRef();
    _prevSibling(p_Ref) = NodeRef();
    _nextSibling(p_Ref) = NodeRef();

    _position(p_Ref) = _worldPosition(p_Ref) = glm::vec3();
    _orientation(p_Ref) = _worldOrientation(p_Ref) =
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    _size(p_Ref) = _worldSize(p_Ref) = glm::vec3(1.0f, 1.0f, 1.0f);
    Math::setAABBZero(_worldAABB(p_Ref));
    Math::setAABBZero(_localAABB(p_Ref));
  }

  // <-

  /// Requests a new reference for a Node Component.
  _INTR_INLINE static NodeRef createNode(Entity::EntityRef p_ParentEntity)
  {
    _INTR_ASSERT(p_ParentEntity.isValid());

    NodeRef ref = Dod::Components::ComponentManagerBase<
        NodeData,
        _INTR_MAX_NODE_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    initNode(ref);

    internalAddToRootNodeArray(ref);

    Resources::EventManager::queueEventIfNotExisting(_N(NodeCreated));

    return ref;
  }

  // <-

  /// Collects all Nodes recursively starting at the given Node and puts
  /// them in the provided array.
  _INTR_INLINE static void collectNodes(NodeRef p_Node, NodeRefArray& p_Nodes)
  {
    static _INTR_ARRAY(NodeRef) nodeStack;
    nodeStack.clear();
    nodeStack.push_back(p_Node);

    while (!nodeStack.empty())
    {
      NodeRef currentNode = nodeStack.back();
      nodeStack.pop_back();

      NodeRef firstChild = _firstChild(currentNode);
      NodeRef nextSibling = _nextSibling(currentNode);

      if (firstChild.isValid())
      {
        nodeStack.push_back(firstChild);
      }
      if (p_Node != currentNode && nextSibling.isValid())
      {
        nodeStack.push_back(nextSibling);
      }

      p_Nodes.push_back(currentNode);
    }
  }

  // <-

  /// Collects all Entities recursively starting at the given Node and puts
  /// them in the provided Array.
  _INTR_INLINE static void collectEntities(NodeRef p_Node,
                                           Entity::EntityRefArray& p_Entities)
  {
    NodeRefArray nodes;
    collectNodes(p_Node, nodes);

    for (uint32_t i = 0u; i < nodes.size(); ++i)
    {
      NodeRef currentNode = nodes[i];
      p_Entities.push_back(_entity(currentNode));
    }
  }

  // <-

  /// Destroys the given Node Component by putting the reference back into the
  /// pool.
  _INTR_INLINE static void destroyNode(NodeRef p_Node)
  {
    NodeRefArray nodes;
    collectNodes(p_Node, nodes);

    for (uint32_t i = 0u; i < nodes.size(); ++i)
    {
      NodeRef currentNode = nodes[i];

      NodeRef parent = _parent(currentNode);

      // Make sure to patch all nodes in the hierarchy if this is a child node
      if (parent.isValid())
      {
        detachChild(currentNode);
      }

      internalRemoveFromRootNodeArray(currentNode);

      // Destroy the actual resource
      Dod::Components::ComponentManagerBase<
          NodeData,
          _INTR_MAX_NODE_COMPONENT_COUNT>::_destroyComponent(currentNode);
    }

    Resources::EventManager::queueEventIfNotExisting(_N(NodeDestroyed));
  }

  // <-

  /// Rebuilds the internal sorted node array.
  _INTR_INLINE static void rebuildTree()
  {
    _sortedNodes.clear();

    for (uint32_t i = 0; i < _rootNodes.size(); ++i)
    {
      NodeRef currentRootNode = _rootNodes[i];

      NodeRefArray nodes;
      collectNodes(currentRootNode, _sortedNodes);
    }
  }

  // <-

  /// Updates the transformations for the provided Nodes.
  static void updateTransforms(const NodeRefArray& p_Nodes);

  // <-

  /// Updates all transformation for all trees in the manager.
  _INTR_INLINE static void updateTransforms()
  {
    updateTransforms(_sortedNodes);
  }

  // <-

  /// Updates the transformations recursively starting at the given Node.
  _INTR_INLINE static void updateTransforms(NodeRef p_RootNode)
  {
    NodeRefArray nodes;
    collectNodes(p_RootNode, nodes);
    updateTransforms(nodes);
  }

  // <-

  /// Rebuilds the internal sorted node array and updates all node transforms.
  _INTR_INLINE static void rebuildTreeAndUpdateTransforms()
  {
    rebuildTree();
    updateTransforms();
  }

  // <-

  /// Attaches the given child Node to the provided (soon to be) parent Node.
  /// The current local
  /// transformation of the node is kept and thus potentially changes the world
  /// transformation.
  _INTR_INLINE static void attachChildIgnoreParent(NodeRef p_Parent,
                                                   NodeRef p_Child)
  {
    _INTR_ASSERT(!_parent(p_Child).isValid() &&
                 "This node already has a parent");
    _INTR_ASSERT(!_prevSibling(p_Child).isValid() &&
                 !_nextSibling(p_Child).isValid() &&
                 "This node is already part of a hierarchy");

    const NodeRef firstChild = _firstChild(p_Parent);

    {
      _parent(p_Child) = p_Parent;

      // First child? Just set it and we're done
      if (!firstChild.isValid())
      {
        _firstChild(p_Parent) = p_Child;
      }
      else
      {
        // The first child of a node has to be the first in the chain
        _INTR_ASSERT(!_prevSibling(firstChild).isValid());

        _prevSibling(firstChild) = p_Child;
        _nextSibling(p_Child) = firstChild;
        _firstChild(p_Parent) = p_Child;
      }
    }

    internalRemoveFromRootNodeArray(p_Child);
  }

  // <-

  /// Attaches the child Node to the provided (soon to be) parent Node. Adjusts
  /// the local
  /// transformation to keep the world transformation.
  _INTR_INLINE static void attachChild(NodeRef p_Parent, NodeRef p_Child)
  {
    attachChildIgnoreParent(p_Parent, p_Child);

    // Undo the parent's transform
    {
      glm::quat inverseParentWorldOrient =
          glm::inverse(_worldOrientation(p_Parent));

      _position(p_Child) = inverseParentWorldOrient *
                           (_position(p_Child) - _worldPosition(p_Parent));
      _orientation(p_Child) = _orientation(p_Child) * inverseParentWorldOrient;
      _size(p_Child) = _size(p_Child) / _worldSize(p_Parent);
    }
  }

  // <-

  /// Detaches the Node from the current parent node.
  _INTR_INLINE static void detachChild(NodeRef p_Child)
  {
    NodeRef parent = _parent(p_Child);
    _INTR_ASSERT(parent.isValid() && "This node has no parent");

    NodeRef prevSibling = _prevSibling(p_Child);
    NodeRef nextSibling = _nextSibling(p_Child);

    // No siblings? Just adjust the parent
    if (!prevSibling.isValid() && !nextSibling.isValid())
    {
      _firstChild(parent) = NodeRef();
    }
    // We're right in the middle? Connect the other two nodes
    else if (prevSibling.isValid() && nextSibling.isValid())
    {
      _nextSibling(prevSibling) = nextSibling;
      _prevSibling(nextSibling) = prevSibling;
    }
    // We're at the start - adjust the parent and remove the child from the
    // following node
    else if (!prevSibling.isValid() && nextSibling.isValid())
    {
      _firstChild(parent) = nextSibling;
      _prevSibling(nextSibling) = NodeRef();
    }
    // We're at the end - just remove it from the previous node
    else if (prevSibling.isValid() && !nextSibling.isValid())
    {
      _nextSibling(prevSibling) = NodeRef();
    }

    // Keep the same transform after detaching
    _position(p_Child) = _worldPosition(p_Child);
    _size(p_Child) = _worldSize(p_Child);
    _orientation(p_Child) = _worldOrientation(p_Child);

    _prevSibling(p_Child) = NodeRef();
    _nextSibling(p_Child) = NodeRef();
    _parent(p_Child) = NodeRef();

    // This is once again a root node
    internalAddToRootNodeArray(p_Child);
  }

  // <-

  /// Compiles all exposed properties to a JSON descriptor.
  _INTR_INLINE static void compileDescriptor(NodeRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "localPos", _INTR_CREATE_PROP(p_Document, _N(NodeLocalTransform),
                                      _N(vec3), _position(p_Ref), false, false),
        p_Document.GetAllocator());
    p_Properties.AddMember("localOrient",
                           _INTR_CREATE_PROP(p_Document, _N(NodeLocalTransform),
                                             _N(rotation), _orientation(p_Ref),
                                             false, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "localSize", _INTR_CREATE_PROP(p_Document, _N(NodeLocalTransform),
                                       _N(vec3), _size(p_Ref), false, false),
        p_Document.GetAllocator());

    // Read only
    p_Properties.AddMember("worldPos",
                           _INTR_CREATE_PROP(p_Document, _N(NodeWorldTransform),
                                             _N(vec3), _worldPosition(p_Ref),
                                             true, false),
                           p_Document.GetAllocator());
    p_Properties.AddMember(
        "worldOrient",
        _INTR_CREATE_PROP(p_Document, _N(NodeWorldTransform), _N(rotation),
                          _worldOrientation(p_Ref), true, false),
        p_Document.GetAllocator());
    p_Properties.AddMember("worldSize",
                           _INTR_CREATE_PROP(p_Document, _N(NodeWorldTransform),
                                             _N(vec3), _worldSize(p_Ref), true,
                                             false),
                           p_Document.GetAllocator());
  }

  /// Init. the Node from the provided JSON descriptor.
  _INTR_INLINE static void initFromDescriptor(NodeRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("localPos"))
    {
      _position(p_Ref) = JsonHelper::readPropertyVec3(p_Properties["localPos"]);
    }
    if (p_Properties.HasMember("localOrient"))
    {
      _orientation(p_Ref) =
          JsonHelper::readPropertyQuat(p_Properties["localOrient"]);
    }
    if (p_Properties.HasMember("localSize"))
    {
      _size(p_Ref) = JsonHelper::readPropertyVec3(p_Properties["localSize"]);
    }
  }

  // Getter/Setter
  // Intrinsic

  /// Gets the total amount of available sorted nodes.
  _INTR_INLINE static uint32_t getSortedNodeCount()
  {
    return (uint32_t)_sortedNodes.size();
  }
  /// Gets the sorted node at the given index.
  _INTR_INLINE static NodeRef getSortedNodeAtIndex(uint32_t p_Idx)
  {
    return _sortedNodes[p_Idx];
  }

  /// Returns the (local) position.
  _INTR_INLINE static const glm::vec3& getPosition(NodeRef p_Ref)
  {
    return _data.position[p_Ref._id];
  }
  /// Sets the (local) position.
  _INTR_INLINE static void setPosition(NodeRef p_Ref,
                                       const glm::vec3& p_Position)
  {
    _data.position[p_Ref._id] = p_Position;
  }

  /// Returns the (local) orientation.
  _INTR_INLINE static const glm::quat& getOrientation(NodeRef p_Ref)
  {
    return _data.orientation[p_Ref._id];
  }
  /// Sets the (local) orientation.
  _INTR_INLINE static void setOrientation(NodeRef p_Ref,
                                          const glm::quat& p_Orientation)
  {
    _data.orientation[p_Ref._id] = p_Orientation;
  }

  /// Returns the (local) size.
  _INTR_INLINE static const glm::vec3& getSize(NodeRef p_Ref)
  {
    return _data.size[p_Ref._id];
  }
  /// Sets the (local) size.
  _INTR_INLINE static void setSize(NodeRef p_Ref, const glm::vec3& p_Size)
  {
    _data.size[p_Ref._id] = p_Size;
  }

  // <-

  // Member refs
  // Intrinsic

  /// The parent Node of this Node. If any.
  _INTR_INLINE static NodeRef& _parent(NodeRef p_Ref)
  {
    return _data.parent[p_Ref._id];
  }
  /// The first child Node of this Node. If any.
  _INTR_INLINE static NodeRef& _firstChild(NodeRef p_Ref)
  {
    return _data.firstChild[p_Ref._id];
  }
  /// The previous sibling Node of this Node. If any.
  _INTR_INLINE static NodeRef& _prevSibling(NodeRef p_Ref)
  {
    return _data.prevSibling[p_Ref._id];
  }
  /// The next sibling Node of this Node. If any.
  _INTR_INLINE static NodeRef& _nextSibling(NodeRef p_Ref)
  {
    return _data.nextSibling[p_Ref._id];
  }

  // <-

  /// The (local) position.
  _INTR_INLINE static glm::vec3& _position(NodeRef p_Ref)
  {
    return _data.position[p_Ref._id];
  }
  /// The (local) orientation.
  _INTR_INLINE static glm::quat& _orientation(NodeRef p_Ref)
  {
    return _data.orientation[p_Ref._id];
  }
  /// The (local) size.
  _INTR_INLINE static glm::vec3& _size(NodeRef p_Ref)
  {
    return _data.size[p_Ref._id];
  }

  // <-

  /// The (world) position.
  _INTR_INLINE static glm::vec3& _worldPosition(NodeRef p_Ref)
  {
    return _data.worldPosition[p_Ref._id];
  }
  /// The (world) orientation.
  _INTR_INLINE static glm::quat& _worldOrientation(NodeRef p_Ref)
  {
    return _data.worldOrientation[p_Ref._id];
  }
  /// The (world) size.
  _INTR_INLINE static glm::vec3& _worldSize(NodeRef p_Ref)
  {
    return _data.worldSize[p_Ref._id];
  }

  /// The world transform/matrix.
  _INTR_INLINE static glm::mat4& _worldMatrix(NodeRef p_Ref)
  {
    return _data.worldMatrix[p_Ref._id];
  }
  /// The normal matrix.
  _INTR_INLINE static glm::mat4& _normalMatrix(NodeRef p_Ref)
  {
    return _data.normalMatrix[p_Ref._id];
  }

  /// The inverse of the world matrix.
  _INTR_INLINE static glm::mat4& _inverseWorldMatrix(NodeRef p_Ref)
  {
    return _data.inverseWorldMatrix[p_Ref._id];
  }

  /// The (world) axis aligned bounding box.
  _INTR_INLINE static Math::AABB& _worldAABB(NodeRef p_Ref)
  {
    return _data.worldAABB[p_Ref._id];
  }
  /// The (local) axis aligned bounding box.
  _INTR_INLINE static Math::AABB& _localAABB(NodeRef p_Ref)
  {
    return _data.localAABB[p_Ref._id];
  }

  /// The (world) bounding sphere.
  _INTR_INLINE static Math::Sphere& _worldBoundingSphere(NodeRef p_Ref)
  {
    return _data.worldBoundingSphere[p_Ref._id];
  }

  /// The visibility mask for each node.
  /// Stores the current visibility/culling information where each bit
  /// represents the visibility for an active
  /// Frustum in the scene. This is updated during culling.
  _INTR_INLINE static uint32_t& _visibilityMask(NodeRef p_Ref)
  {
    return _data.visibilityMask[p_Ref._id];
  }

  // <-

private:
  /// Adds the given Node to the root node array.
  _INTR_INLINE static void internalAddToRootNodeArray(NodeRef p_Ref)
  {
    _rootNodes.push_back(p_Ref);
  }

  // <-

  /// Removes the given Node from the root node array.
  _INTR_INLINE static void internalRemoveFromRootNodeArray(NodeRef p_Ref)
  {
    for (uint32_t i = 0u; i < _rootNodes.size(); ++i)
    {
      NodeRef currentNode = _rootNodes[i];

      if (currentNode == p_Ref)
      {
        // Erase and swap
        _rootNodes[i] = _rootNodes[_rootNodes.size() - 1u];
        _rootNodes.resize(_rootNodes.size() - 1u);
        break;
      }
    }
  }

  // <-

  /// The root nodes of the trees.
  static NodeRefArray _rootNodes;
  /// The sorted nodes of all trees.
  static NodeRefArray _sortedNodes;
};
}
}
}

namespace std
{
template <> class hash<Intrinsic::Core::Components::NodeRef>
{
public:
  size_t operator()(const Intrinsic::Core::Components::NodeRef& p_Ref) const
  {
    return p_Ref._id;
  }
};
};
