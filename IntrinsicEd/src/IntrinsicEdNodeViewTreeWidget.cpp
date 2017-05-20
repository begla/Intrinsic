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

// Precompiled header file
#include "stdafx_editor.h"
#include "stdafx.h"

// Helpers
void expandAllParents(QTreeWidgetItem* p_Item)
{
  p_Item->setExpanded(true);

  if (p_Item->parent())
  {
    expandAllParents(p_Item->parent());
  }
}

IntrinsicEdNodeViewTreeWidget::IntrinsicEdNodeViewTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
  QObject::connect(this, SIGNAL(customContextMenuRequested(QPoint)), this,
                   SLOT(onShowContextMenuForTreeView(QPoint)));
  QObject::connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
                   SLOT(onItemChanged(QTreeWidgetItem*, int)));
  QObject::connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
                   SLOT(onItemDoubleClicked(QTreeWidgetItem*, int)));
  QObject::connect(
      this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      this, SLOT(onItemSelected(QTreeWidgetItem*, QTreeWidgetItem*)));

  setContextMenuPolicy(Qt::CustomContextMenu);
  setHeaderHidden(true);
  setDragDropMode(QAbstractItemView::DragDrop);
  setSortingEnabled(true);
  sortByColumn(0u, Qt::AscendingOrder);

  onPopulateNodeTree();

  Resources::EventManager::connect(
      _N(NodeCreated),
      std::bind(&IntrinsicEdNodeViewTreeWidget::onNodeCreatedOrDestroyed, this,
                std::placeholders::_1));
  Resources::EventManager::connect(
      _N(NodeDestroyed),
      std::bind(&IntrinsicEdNodeViewTreeWidget::onNodeCreatedOrDestroyed, this,
                std::placeholders::_1));
  Resources::EventManager::connect(
      _N(CurrentlySelectedEntityChanged),
      std::bind(
          &IntrinsicEdNodeViewTreeWidget::onCurrentlySelectedEntityChanged,
          this, std::placeholders::_1));
}

IntrinsicEdNodeViewTreeWidget::~IntrinsicEdNodeViewTreeWidget() {}

void IntrinsicEdNodeViewTreeWidget::onNodeCreatedOrDestroyed(
    Resources::EventRef p_Event)
{
  onPopulateNodeTree();
}

void IntrinsicEdNodeViewTreeWidget::onPopulateNodeTree()
{
  // Store the current collapsed state of the tree
  for (auto it = _itemToNodeMap.begin(); it != _itemToNodeMap.end(); ++it)
  {
    QTreeWidgetItem* item = it->first;
    _nodeCollapsedState[it->second] = !item->isExpanded();
  }

  clear();
  _nodeToItemMap.clear();
  _itemToNodeMap.clear();

  for (uint32_t i = 0u; i < Components::NodeManager::getSortedNodeCount(); ++i)
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getSortedNodeAtIndex(i);

    // Hide spawned nodes
    // TODO: Visualize them separately
    if ((Components::NodeManager::_flags(nodeRef) &
         Components::NodeFlags::Flags::kSpawned) > 0u)
      continue;

    const Name& name =
        Entity::EntityManager::_name(Components::NodeManager::_entity(nodeRef));

    QTreeWidgetItem* item = nullptr;
    QString nodeTitle = name._string.c_str();

    if (!Components::NodeManager::_parent(nodeRef).isValid())
    {
      item = new QTreeWidgetItem(this);
      _nodeToItemMap[nodeRef] = item;

      item->setIcon(0, QIcon(":/Icons/globe"));
    }
    else
    {
      Components::NodeRef parentNodeRef =
          Components::NodeManager::_parent(nodeRef);
      QTreeWidgetItem* parentItem = _nodeToItemMap[parentNodeRef];

      item = new QTreeWidgetItem(parentItem);
      _nodeToItemMap[nodeRef] = item;
      item->setIcon(0, QIcon(":/Icons/target"));
    }

    if (i == 0u)
    {
      item->setExpanded(true);
    }

    // Restore collapsed state
    auto nodeCollapsedState = _nodeCollapsedState.find(nodeRef);
    if (nodeCollapsedState != _nodeCollapsedState.end())
    {
      item->setExpanded(!nodeCollapsedState->second);
    }

    item->setText(0, nodeTitle);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    _itemToNodeMap[item] = nodeRef;
    _INTR_ASSERT(item);
  }

  emit nodeTreePopulated(Components::NodeManager::getSortedNodeCount());
  onCurrentlySelectedEntityChanged(Resources::EventRef());
}

