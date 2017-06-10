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
#include "stdafx.h"
#include "stdafx_editor.h"

IntrinsicEdPrefabsView::IntrinsicEdPrefabsView(QWidget* parent)
    : QTreeView(parent)
{
  _model = new IntrinsicEdPrefabModel();
  _model->setNameFilters({"*.prefab.json"});
  _model->setNameFilterDisables(false);
  setModel(_model);
  setRootIndex(_model->setRootPath("media/prefabs"));
  // setHeaderHidden(true);
  setColumnHidden(2, true);
}

IntrinsicEdPrefabsView::~IntrinsicEdPrefabsView() {}

// <-

QStringList IntrinsicEdPrefabModel::mimeTypes() const
{
  return {"PrefabFilePath"};
}

QMimeData*
IntrinsicEdPrefabModel::mimeData(const QModelIndexList& indexes) const
{
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  stream << filePath(indexes[0]);

  mimeData->setData("PrefabFilePath", encodedData);
  return mimeData;
}

QVariant IntrinsicEdPrefabModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DecorationRole && index.column() == 0)
  {
    QFileInfo info = fileInfo(index);

    if (info.isFile())
    {
      return IntrinsicEd::getPixmap("Prefab");
    }
    else if (info.isDir())
    {
      return IntrinsicEd::getPixmap("Folder");
    }
  }
  return QFileSystemModel::data(index, role);
}