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

#include "IntrinsicEdManagerWindowBase.h"

// Ui
#include "ui_IntrinsicEdManagerWindow.h"

IntrinsicEdManagerWindowBase::IntrinsicEdManagerWindowBase(QWidget* parent)
    : QWidget(parent)
{
  _ui.setupUi(this);

  QObject::connect(_ui.resourceView, SIGNAL(customContextMenuRequested(QPoint)),
                   this, SLOT(onShowResourceContextMenu(QPoint)));
  QObject::connect(_ui.createResource, SIGNAL(clicked()), this,
                   SLOT(onCreateResource()));
  QObject::connect(_ui.saveManager, SIGNAL(clicked()), this,
                   SLOT(onSaveManager()));
  QObject::connect(_ui.refreshManager, SIGNAL(clicked()), this,
                   SLOT(onPopulateResourceTree()));
  QObject::connect(
      _ui.resourceView,
      SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(onItemSelected(QTreeWidgetItem*, QTreeWidgetItem*)));
  QObject::connect(_ui.resourceView, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                   this, SLOT(onItemChanged(QTreeWidgetItem*, int)));

  _ui.propertyView->setFeatures(0);
  _ui.resourceView->setSortingEnabled(true);
  _ui.resourceView->sortByColumn(0u, Qt::AscendingOrder);
  _resourceName = "Resource";
}

void IntrinsicEdManagerWindowBase::onPopulateResourceTree()
{
  _ui.resourceView->clear();
  _resourceToItemMapping.clear();
  _itemToResourceMapping.clear();

  rapidjson::Document doc;

  const uint32_t resourceCount =
      (uint32_t)_resourceManagerEntry.getActiveResourceCountFunction();

  QTreeWidgetItem* volatileResourcesItem = new QTreeWidgetItem();
  volatileResourcesItem->setText(0, "Volatile");
  volatileResourcesItem->setIcon(0, QIcon(":/Icons/folder"));
  _ui.resourceView->addTopLevelItem(volatileResourcesItem);

  QTreeWidgetItem* storedResourcesItem = new QTreeWidgetItem();
  storedResourcesItem->setText(0, "Stored");
  storedResourcesItem->setIcon(0, QIcon(":/Icons/folder"));
  _ui.resourceView->addTopLevelItem(storedResourcesItem);

  for (uint32_t i = 0u; i < resourceCount; ++i)
  {
    Dod::Ref resource =
        _resourceManagerEntry.getActiveResourceAtIndexFunction(i);
    _INTR_ASSERT(resource.isValid());

    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
    _propertyCompilerEntry.compileFunction(resource, true, properties, doc);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, properties["name"]["value"].GetString());
    item->setIcon(0, _resourceIcon);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    _itemToResourceMapping[item] = resource;
    _resourceToItemMapping[resource] = item;

    _INTR_ASSERT(_resourceManagerEntry.getResourceFlagsFunction);
    if ((_resourceManagerEntry.getResourceFlagsFunction(resource) &
         Dod::Resources::ResourceFlags::kResourceVolatile) > 0u)
    {
      volatileResourcesItem->addChild(item);
    }
    else
    {
      storedResourcesItem->addChild(item);
    }
  }

  storedResourcesItem->setExpanded(true);
  emit resourceTreePopulated();
}

void IntrinsicEdManagerWindowBase::onShowResourceContextMenu(QPoint p_Pos)
{
  QMenu* contextMenu = new QMenu(this);
  initContextMenu(contextMenu);
  contextMenu->popup(_ui.resourceView->viewport()->mapToGlobal(p_Pos));
}

void IntrinsicEdManagerWindowBase::onItemChanged(QTreeWidgetItem* item,
                                                 int column)
{
  if (item)
  {
    rapidjson::Document doc;
    Dod::Ref resource = _itemToResourceMapping[item];

    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
    _propertyCompilerEntry.compileFunction(resource, true, properties, doc);

    if (strcmp(properties["name"]["value"].GetString(),
               item->text(0).toStdString().c_str()) != 0)
    {
      _INTR_STRING newResourceName =
          makeResourceNameUnique(item->text(0).toStdString().c_str());

      properties["name"]["value"].SetString(newResourceName.c_str(),
                                            doc.GetAllocator());
      item->setText(0, newResourceName.c_str());

      _propertyCompilerEntry.initFunction(resource, properties);
      _ui.propertyView->clearAndUpdatePropertyView();
    }
  }
}

_INTR_STRING
IntrinsicEdManagerWindowBase::makeResourceNameUnique(const char* p_Name)
{
  rapidjson::Document doc;

  _INTR_STRING newResourceName = p_Name;
  uint32_t resourceIndex = 1;

  while (true)
  {
    bool found = false;
    for (uint32_t i = 0u;
         i < _resourceManagerEntry.getActiveResourceCountFunction(); ++i)
    {
      Dod::Ref resource =
          _resourceManagerEntry.getActiveResourceAtIndexFunction(i);

      rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
      _propertyCompilerEntry.compileFunction(resource, false, properties, doc);

      if (properties["name"]["value"].GetString() == newResourceName)
      {
        const _INTR_STRING nameWithoutSuffix =
            StringUtil::stripNumberSuffix(p_Name);
        newResourceName =
            nameWithoutSuffix +
            StringUtil::toString<uint32_t>(resourceIndex++).c_str();
        found = true;
      }
    }

    if (!found)
    {
      break;
    }
  }

  return newResourceName;
}