void IntrinsicEdNodeViewTreeWidget::createAddComponentContextMenu(QMenu* p_Menu)
{
  QTreeWidgetItem* currIt = currentItem();

  if (currIt)
  {
    Components::NodeRef currentNode = _itemToNodeMap[currIt];
    Entity::EntityRef entity = Components::NodeManager::_entity(currentNode);

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      const _INTR_STRING& compName = it->first._string;
      Dod::Components::ComponentManagerEntry& entry =
          Application::_componentManagerMapping[compName];

      if (!entry.getComponentForEntityFunction(entity).isValid())
      {
        QAction* createComp = new QAction(
            QIcon(IntrinsicEd::_componentToIconMapping[compName].c_str()),
            compName.c_str(), p_Menu);
        p_Menu->addAction(createComp);

        QObject::connect(createComp, SIGNAL(triggered()), this,
                         SLOT(onCreateComponent()));
      }
    }
  }
}

void IntrinsicEdNodeViewTreeWidget::createRemoveComponentContextMenu(
    QMenu* p_Menu)
{
  QTreeWidgetItem* currIt = currentItem();

  if (currIt)
  {
    Components::NodeRef currentNode = _itemToNodeMap[currIt];
    Entity::EntityRef entity = Components::NodeManager::_entity(currentNode);

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      const _INTR_STRING& compName = it->first._string;
      Dod::Components::ComponentManagerEntry& entry =
          Application::_componentManagerMapping[compName];

      if (entry.getComponentForEntityFunction(entity).isValid())
      {
        QAction* removeComp = new QAction(
            QIcon(IntrinsicEd::_componentToIconMapping[compName].c_str()),
            compName.c_str(), p_Menu);
        p_Menu->addAction(removeComp);

        QObject::connect(removeComp, SIGNAL(triggered()), this,
                         SLOT(onDestroyComponent()));
      }
    }
  }
}

void IntrinsicEdNodeViewTreeWidget::onCreateComponent()
{
  QTreeWidgetItem* currIt = currentItem();
  Components::NodeRef currentNode = _itemToNodeMap[currIt];
  Entity::EntityRef entity = Components::NodeManager::_entity(currentNode);

  QAction* action = (QAction*)QObject::sender();
  _INTR_STRING compName = action->text().toStdString().c_str();
  Dod::Components::ComponentManagerEntry& entry =
      Application::_componentManagerMapping[compName];

  _INTR_ASSERT(entry.createFunction);
  Dod::Ref compRef = entry.createFunction(entity);

  if (entry.resetToDefaultFunction)
  {
    entry.resetToDefaultFunction(compRef);
  }

  if (entry.createResourcesFunction)
  {
    Dod::RefArray components = {compRef};
    entry.createResourcesFunction(components);
  }

  if (entry.onInsertionDeletionFinishedAction)
  {
    entry.onInsertionDeletionFinishedAction();
  }

  onItemSelected(currIt, nullptr);
  onPopulateNodeTree();
}

void IntrinsicEdNodeViewTreeWidget::onDestroyComponent()
{
  QTreeWidgetItem* currIt = currentItem();
  Components::NodeRef currentNode = _itemToNodeMap[currIt];
  Entity::EntityRef entity = Components::NodeManager::_entity(currentNode);

  QAction* action = (QAction*)QObject::sender();
  _INTR_STRING compName = action->text().toStdString().c_str();
  Dod::Components::ComponentManagerEntry& entry =
      Application::_componentManagerMapping[compName];
  Dod::Ref componentRef = entry.getComponentForEntityFunction(entity);

  if (entry.destroyResourcesFunction)
  {
    Dod::RefArray components;
    components.push_back(componentRef);
    entry.destroyResourcesFunction(components);
  }

  _INTR_ASSERT(entry.destroyFunction);
  entry.destroyFunction(componentRef);

  if (entry.onInsertionDeletionFinishedAction)
  {
    entry.onInsertionDeletionFinishedAction();
  }

  onItemSelected(currIt, nullptr);
  onPopulateNodeTree();
}

void IntrinsicEdNodeViewTreeWidget::onShowContextMenuForTreeView(QPoint p_Pos)
{
  QMenu* contextMenu = new QMenu(this);

  QAction* createNode =
      new QAction(QIcon(":/Icons/plus"), "Create (Child) Node", this);
  contextMenu->addAction(createNode);

  QObject::connect(createNode, SIGNAL(triggered()), this, SLOT(onCreateNode()));

  QTreeWidgetItem* currIt = currentItem();
  if (currIt)
  {
    Components::NodeRef currentNode = _itemToNodeMap[currIt];

    // Don't allow deleting the main node
    if (World::getRootNode() != currentNode)
    {
      QAction* cloneNode =
          new QAction(QIcon(":/Icons/plus"), "Clone Node", this);
      contextMenu->addAction(cloneNode);
      QObject::connect(cloneNode, SIGNAL(triggered()), this,
                       SLOT(onCloneNode()));

      QAction* deleteNode =
          new QAction(QIcon(":/Icons/minus"), "Delete Node", this);
      contextMenu->addAction(deleteNode);
      QObject::connect(deleteNode, SIGNAL(triggered()), this,
                       SLOT(onDeleteNode()));
    }

    QMenu* addComponentMenu = new QMenu("Add Component", this);
    addComponentMenu->setIcon(QIcon(":/Icons/plus"));
    QMenu* removeComponentMenu = new QMenu("Remove Component", this);
    removeComponentMenu->setIcon(QIcon(":/Icons/minus"));

    createAddComponentContextMenu(addComponentMenu);
    createRemoveComponentContextMenu(removeComponentMenu);

    contextMenu->addSeparator();
    contextMenu->addMenu(addComponentMenu);
    contextMenu->addMenu(removeComponentMenu);
  }

  contextMenu->popup(viewport()->mapToGlobal(p_Pos));
}

