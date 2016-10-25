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
