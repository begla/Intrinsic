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
  void onCreateComponent();
  void onDestroyComponent();
  void onItemDoubleClicked(QTreeWidgetItem* item, int column);
  void onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void onItemChanged(QTreeWidgetItem* item, int column);

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