void IntrinsicEdNodeViewTreeWidget::onCloneNode() { cloneNode(currentItem()); }

void IntrinsicEdNodeViewTreeWidget::onCreateNode()
{
  createNode(currentItem());
}

void IntrinsicEdNodeViewTreeWidget::onCreateRootNode() { createNode(nullptr); }

void IntrinsicEdNodeViewTreeWidget::createNode(QTreeWidgetItem* p_Parent)
{
  {
    Entity::EntityRef entityRef = Entity::EntityManager::createEntity("Node");
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);

    QTreeWidgetItem* item;
    if (p_Parent)
    {
      Components::NodeRef parentNode = _itemToNodeMap[p_Parent];
      Components::NodeManager::attachChild(parentNode, nodeRef);

      item = new QTreeWidgetItem(p_Parent);
      _nodeToItemMap[nodeRef] = item;
      _itemToNodeMap[item] = nodeRef;

      item->setText(0, Entity::EntityManager::_name(
                           Components::NodeManager::_entity(nodeRef))
                           ._string.c_str());
      item->setIcon(0, QIcon(":/Icons/target"));
    }
    else
    {
      item = new QTreeWidgetItem(this);
      _nodeToItemMap[nodeRef] = item;
      _itemToNodeMap[item] = nodeRef;

      item->setText(0, Entity::EntityManager::_name(
                           Components::NodeManager::_entity(nodeRef))
                           ._string.c_str());
      item->setIcon(0, QIcon(":/Icons/globe"));
    }

    _INTR_ASSERT(item);

    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void IntrinsicEdNodeViewTreeWidget::cloneNode(QTreeWidgetItem* p_Node)
{
  QTreeWidgetItem* currIt = currentItem();
  Components::NodeRef currentNode = _itemToNodeMap[currIt];

  World::cloneNodeFull(currentNode);
  onPopulateNodeTree();
}

void IntrinsicEdNodeViewTreeWidget::onDeleteNode()
{
  QTreeWidgetItem* currIt = currentItem();
  Components::NodeRef currentNode = _itemToNodeMap[currIt];

  // Update the currently selected entity
  if (Components::NodeManager::_entity(currentNode) ==
      GameStates::Editing::_currentlySelectedEntity)
  {
    GameStates::Editing::_currentlySelectedEntity = Dod::Ref();
  }

  Components::NodeRefArray nodes;
  Components::NodeManager::collectNodes(currentNode, nodes);

  // Cleanup cached nodes and all attached children
  for (uint32_t i = 0u; i < nodes.size(); ++i)
  {
    Components::NodeRef nodeRef = nodes[i];
    QTreeWidgetItem* item = _nodeToItemMap[nodeRef];

    _nodeToItemMap[nodeRef];
    _itemToNodeMap.erase(item);
  }

  onItemSelected(nullptr, currIt);
  delete currIt;

  // Delete node from world
  World::destroyNodeFull(currentNode);
}

void IntrinsicEdNodeViewTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
  event->acceptProposedAction();
}

QStringList IntrinsicEdNodeViewTreeWidget::mimeTypes() const
{
  QStringList forms = {"SingleNodeMimeData"};
  return forms;
}

QMimeData* IntrinsicEdNodeViewTreeWidget::mimeData(
    const QList<QTreeWidgetItem*> items) const
{
  if (!items.empty())
  {
    SingleNodeMimeData* mimeData = new SingleNodeMimeData();
    mimeData->node = items[0];

    return mimeData;
  }

  return nullptr;
}

