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
#include "stdafx_editor.h"
#include "stdafx.h"

using namespace CComponents;

IntrinsicEdViewport::IntrinsicEdViewport(QWidget* parent) : QWidget(parent)
{
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  setAcceptDrops(true);

  Resources::EventManager::connect(_N(MouseMoved),
                                   std::bind(&IntrinsicEdViewport::onMouseMoved,
                                             this, std::placeholders::_1));
  Resources::EventManager::connect(
      _N(AxisChanged), std::bind(&IntrinsicEdViewport::onAxisChanged, this,
                                 std::placeholders::_1));
  Resources::EventManager::connect(_N(KeyPressed),
                                   std::bind(&IntrinsicEdViewport::onKeyPressed,
                                             this, std::placeholders::_1));
  Resources::EventManager::connect(
      _N(KeyReleased), std::bind(&IntrinsicEdViewport::onKeyReleased, this,
                                 std::placeholders::_1));
}

IntrinsicEdViewport::~IntrinsicEdViewport() {}

void IntrinsicEdViewport::onKeyPressed(Resources::EventRef p_EventRef)
{
  const Resources::QueuedEventData& eventData =
      Resources::EventManager::_queuedEventData(p_EventRef);

  switch (eventData.keyEvent.key)
  {
  case Input::Key::kEscape:
    if (!IntrinsicEd::_mainWindow->_viewport->parent())
      IntrinsicEd::_mainWindow->onEndFullscreen();
    else
      IntrinsicEd::_mainWindow->onEditingGameState();
    break;
  case Input::Key::kF1:
    IntrinsicEd::_mainWindow->onEditingGameState();
    break;
  case Input::Key::kF2:
    IntrinsicEd::_mainWindow->onBenchmarkGameState();
    break;
  case Input::Key::kF3:
    IntrinsicEd::_mainWindow->onMainGameState();
    break;
  case Input::Key::kF10:
    IntrinsicEd::_mainWindow->onRecompileShaders();
    break;
  case Input::Key::k1:
    IntrinsicEd::_mainWindow->onEditingModeDefault();
    break;
  case Input::Key::k2:
    IntrinsicEd::_mainWindow->onEditingModeSelection();
    break;
  case Input::Key::k3:
    IntrinsicEd::_mainWindow->onEditingModeTranslation();
    break;
  case Input::Key::k4:
    IntrinsicEd::_mainWindow->onEditingModeRotation();
    break;
  case Input::Key::k5:
    IntrinsicEd::_mainWindow->onEditingModeScale();
    break;
  }
}

void IntrinsicEdViewport::dragEnterEvent(QDragEnterEvent* event)
{
  const QMimeData* mimeData = event->mimeData();
  if (mimeData->hasFormat("PrefabFilePath") && !_currentPrefab.isValid())
  {
    event->accept();

    QByteArray encodedData = mimeData->data("PrefabFilePath");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QString prefabName;
    stream >> prefabName;

    spawnPrefab(prefabName.toStdString().c_str());
  }
}

void IntrinsicEdViewport::dragLeaveEvent(QDragLeaveEvent* event)
{
  if (_currentPrefab.isValid())
  {
    World::destroyNodeFull(NodeManager::getComponentForEntity(_currentPrefab));

    if (GameStates::Editing::_currentlySelectedEntity == _currentPrefab)
    {
      GameStates::Editing::_currentlySelectedEntity =
          NodeManager::_entity(World::getRootNode());
    }

    _currentPrefab = Dod::Ref();
  }
}

_INTR_INLINE void IntrinsicEdViewport::positionNodeOnGround(Dod::Ref p_NodeRef)
{

  CameraRef camRef = World::getActiveCamera();
  NodeRef camNodeRef =
      NodeManager::getComponentForEntity(CameraManager::_entity(camRef));

  QPoint mousePos = mapFromGlobal(QCursor::pos());
  const glm::vec2 mousePosRel =
      glm::vec2((float)mousePos.x() / geometry().width(),
                (float)mousePos.y() / geometry().height());

  const Math::Ray worldRay = Math::calcMouseRay(
      NodeManager::_worldPosition(camNodeRef), mousePosRel,
      Components::CameraManager::_inverseViewProjectionMatrix(camRef));

  // Spawn the object close to the ground
  physx::PxRaycastHit hit;
  if (PhysxHelper::raycast(worldRay, hit, 50000.0f,
                           physx::PxQueryFlag::eSTATIC))
  {
    NodeManager::_position(p_NodeRef) = worldRay.o + worldRay.d * hit.distance;
  }
  else
  {
    NodeManager::_position(p_NodeRef) = worldRay.o + worldRay.d * 10.0f;
  }
}

void IntrinsicEdViewport::dragMoveEvent(QDragMoveEvent* event)
{
  if (_currentPrefab.isValid())
  {
    NodeRef nodeRef = NodeManager::getComponentForEntity(_currentPrefab);
    positionNodeOnGround(nodeRef);
    NodeManager::updateTransforms(nodeRef);

    if (IntrinsicEd::_prefabsBrowser->_ui.alignWithGround->isChecked())
    {
      World::alignNodeWithGround(nodeRef);
      NodeManager::updateTransforms(nodeRef);
    }

    setFocus();
  }

  IntrinsicEd::_mainWindow->tickMainLoop();
}

void IntrinsicEdViewport::dropEvent(QDropEvent* event)
{
  _currentPrefab = Dod::Ref();
}

void IntrinsicEdViewport::spawnPrefab(const _INTR_STRING& p_PrefabFilePath)
{
  NodeRef nodeRef = World::loadNodeHierarchy(p_PrefabFilePath);
  Entity::EntityRef entityRef = NodeManager::_entity(nodeRef);
  NodeManager::attachChild(World::getRootNode(), nodeRef);

  positionNodeOnGround(nodeRef);

  const glm::vec3 randomRotEuler = glm::vec3(
      Math::calcRandomFloatMinMax(
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMinX->value()),
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMaxX->value())),
      Math::calcRandomFloatMinMax(
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMinY->value()),
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMaxY->value())),
      Math::calcRandomFloatMinMax(
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMinZ->value()),
          glm::radians(IntrinsicEd::_prefabsBrowser->_ui.rotMaxZ->value())));
  const glm::vec2 randomScale =
      glm::vec2(Math::calcRandomFloatMinMax(
                    IntrinsicEd::_prefabsBrowser->_ui.scaleMinHor->value(),
                    IntrinsicEd::_prefabsBrowser->_ui.scaleMaxHor->value()),
                Math::calcRandomFloatMinMax(
                    IntrinsicEd::_prefabsBrowser->_ui.scaleMinVert->value(),
                    IntrinsicEd::_prefabsBrowser->_ui.scaleMaxVert->value()));

  NodeManager::_orientation(nodeRef) =
      NodeManager::_orientation(nodeRef) * glm::quat(randomRotEuler);
  NodeManager::_size(nodeRef) *=
      glm::vec3(randomScale.x, randomScale.y, randomScale.x);

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  World::loadNodeResources(nodeRef);

  GameStates::Editing::_currentlySelectedEntity = entityRef;
  _currentPrefab = entityRef;
}

void IntrinsicEdViewport::onKeyReleased(Resources::EventRef p_EventRef) {}

void IntrinsicEdViewport::onAxisChanged(Resources::EventRef p_EventRef) {}

void IntrinsicEdViewport::onMouseMoved(Resources::EventRef p_EventRef) {}
