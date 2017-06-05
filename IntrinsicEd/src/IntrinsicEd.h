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
#include "ui_IntrinsicEd.h"

// Fwd. decls.
class IntrinsicEdNodeView;
class IntrinsicEdPropertyView;
class IntrinsicEdManagerWindowGpuProgram;
class IntrinsicEdManagerWindowScript;
class IntrinsicEdManagerWindowAsset;
class IntrinsicEdManagerWindowMesh;
class IntrinsicEdManagerWindowMaterial;
class IntrinsicEdManagerWindowImage;
class IntrinsicEdManagerWindowPostEffect;
class IntrinsicEdViewport;

class IntrinsicEd : public QMainWindow
{
  Q_OBJECT

public:
  IntrinsicEd(QWidget* parent = 0);
  ~IntrinsicEd();

  void closeEvent(QCloseEvent*) override;
  int enterMainLoop();

  static IntrinsicEd* _mainWindow;
  static IntrinsicEdNodeView* _nodeView;
  static IntrinsicEdPropertyView* _propertyView;
  static IntrinsicEdManagerWindowGpuProgram* _managerWindowGpuProgram;
  static IntrinsicEdManagerWindowScript* _managerWindowScript;
  static IntrinsicEdManagerWindowAsset* _managerWindowAsset;
  static IntrinsicEdManagerWindowImage* _managerWindowImage;
  static IntrinsicEdManagerWindowMesh* _managerWindowMesh;
  static IntrinsicEdManagerWindowMaterial* _managerWindowMaterial;
  static IntrinsicEdManagerWindowPostEffect* _managerWindowPostEffect;
  static IntrinsicEdViewport* _viewport;

  static const QIcon& getIcon(const _INTR_STRING& p_String)
  {
    return _icons[_stringToIconMapping[p_String]];
  }

  static const QPixmap& getPixmap(const _INTR_STRING& p_Pixmap)
  {
    return _pixmaps[_stringToPixmapMapping[p_Pixmap]];
  }

public slots:
  void onSaveEditorSettings();
  void onLoadEditorSettings();
  void onExit();
  void onLoadWorld();
  void onReloadWorld();
  void onReloadSettingsAndRendererConfig();
  void onSaveWorld();
  void onSaveWorldAs();
  void onFullscreen();
  void onEndFullscreen();
  void onEditingModeDefault();
  void onEditingModeSelection();
  void onEditingModeTranslation();
  void onEditingModeRotation();
  void onEditingModeScale();
  void onEditingGameState();
  void onMainGameState();
  void onBenchmarkGameState();
  void onGridSizeChanged(double p_Value);
  void onGizmoSizeChanged(double p_Value);
  void onCameraSpeedChanged(double p_Value);
  void onTimeModChanged(double p_Value);
  void onDayNightSliderChanged(int p_Value);
  void onCreateCube();
  void onCreateRigidBody();
  void onCreateRigidBodySphere();
  void onCreateLight();
  void onCreateSphere();
  void onShowDebugGeometryContextMenu();
  void onDebugGeometryChanged();
  void onOpenMicroprofile();
  void onCompileShaders();
  void onRecompileShaders();
  void onSettingsFileChanged(const QString&);

private:
  void updateSettingsChangeWatch();
  void updateUI();

  QMenu _debugGeometryContextMenu;

  QSlider* _dayNightSlider;
  QFileSystemWatcher* _settingsChangeWatch;
  bool _settingsUpdatePending;

  static _INTR_ARRAY(QIcon) _icons;
  static _INTR_ARRAY(QPixmap) _pixmaps;
  static _INTR_HASH_MAP(_INTR_STRING, uint32_t) _stringToIconMapping;
  static _INTR_HASH_MAP(_INTR_STRING, uint32_t) _stringToPixmapMapping;

  Ui::IntrinsicEdClass _ui;
};
