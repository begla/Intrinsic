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

class IntrinsicEdManagerWindowAsset : public IntrinsicEdManagerWindowBase
{
  Q_OBJECT

public:
  IntrinsicEdManagerWindowAsset(QWidget* parent);
  virtual void initContextMenu(QMenu* p_ContextMenu) override;
  virtual void onPopulateResourceTree() override;
  void dropEvent(QDropEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;

public slots:
  void onAssetChanged(const QString& p_FileName);
  void onResourceTreePopulated();
  void onCompileAsset();
  void onCompileQueuedAssets();

private:
  QFileSystemWatcher* _assetChangeWatch;
  QTimer _assetRecompileTimer;
  Dod::RefArray _assetsToRecompile;
};
