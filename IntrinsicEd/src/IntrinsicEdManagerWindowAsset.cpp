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
#include "stdafx_assets.h"

using namespace Intrinsic::AssetManagement::Resources;

IntrinsicEdManagerWindowAsset::IntrinsicEdManagerWindowAsset(QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("Assets");
  setAcceptDrops(true);
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(Asset)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(Asset)];
  _resourceIcon = QIcon(":/Icons/asset");
  _managerFilePath = "managers/Asset.manager.json";
  _resourceName = "Asset";

  _assetChangeWatch = new QFileSystemWatcher(this);
  QObject::connect(_assetChangeWatch, SIGNAL(fileChanged(const QString&)), this,
                   SLOT(onAssetChanged(const QString&)));
  QObject::connect(this, SIGNAL(resourceTreePopulated()), this,
                   SLOT(onResourceTreePopulated()));
  QObject::connect(&_assetRecompileTimer, SIGNAL(timeout()),
                   SLOT(onCompileQueuedAssets()));

  _assetRecompileTimer.setSingleShot(true);

  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowAsset::onPopulateResourceTree()
{
  _ui.resourceView->clear();
  _resourceToItemMapping.clear();
  _itemToResourceMapping.clear();

  rapidjson::Document doc;

  const uint32_t resourceCount = AssetManager::getActiveResourceCount();

  QTreeWidgetItem* textures = new QTreeWidgetItem();
  {
    textures->setText(0, "Textures");
    textures->setIcon(0, QIcon(":/Icons/picture"));
    _ui.resourceView->addTopLevelItem(textures);
  }

  QTreeWidgetItem* meshes = new QTreeWidgetItem();
  {
    meshes->setText(0, "Meshes");
    meshes->setIcon(0, QIcon(":/Icons/user"));
    _ui.resourceView->addTopLevelItem(meshes);
  }

  QTreeWidgetItem* generalAssets = new QTreeWidgetItem();
  {
    generalAssets->setText(0, "General");
    generalAssets->setIcon(0, QIcon(":/Icons/asset"));
    _ui.resourceView->addTopLevelItem(generalAssets);
  }

  for (uint32_t i = 0u; i < resourceCount; ++i)
  {
    Dod::Ref assetEntry = AssetManager::getActiveResourceAtIndex(i);
    _INTR_ASSERT(assetEntry.isValid());

    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
    _propertyCompilerEntry.compileFunction(assetEntry, properties, doc);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, properties["name"]["value"].GetString());
    item->setIcon(0, _resourceIcon);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    _itemToResourceMapping[item] = assetEntry;
    _resourceToItemMapping[assetEntry] = item;

    if (AssetManager::_descAssetType(assetEntry) ==
        Intrinsic::AssetManagement::Resources::AssetType::kMesh)
    {
      meshes->addChild(item);
    }
    else if (
        AssetManager::_descAssetType(assetEntry) ==
            Intrinsic::AssetManagement::Resources::AssetType::kColorTexture ||
        AssetManager::_descAssetType(assetEntry) ==
            Intrinsic::AssetManagement::Resources::AssetType::kAlphaTexture ||
        AssetManager::_descAssetType(assetEntry) ==
            Intrinsic::AssetManagement::Resources::AssetType::kNormalTexture ||
        AssetManager::_descAssetType(assetEntry) ==
            Intrinsic::AssetManagement::Resources::AssetType::kHdrTexture)
    {
      textures->addChild(item);
    }
    else
    {
      generalAssets->addChild(item);
    }
  }

  textures->setExpanded(true);
  meshes->setExpanded(true);
  generalAssets->setExpanded(true);

  emit resourceTreePopulated();
}

void IntrinsicEdManagerWindowAsset::initContextMenu(QMenu* p_ContextMenu)
{
  IntrinsicEdManagerWindowBase::initContextMenu(p_ContextMenu);

  QTreeWidgetItem* currIt = _ui.resourceView->currentItem();
  Dod::Ref resRef = _itemToResourceMapping[currIt];

  if (resRef.isValid())
  {
    p_ContextMenu->addSeparator();

    QAction* compileAsset =
        new QAction(QIcon(":/Icons/asset"), "Compile " + _resourceName, this);
    p_ContextMenu->addAction(compileAsset);
    QObject::connect(compileAsset, SIGNAL(triggered()), this,
                     SLOT(onCompileAsset()));
  }
}