void IntrinsicEdManagerWindowBase::onCreateResource()
{
  _INTR_STRING newResourceName =
      makeResourceNameUnique(_resourceName.toStdString().c_str());
  Dod::Ref newResourceRef =
      _resourceManagerEntry.createFunction(newResourceName.c_str());
  if (_resourceManagerEntry.resetToDefaultFunction)
  {
    _resourceManagerEntry.resetToDefaultFunction(newResourceRef);
  }
  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowBase::onCloneResource()
{
  Dod::Ref templateResourceRef =
      _itemToResourceMapping[_ui.resourceView->currentItem()];

  if (templateResourceRef.isValid())
  {
    Dod::Ref cloneedResourceRef =
        _resourceManagerEntry.createFunction(_N(Dummy));

    if (_resourceManagerEntry.resetToDefaultFunction)
    {
      _resourceManagerEntry.resetToDefaultFunction(cloneedResourceRef);
    }

    rapidjson::Document doc;
    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);

    _propertyCompilerEntry.compileFunction(templateResourceRef, false,
                                           properties, doc);
    properties["name"]["value"].SetString(
        makeResourceNameUnique(properties["name"]["value"].GetString()).c_str(),
        doc.GetAllocator());
    _propertyCompilerEntry.initFunction(cloneedResourceRef, properties);

    if (_resourceManagerEntry.createResourcesFunction)
    {
      Dod::RefArray resourcesToCreate = {cloneedResourceRef};
      _resourceManagerEntry.createResourcesFunction(resourcesToCreate);
    }

    onPopulateResourceTree();
  }
}

void IntrinsicEdManagerWindowBase::onDestroyResource()
{
  Dod::Ref resource = _itemToResourceMapping[_ui.resourceView->currentItem()];

  if (resource.isValid())
  {
    _resourceManagerEntry.destroyFunction(resource);

    onPopulateResourceTree();

    _ui.propertyView->clearPropertySet();
    _ui.propertyView->clearAndUpdatePropertyView();
  }
}

void IntrinsicEdManagerWindowBase::onSaveManager()
{
  if (_resourceManagerEntry.saveToSingleFileFunction)
  {
    _resourceManagerEntry.saveToSingleFileFunction(
        _managerFilePath.toStdString().c_str());
  }
  if (_resourceManagerEntry.saveToMultipleFilesFunction)
  {
    _resourceManagerEntry.saveToMultipleFilesFunction(
        _managerPath.toStdString().c_str(),
        _managerExtension.toStdString().c_str());
  }

  new IntrinsicEdNotificationSimple(this, "Saved manager to file...");
}

void IntrinsicEdManagerWindowBase::initContextMenu(QMenu* p_ContextMenu)
{
  QAction* createResource =
      new QAction(QIcon(":/Icons/plus"), "Create " + _resourceName, this);
  p_ContextMenu->addAction(createResource);
  QObject::connect(createResource, SIGNAL(triggered()), this,
                   SLOT(onCreateResource()));

  QTreeWidgetItem* currIt = _ui.resourceView->currentItem();
  Dod::Ref resRef = _itemToResourceMapping[currIt];

  if (resRef.isValid())
  {
    QAction* destroyResource =
        new QAction(QIcon(":/Icons/minus"), "Delete " + _resourceName, this);
    p_ContextMenu->addAction(destroyResource);
    QObject::connect(destroyResource, SIGNAL(triggered()), this,
                     SLOT(onDestroyResource()));

    p_ContextMenu->addSeparator();

    QAction* cloneResource =
        new QAction(QIcon(":/Icons/plus"), "Clone " + _resourceName, this);
    p_ContextMenu->addAction(cloneResource);
    QObject::connect(cloneResource, SIGNAL(triggered()), this,
                     SLOT(onCloneResource()));
  }
}

void IntrinsicEdManagerWindowBase::onItemSelected(QTreeWidgetItem* current,
                                                  QTreeWidgetItem* previous)
{
  if (current)
  {
    Dod::Ref resource = _itemToResourceMapping[current];

    if (resource.isValid())
    {
      Dod::PropertyCompilerEntry entry;
      entry.compileFunction = _propertyCompilerEntry.compileFunction;
      entry.initFunction = _propertyCompilerEntry.initFunction;
      entry.ref = resource;

      _ui.propertyView->clearPropertySet();
      _ui.propertyView->addPropertySet(entry, _resourceManagerEntry);
      _ui.propertyView->clearAndUpdatePropertyView();
    }
  }
}
