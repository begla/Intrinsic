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
  void onGridSizeChanged(double p_Value);
  void onGizmoSizeChanged(double p_Value);
  void onCameraSpeedChanged(double p_Value);
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

private:
  QByteArray _tempStoredGeometry;
  QByteArray _tempStoredState;

  QMenu _createContextMenu;
  QMenu _debugContextMenu;
  QMenu _debugGeometryContextMenu;

  Ui::IntrinsicEdClass _ui;
};
