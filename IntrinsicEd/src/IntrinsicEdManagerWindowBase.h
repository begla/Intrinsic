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

// UI related includes
#include "ui_IntrinsicEdManagerWindow.h"

class IntrinsicEdManagerWindowBase : public QWidget
{
  Q_OBJECT

public:
  IntrinsicEdManagerWindowBase(QWidget* parent);

  virtual void initContextMenu(QMenu* p_ContextMenu);

public slots:
  void onShowResourceContextMenu(QPoint p_Pos);
  void onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
  void onCreateResource();
  void onDestroyResource();
  void onCloneResource();
  virtual void onPopulateResourceTree();
  void onSaveManager();
  void onItemChanged(QTreeWidgetItem* item, int column);

signals:
  void resourceTreePopulated();

protected:
  _INTR_STRING makeResourceNameUnique(const char* p_Name);

  Ui::IntrinsicEdManagerWindowClass _ui;
  Dod::Resources::ResourceManagerEntry _resourceManagerEntry;
  Dod::PropertyCompilerEntry _propertyCompilerEntry;
  QIcon _resourceIcon;
  QString _managerFilePath;
  QString _managerPath;
  QString _managerExtension;
  QString _resourceName;

  _INTR_HASH_MAP(QTreeWidgetItem*, Dod::Ref) _itemToResourceMapping;
  _INTR_HASH_MAP(Dod::Ref, QTreeWidgetItem*) _resourceToItemMapping;
};
