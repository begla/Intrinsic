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

class IntrinsicEdViewport : public QWidget
{
  Q_OBJECT

public:
  IntrinsicEdViewport(QWidget* parent = nullptr);
  ~IntrinsicEdViewport();
  void dropEvent(QDropEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;

  void positionNodeOnGround(Intrinsic::Core::Dod::Ref p_NodeRef);
  void spawnPrefab(const _INTR_STRING& p_PrefabFilePath);

  void onKeyPressed(Resources::EventRef p_EventRef);
  void onKeyReleased(Resources::EventRef p_EventRef);
  void onAxisChanged(Resources::EventRef p_EventRef);
  void onMouseMoved(Resources::EventRef p_EventRef);

private:
  Intrinsic::Core::Dod::Ref _currentPrefab;
};
