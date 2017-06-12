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

#pragma once

struct SingleNodeMimeData : public QMimeData
{
  QTreeWidgetItem* node;

  SingleNodeMimeData() : node(nullptr)
  {
    forms.push_back("SingleNodeMimeData");
  }

  QStringList formats() const { return forms; }

  QStringList forms;
};

class IntrinsicEdNodeViewTreeWidget : public QTreeWidget
{
  typedef _INTR_HASH_MAP(Components::NodeRef, QTreeWidgetItem*) NodeToItemMap;
  typedef _INTR_HASH_MAP(Components::NodeRef, bool) NodeCollapsedState;
  typedef _INTR_HASH_MAP(QTreeWidgetItem*, Components::NodeRef) ItemToNodeMap;

  Q_OBJECT

public:
  IntrinsicEdNodeViewTreeWidget(QWidget* parent = 0);
  ~IntrinsicEdNodeViewTreeWidget();

  void dragEnterEvent(QDragEnterEvent* event);
  bool dropMimeData(QTreeWidgetItem* p_Parent, int p_Index,
                    const QMimeData* p_Data, Qt::DropAction p_Action);
  QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;
  QStringList mimeTypes() const;

  void onNodeCreatedOrDestroyed(Resources::EventRef p_Event);
  void onCurrentlySelectedEntityChanged(Resources::EventRef p_Event);

public slots:
  void onPopulateNodeTree();
  void onShowContextMenuForTreeView(QPoint p_Pos);
  void onCreateNode();
  void onCreateRootNode();
  void onCloneNode();
  void onDeleteNode();
  void onSaveNodeAsPrefab();
  void onCreateComponent();
  void onDestroyComponent();
  void onItemDoubleClicked(QTreeWidgetItem* item, int column);
  void onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void onItemChanged(QTreeWidgetItem* item, int column);
  void onCaptureIrradianceProbe();
  void onCaptureAllIrradianceProbes();

signals:
  void nodeTreePopulated(uint32_t p_NodeCount);

private:
  void createAddComponentContextMenu(QMenu* p_Menu);
  void createRemoveComponentContextMenu(QMenu* p_Menu);
  void createNode(QTreeWidgetItem* p_Parent);
  void cloneNode(QTreeWidgetItem* p_Node);

  NodeToItemMap _nodeToItemMap;
  ItemToNodeMap _itemToNodeMap;
  NodeCollapsedState _nodeCollapsedState;
};