void IntrinsicEdManagerWindowAsset::onCompileAsset()
{
  QTreeWidgetItem* currIt = _ui.resourceView->currentItem();
  Dod::Ref resource = _itemToResourceMapping[currIt];

  if (resource.isValid())
  {
    if (std::find(_assetsToRecompile.begin(), _assetsToRecompile.end(),
                  resource) == _assetsToRecompile.end())
    {
      _assetsToRecompile.push_back(resource);
      _assetRecompileTimer.start(100);
    }
  }
}

void IntrinsicEdManagerWindowAsset::onAssetChanged(const QString& p_FileName)
{
  // Recompile assets

  for (uint32_t i = 0u; i < AssetManager::getActiveResourceCount(); ++i)
  {
    AssetRef assetRef = AssetManager::getActiveResourceAtIndex(i);

    _INTR_STRING fileName, extension;
    StringUtil::extractFileNameAndExtension(p_FileName.toStdString().c_str(),
                                            fileName, extension);

    if (AssetManager::_descAssetFileName(assetRef) == (fileName + extension))
    {
      if (std::find(_assetsToRecompile.begin(), _assetsToRecompile.end(),
                    assetRef) == _assetsToRecompile.end())
      {
        _assetsToRecompile.push_back(assetRef);
      }
    }
  }

  _assetRecompileTimer.start(1000);
}

void IntrinsicEdManagerWindowAsset::onResourceTreePopulated()
{
  QStringList files = _assetChangeWatch->files();

  if (!files.empty())
    _assetChangeWatch->removePaths(files);

  for (uint32_t i = 0u; i < AssetManager::getActiveResourceCount(); ++i)
  {
    AssetRef ref = AssetManager::getActiveResourceAtIndex(i);

    if (AssetManager::_descAssetType(ref) == AssetType::kMesh)
    {
      _assetChangeWatch->addPath(
          QString(Settings::Manager::_assetMeshPath.c_str()) + "/" +
          AssetManager::_descAssetFileName(ref).c_str());
    }
    else if (AssetManager::_descAssetType(ref) == AssetType::kColorTexture ||
             AssetManager::_descAssetType(ref) == AssetType::kAlphaTexture ||
             AssetManager::_descAssetType(ref) == AssetType::kHdrTexture ||
             AssetManager::_descAssetType(ref) == AssetType::kNormalTexture)
    {
      _assetChangeWatch->addPath(
          QString(Settings::Manager::_assetTexturePath.c_str()) + "/" +
          AssetManager::_descAssetFileName(ref).c_str());
    }
  }
}

void IntrinsicEdManagerWindowAsset::onCompileQueuedAssets()
{
  Intrinsic::AssetManagement::Resources::AssetManager::compileAssets(
      _assetsToRecompile);
  _assetsToRecompile.clear();

  onResourceTreePopulated();
}

void IntrinsicEdManagerWindowAsset::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept();
}

void IntrinsicEdManagerWindowAsset::dropEvent(QDropEvent* event)
{
  const QMimeData* mimeData = event->mimeData();

  if (mimeData->hasUrls())
  {
    QStringList pathList;
    QList<QUrl> urlList = mimeData->urls();

    AssetRef lastCreatedAsset = Dod::Ref();

    for (int i = 0; i < urlList.size(); ++i)
    {
      QString path = urlList.at(i).toLocalFile();

      _INTR_STRING fileName, extension;
      StringUtil::extractFileNameAndExtension(path.toStdString().c_str(),
                                              fileName, extension);

      AssetRef existingAsset = AssetManager::_getResourceByName(fileName);
      if (!existingAsset.isValid())
      {
        AssetType::Enum assetType = AssetType::kNone;
        if (extension == ".fbx")
        {
          assetType = AssetType::kMesh;
        }
        else if (extension == ".TGA" || extension == ".tga")
        {
          assetType = AssetType::kColorTexture;
        }

        if (assetType != AssetType::kNone)
        {
          AssetRef assetRef = AssetManager::createAsset(fileName);
          AssetManager::resetToDefault(assetRef);

          AssetManager::_descAssetFileName(assetRef) = fileName + extension;
          AssetManager::_descAssetType(assetRef) = assetType;

          lastCreatedAsset = assetRef;
        }
      }
      else
      {
        QTreeWidgetItem* item = _resourceToItemMapping[existingAsset];
        item->setSelected(true);
        item->parent()->setExpanded(true);
        onItemSelected(item, nullptr);
      }
    }

    if (lastCreatedAsset.isValid())
    {
      onPopulateResourceTree();
      QTreeWidgetItem* item = _resourceToItemMapping[lastCreatedAsset];

      item->setSelected(true);
      item->parent()->setExpanded(true);
      onItemSelected(item, nullptr);
    }
  }
}