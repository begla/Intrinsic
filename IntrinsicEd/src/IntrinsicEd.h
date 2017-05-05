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

  static _INTR_HASH_MAP(_INTR_STRING, _INTR_STRING) _categoryToIconMapping;
  static _INTR_HASH_MAP(_INTR_STRING, _INTR_STRING) _componentToIconMapping;

public slots:
  void onSaveEditorSettings();
  void onLoadEditorSettings();
  void onExit();
  void onLoadWorld();
  void onReloadSettingsAndRendererConfig();
  void onSaveWorld();
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
  void onCreateCube();
  void onCreateRigidBody();
  void onCreateRigidBodySphere();
  void onCreateSphere();
  void onSpawnVegetation();
  void onSpawnGrass();
  void onShowDebugContextMenu();
  void onShowDebugGeometryContextMenu();
  void onShowCreateContextMenu();
  void onDebugGeometryChanged();
  void onOpenMicroprofile();
  void onCompileShaders();
  void onRecompileShaders();

private:
  QMenu _createContextMenu;
  QMenu _debugContextMenu;
  QMenu _debugGeometryContextMenu;

  Ui::IntrinsicEdClass _ui;
};