bool IntrinsicEdNodeViewTreeWidget::dropMimeData(QTreeWidgetItem* p_Parent,
                                                 int p_Index,
                                                 const QMimeData* p_Data,
                                                 Qt::DropAction p_Action)
{
  if (p_Data && p_Data->hasFormat("SingleNodeMimeData"))
  {
    {
      SingleNodeMimeData* mimeData = (SingleNodeMimeData*)p_Data;

      QTreeWidgetItem* rootItem = mimeData->node;
      // Don't allow attaching nodes to themselves
      if (p_Parent == rootItem)
      {
        return false;
      }
      else
      {
        // Don't let nodes become their own children
        if (p_Parent)
        {
          QTreeWidgetItem* item = p_Parent->parent();

          while (item)
          {
            if (item == rootItem)
            {
              return false;
            }

            item = item->parent();
          }
        }
      }

      Components::NodeRef parentNode = Components::NodeRef();
      if (p_Parent)
      {
        parentNode = _itemToNodeMap[p_Parent];
      }

      Components::NodeRef rootNode = _itemToNodeMap[rootItem];

      if (Components::NodeManager::_parent(rootNode).isValid())
      {
        Components::NodeManager::detachChild(rootNode);
      }
      if (mimeData->node->parent())
      {
        mimeData->node->parent()->removeChild(mimeData->node);
      }
      else
      {
        invisibleRootItem()->removeChild(mimeData->node);
      }

      if (parentNode.isValid())
      {
        Components::NodeManager::attachChild(parentNode, rootNode);
      }
      if (p_Parent)
      {
        p_Parent->addChild(mimeData->node);
      }
      else
      {
        insertTopLevelItem(0, mimeData->node);
      }
    }
    Components::NodeManager::rebuildTree();

    return true;
  }

  return false;
}

void IntrinsicEdNodeViewTreeWidget::onItemChanged(QTreeWidgetItem* item,
                                                  int column)
{
  if (item)
  {
    auto currentItem = _itemToNodeMap.find(item);
    if (currentItem != _itemToNodeMap.end())
    {
      Components::NodeRef node = currentItem->second;
      Entity::EntityRef entityRef = Components::NodeManager::_entity(node);
      Name newName = item->text(column).toStdString().c_str();

      // Adjust the name of the node if it changed
      if (newName != Entity::EntityManager::_name(entityRef))
      {
        Entity::EntityManager::rename(entityRef, newName);
        item->setText(
            0, Entity::EntityManager::_name(entityRef).getString().c_str());
      }
    }
  }
}

void IntrinsicEdNodeViewTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item,
                                                        int column)
{
  editItem(item, column);
}

void IntrinsicEdNodeViewTreeWidget::onItemSelected(QTreeWidgetItem* current,
                                                   QTreeWidgetItem* previous)
{
  if (current && _itemToNodeMap.find(current) != _itemToNodeMap.end())
  {
    IntrinsicEd::_propertyView->clearPropertySet();

    Components::NodeRef node = _itemToNodeMap[current];
    Entity::EntityRef entity = Components::NodeManager::_entity(node);

    // Add entity
    {
      Dod::PropertyCompilerEntry entry;
      entry.compileFunction = Entity::EntityManager::compileDescriptor;
      entry.initFunction = Entity::EntityManager::initFromDescriptor;
      entry.ref = entity;

      Dod::Components::ComponentManagerEntry dummyManagerEntry;
      IntrinsicEd::_propertyView->addPropertySet(entry, dummyManagerEntry);
    }

    // Add components
    for (auto propCompIt =
             Application::_componentPropertyCompilerMapping.begin();
         propCompIt != Application::_componentPropertyCompilerMapping.end();
         ++propCompIt)
    {
      auto componentManagerIt =
          Application::_componentManagerMapping.find(propCompIt->first);
      if (componentManagerIt != Application::_componentManagerMapping.end())
      {
        Dod::Ref compRef =
            componentManagerIt->second.getComponentForEntityFunction(entity);

        if (compRef.isValid())
        {
          Dod::PropertyCompilerEntry entry = propCompIt->second;
          entry.ref = compRef;

          IntrinsicEd::_propertyView->addPropertySet(
              entry, componentManagerIt->second);
        }
      }
    }

    IntrinsicEd::_propertyView->clearAndUpdatePropertyView();

    // Set the currently selected object
    GameStates::Editing::_currentlySelectedEntity = entity;
  }
  else
  {
    IntrinsicEd::_propertyView->clearPropertySet();
    IntrinsicEd::_propertyView->clearPropertyView();
  }
}

void IntrinsicEdNodeViewTreeWidget::onCurrentlySelectedEntityChanged(
    Resources::EventRef p_Event)
{
  setCurrentItem(nullptr);

  if (GameStates::Editing::_currentlySelectedEntity.isValid())
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (nodeRef.isValid())
    {
      QTreeWidgetItem* item = _nodeToItemMap[nodeRef];

      if (item)
      {
        setCurrentItem(item);
        onItemSelected(item, nullptr);

        if (item->parent())
        {
          expandAllParents(item->parent());
        }
      }
    }
  }
}
