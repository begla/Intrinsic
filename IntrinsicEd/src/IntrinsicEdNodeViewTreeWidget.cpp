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

using namespace RVResources;

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

    Entity::EntityRef entityRef = Components::NodeManager::_entity(nodeRef);
    const Name& name = Entity::EntityManager::_name(entityRef);

    QTreeWidgetItem* item = nullptr;
    QString nodeTitle = name.getString().c_str();

    if (!Components::NodeManager::_parent(nodeRef).isValid())
    {
      item = new QTreeWidgetItem(this);
      _nodeToItemMap[nodeRef] = item;

      item->setIcon(0, IntrinsicEd::getIcon("RootNode"));
    }
    else
    {
      Components::NodeRef parentNodeRef =
          Components::NodeManager::_parent(nodeRef);
      QTreeWidgetItem* parentItem = _nodeToItemMap[parentNodeRef];

      item = new QTreeWidgetItem(parentItem);
      _nodeToItemMap[nodeRef] = item;

      // Use the first component found as an icon
      QIcon iconToUse = IntrinsicEd::getIcon("Node");
      for (auto it = Application::_componentManagerMapping.begin();
           it != Application::_componentManagerMapping.end(); ++it)
      {
        const Name& compName = it->first;

        // Don't count nodes
        if (compName == "Node")
          continue;

        Dod::Components::ComponentManagerEntry& entry = it->second;
        if (entry.getComponentForEntityFunction(entityRef).isValid())
        {
          iconToUse = IntrinsicEd::getIcon(compName.getString());
          break;
        }
      }

      item->setIcon(0, iconToUse);
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

    QMap<QString, Dod::Components::ComponentManagerEntry*> sortedComponents;

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      const _INTR_STRING& compName = it->first.getString();
      Dod::Components::ComponentManagerEntry& entry = it->second;
      sortedComponents[compName.c_str()] = &entry;
    }

    for (auto it = sortedComponents.begin(); it != sortedComponents.end(); ++it)
    {
      const QString& compName = it.key();
      Dod::Components::ComponentManagerEntry& entry = *it.value();

      if (!entry.getComponentForEntityFunction(entity).isValid())
      {
        QAction* createComp =
            new QAction(IntrinsicEd::getIcon(compName.toStdString().c_str()),
                        compName.toStdString().c_str(), p_Menu);
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

    QMap<QString, Dod::Components::ComponentManagerEntry*> sortedComponents;

    for (auto it = Application::_componentManagerMapping.begin();
         it != Application::_componentManagerMapping.end(); ++it)
    {
      const _INTR_STRING& compName = it->first.getString();
      Dod::Components::ComponentManagerEntry& entry = it->second;
      sortedComponents[compName.c_str()] = &entry;
    }

    for (auto it = sortedComponents.begin(); it != sortedComponents.end(); ++it)
    {
      const QString& compName = it.key();

      // Nodes can't be deleted
      if (compName == "Node")
        continue;

      Dod::Components::ComponentManagerEntry& entry = *it.value();

      if (entry.getComponentForEntityFunction(entity).isValid())
      {
        QAction* removeComp =
            new QAction(IntrinsicEd::getIcon(compName.toStdString().c_str()),
                        compName.toStdString().c_str(), p_Menu);
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

  {
    QAction* createNode = new QAction(QIcon(":/Icons/icons/essential/plus.png"),
                                      "Create (Child) Node", this);
    contextMenu->addAction(createNode);
    QObject::connect(createNode, SIGNAL(triggered()), this,
                     SLOT(onCreateNode()));
  }

  QTreeWidgetItem* currIt = currentItem();
  if (currIt)
  {
    Components::NodeRef currentNode = _itemToNodeMap[currIt];
    Entity::EntityRef currentEntity =
        Components::NodeManager::_entity(currentNode);

    // Don't allow deleting the main node
    if (World::_rootNode != currentNode)
    {
      QAction* cloneNode =
          new QAction(QIcon(":/Icons/icons/cad/layer.png"), "Clone Node", this);
      contextMenu->addAction(cloneNode);
      QObject::connect(cloneNode, SIGNAL(triggered()), this,
                       SLOT(onCloneNode()));

      QAction* deleteNode = new QAction(
          QIcon(":/Icons/icons/essential/minus.png"), "Delete Node", this);
      contextMenu->addAction(deleteNode);
      QObject::connect(deleteNode, SIGNAL(triggered()), this,
                       SLOT(onDeleteNode()));
    }

    QMenu* addComponentMenu = new QMenu("Add Component", this);
    addComponentMenu->setIcon(QIcon(":/Icons/icons/essential/plus.png"));
    QMenu* removeComponentMenu = new QMenu("Remove Component", this);
    removeComponentMenu->setIcon(QIcon(":/Icons/icons/essential/minus.png"));

    createAddComponentContextMenu(addComponentMenu);
    createRemoveComponentContextMenu(removeComponentMenu);

    contextMenu->addSeparator();

    contextMenu->addMenu(addComponentMenu);
    contextMenu->addMenu(removeComponentMenu);

    Components::IrradianceProbeRef irradProbeRef =
        Components::IrradianceProbeManager::getComponentForEntity(
            currentEntity);
    if (irradProbeRef.isValid())
    {
      contextMenu->addSeparator();

      QAction* captureProbe =
          new QAction(QIcon(":/Icons/icons/various/idea.png"),
                      "Capture Irradiance Probe", this);
      contextMenu->addAction(captureProbe);
      QObject::connect(captureProbe, SIGNAL(triggered()), this,
                       SLOT(onCaptureIrradianceProbe()));

      QAction* captureAllProbes =
          new QAction(QIcon(":/Icons/icons/various/idea.png"),
                      "Capture ALL Irradiance Probes", this);
      contextMenu->addAction(captureAllProbes);
      QObject::connect(captureAllProbes, SIGNAL(triggered()), this,
                       SLOT(onCaptureAllIrradianceProbes()));
    }

    contextMenu->addSeparator();

    QAction* savePrefab =
        new QAction(IntrinsicEd::getIcon("Prefab"), "Save As Prefab", this);
    contextMenu->addAction(savePrefab);
    QObject::connect(savePrefab, SIGNAL(triggered()), this,
                     SLOT(onSaveNodeAsPrefab()));
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
                           .getString()
                           .c_str());
      item->setIcon(0, QIcon(":/Icons/target"));
    }
    else
    {
      item = new QTreeWidgetItem(this);
      _nodeToItemMap[nodeRef] = item;
      _itemToNodeMap[item] = nodeRef;

      item->setText(0, Entity::EntityManager::_name(
                           Components::NodeManager::_entity(nodeRef))
                           .getString()
                           .c_str());
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

  Components::NodeRef nodeRef = World::cloneNodeFull(currentNode);
  Entity::EntityRef entityRef = Components::NodeManager::_entity(nodeRef);
  GameStates::Editing::_currentlySelectedEntity = entityRef;

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

void IntrinsicEdNodeViewTreeWidget::onSaveNodeAsPrefab()
{
  QTreeWidgetItem* currIt = currentItem();
  Components::NodeRef currentNode = _itemToNodeMap[currIt];
  Entity::EntityRef currentEntity =
      Components::NodeManager::_entity(currentNode);

  const _INTR_STRING fileName =
      "media/prefabs/" +
      Entity::EntityManager::_name(currentEntity).getString() + ".prefab.json";

  const QString filePath =
      QFileDialog::getSaveFileName(this, tr("Save Prefab"), fileName.c_str(),
                                   tr("Prefab File (*.prefab.json)"));

  World::saveNodeHierarchy(filePath.toStdString().c_str(), currentNode);
}

void IntrinsicEdNodeViewTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
  event->acceptProposedAction();
}

QStringList IntrinsicEdNodeViewTreeWidget::mimeTypes() const
{
  return {"SingleNodeMimeData"};
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

namespace
{
void captureIrradProbe(
    const Components::IrradianceProbeRefArray& p_IrradProbeRefs, bool p_Clear,
    float p_Time)
{
  using namespace RV;

  static glm::uvec2 atlasIndices[6]{glm::uvec2(0u, 1u), glm::uvec2(1u, 1u),
                                    glm::uvec2(2u, 1u), glm::uvec2(3u, 1u),
                                    glm::uvec2(1u, 0u), glm::uvec2(1u, 2u)};
  static glm::quat rotationsPerAtlasIdx[6] = {
      glm::vec3(0.0f, glm::half_pi<float>(), 0.0f),  // L / +x
      glm::vec3(0.0f, 0.0f, 0.0f),                   // F / +z
      glm::vec3(0.0f, -glm::half_pi<float>(), 0.0f), // R / -x
      glm::vec3(0.0f, -glm::pi<float>(), 0.0f),      // B / -z
      glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f), // T / +y
      glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f)}; // B / -y
  static uint32_t atlasIndexToFaceIdx[6] = {0, 4, 1, 5, 2, 3};

  const glm::uvec2 cubeMapRes =
      RV::RenderSystem::getAbsoluteRenderSize(RV::RenderSize::kCubemap);

  const uint32_t faceSizeInBytes =
      cubeMapRes.x * cubeMapRes.y * 2u * sizeof(uint32_t);

  // Setup camera
  Entity::EntityRef entityRef =
      Entity::EntityManager::createEntity(_N(ProbeCamera));
  Components::NodeRef camNodeRef =
      Components::NodeManager::createNode(entityRef);
  Components::NodeManager::attachChild(World::_rootNode, camNodeRef);
  Components::CameraRef camRef =
      Components::CameraManager::createCamera(entityRef);
  Components::CameraManager::resetToDefault(camRef);
  Components::CameraManager::_descFov(camRef) = glm::radians(90.0f);
  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  Components::CameraRef prevCamera = World::_activeCamera;
  World::setActiveCamera(camRef);

  // Setup buffer for readback
  BufferRef readBackBufferRef =
      BufferManager::createBuffer(_N(IrradianceProbeReadBack));
  {
    BufferManager::resetToDefault(readBackBufferRef);
    BufferManager::addResourceFlags(
        readBackBufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);
    BufferManager::_descMemoryPoolType(readBackBufferRef) =
        RV::MemoryPoolType::kVolatileStagingBuffers;

    BufferManager::_descBufferType(readBackBufferRef) =
        RV::BufferType::kStorage;
    BufferManager::_descSizeInBytes(readBackBufferRef) = faceSizeInBytes;

    BufferManager::createResources({readBackBufferRef});
  }

  uint32_t prevDebugStageFlags = RenderPass::Debug::_activeDebugStageFlags;
  RenderPass::Debug::_activeDebugStageFlags = 0u;
  RenderPass::Clustering::_globalAmbientFactor = 0.0f;
  RenderPass::VolumetricLighting::_globalScatteringFactor = 0.0f;
  float prevTime = World::_currentTime;
  World::_currentTime = p_Time;
  float prevMaxFps = Settings::Manager::_targetFrameRate;
  Settings::Manager::_targetFrameRate = 0.0f;

  for (uint32_t i = 0u; i < p_IrradProbeRefs.size(); ++i)
  {
    Components::IrradianceProbeRef irradProbeRef = p_IrradProbeRefs[i];

    if (p_Clear)
    {
      Components::IrradianceProbeManager::_descSHs(irradProbeRef).clear();
    }

    Entity::EntityRef currentEntity =
        Components::IrradianceProbeManager::_entity(irradProbeRef);
    Components::NodeRef irradNodeRef =
        Components::NodeManager::getComponentForEntity(currentEntity);

    Components::NodeManager::_position(camNodeRef) =
        Components::NodeManager::_worldPosition(irradNodeRef);
    Components::NodeManager::updateTransforms({camNodeRef});

    // Render a couple of frames so everything is correctly faded in/out
    for (uint32_t f = 0u; f < 10u; ++f)
    {
      World::updateDayNightCycle(0.0f);
      Components::PostEffectVolumeManager::blendPostEffects(
          Components::PostEffectVolumeManager::_activeRefs);
      RenderProcess::Default::renderFrame(0.0f);
      ++TaskManager::_frameCounter;
      qApp->processEvents();
    }

    {
#if defined(STORE_ATLAS_DDS)
      gli::texture2d tex = gli::texture2d(
          gli::FORMAT_RGBA16_SFLOAT_PACK16,
          gli::texture2d::extent_type(cubeMapRes.x * 4u, cubeMapRes.y * 3u),
          1u);
#endif // STORE_ATLAS_DDS
      gli::texture_cube texCube =
          gli::texture_cube(gli::FORMAT_RGBA16_SFLOAT_PACK16, cubeMapRes, 1u);

      for (uint32_t atlasIdx = 0u; atlasIdx < 6; ++atlasIdx)
      {
        Components::NodeManager::_orientation(camNodeRef) =
            rotationsPerAtlasIdx[atlasIdx];
        Components::NodeManager::updateTransforms({camNodeRef});

        // Render face
        Components::PostEffectVolumeManager::blendPostEffects(
            Components::PostEffectVolumeManager::_activeRefs);
        RenderProcess::Default::renderFrame(0.0f);
        qApp->processEvents();

        // Wait for the rendering to finish
        RenderSystem::waitForAllFrames();

        // Copy image to host visible memory
        VkCommandBuffer copyCmd = RenderSystem::beginTemporaryCommandBuffer();

        ImageRef sceneImageRef = ImageManager::getResourceByName(_N(Scene));

        ImageManager::insertImageMemoryBarrier(
            copyCmd, sceneImageRef, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkBufferImageCopy bufferImageCopy = {};
        {
          bufferImageCopy.bufferOffset = 0u;
          bufferImageCopy.imageOffset = {};
          bufferImageCopy.bufferRowLength = cubeMapRes.x;
          bufferImageCopy.bufferImageHeight = cubeMapRes.y;
          bufferImageCopy.imageExtent.width = cubeMapRes.x;
          bufferImageCopy.imageExtent.height = cubeMapRes.y;
          bufferImageCopy.imageExtent.depth = 1u;
          bufferImageCopy.imageSubresource.aspectMask =
              VK_IMAGE_ASPECT_COLOR_BIT;
          bufferImageCopy.imageSubresource.baseArrayLayer = 0u;
          bufferImageCopy.imageSubresource.layerCount = 1u;
          bufferImageCopy.imageSubresource.mipLevel = 0u;
        }

        // Read back face
        vkCmdCopyImageToBuffer(copyCmd, ImageManager::_vkImage(sceneImageRef),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               BufferManager::_vkBuffer(readBackBufferRef), 1u,
                               &bufferImageCopy);

        RenderSystem::flushTemporaryCommandBuffer();

        const uint8_t* sceneMemory =
            BufferManager::getGpuMemory(readBackBufferRef);

        memcpy(texCube.data(0u, atlasIndexToFaceIdx[atlasIdx], 0u), sceneMemory,
               faceSizeInBytes);

#if defined(STORE_ATLAS_DDS)
        const glm::uvec2 atlasIndex = atlasIndices[atlasIdx];
        for (uint32_t scanLineIdx = 0u; scanLineIdx < cubeMapRes.y;
             ++scanLineIdx)
        {
          const uint32_t memOffsetInBytes =
              ((atlasIndex.y * cubeMapRes.y * (cubeMapRes.x * 4u)) +
               scanLineIdx * (cubeMapRes.x * 4u) +
               atlasIndex.x * cubeMapRes.x) *
              sizeof(uint32_t) * 2u;

          const uint32_t scanLineSizeInBytes =
              sizeof(uint32_t) * 2u * cubeMapRes.x;
          memcpy((uint8_t*)tex.data() + memOffsetInBytes,
                 sceneMemory + scanLineIdx * scanLineSizeInBytes,
                 scanLineSizeInBytes);
        }
#endif // STORE_ATLAS_DDS
      }

// Save textures
#if defined(STORE_ATLAS_DDS)
      {
        const _INTR_STRING filePath =
            "media/irradiance_probes/" +
            Entity::EntityManager::_name(currentEntity).getString() +
            "_cube_atlas_" + timeString + ".dds";
        gli::save_dds(tex, filePath.c_str());
      }

#endif // STORE_ATLAS_DDS

#if defined(STORE_CUBE_DDS)
      {
        _INTR_STRING timeString = StringUtil::toString(p_Time);
        StringUtil::replace(timeString, ".", "-");

        const _INTR_STRING filePath =
            "media/irradiance_probes/" +
            Entity::EntityManager::_name(currentEntity).getString() + "_cube_" +
            timeString + ".dds";
        gli::save_dds(texCube, filePath.c_str());
      }
#endif // STORE_CUBE_DDS

      // Store SH irrad.
      Components::IrradianceProbeManager::_descSHs(irradProbeRef)
          .push_back(Irradiance::project(texCube));
    }
  }

  // Cleanup and restore
  BufferManager::destroyResources({readBackBufferRef});
  BufferManager::destroyBuffer(readBackBufferRef);
  GpuMemoryManager::resetPool(MemoryPoolType::kVolatileStagingBuffers);

  Settings::Manager::_targetFrameRate = prevMaxFps;
  World::_currentTime = prevTime;
  RenderPass::Clustering::_globalAmbientFactor = 1.0f;
  RenderPass::VolumetricLighting::_globalScatteringFactor = 1.0f;
  RenderPass::Debug::_activeDebugStageFlags = prevDebugStageFlags;
  World::setActiveCamera(prevCamera);
  World::destroyNodeFull(camNodeRef);
}
}

const uint32_t _shTimeSamples = 8u;

void IntrinsicEdNodeViewTreeWidget::onCaptureIrradianceProbe()
{
  using namespace RV;

  Components::IrradianceProbeRef irradProbeRef;
  Components::NodeRef irradNodeRef;
  Entity::EntityRef currentEntity;

  QTreeWidgetItem* currIt = currentItem();
  if (currIt)
  {
    irradNodeRef = _itemToNodeMap[currIt];
    currentEntity = Components::NodeManager::_entity(irradNodeRef);
    irradProbeRef = Components::IrradianceProbeManager::getComponentForEntity(
        currentEntity);

    const glm::uvec2 cubeMapRes =
        RV::RenderSystem::getAbsoluteRenderSize(RV::RenderSize::kCubemap);
    RenderSystem::_customBackbufferDimensions = cubeMapRes;
    RenderSystem::resizeSwapChain(true);

    for (uint32_t i = 0u; i < _shTimeSamples; ++i)
      captureIrradProbe({irradProbeRef}, i == 0, i / (float)_shTimeSamples);

    RenderSystem::_customBackbufferDimensions = glm::uvec2(0u);
    RenderSystem::resizeSwapChain(true);
  }
}

void IntrinsicEdNodeViewTreeWidget::onCaptureAllIrradianceProbes()
{
  using namespace RV;

  const glm::uvec2 cubeMapRes =
      RV::RenderSystem::getAbsoluteRenderSize(RV::RenderSize::kCubemap);
  RenderSystem::_customBackbufferDimensions = cubeMapRes;
  RenderSystem::resizeSwapChain(true);

  for (uint32_t i = 0u; i < _shTimeSamples; ++i)
    captureIrradProbe(Components::IrradianceProbeManager::_activeRefs, i == 0,
                      i / (float)_shTimeSamples);

  RenderSystem::_customBackbufferDimensions = glm::uvec2(0u);
  RenderSystem::resizeSwapChain(true);
}