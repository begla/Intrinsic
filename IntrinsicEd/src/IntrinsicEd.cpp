// ->
// Copyright (c) 2016 Benjamin Glatzel
// Author: Benjamin Glatzel
// <-

// Precompiled header file
#include "stdafx_editor.h"
#include "stdafx.h"
#include "stdafx_assets.h"

// Ui
#include "ui_IntrinsicEd.h"

IntrinsicEdNodeView* IntrinsicEd::_nodeView = nullptr;
IntrinsicEdPropertyView* IntrinsicEd::_propertyView = nullptr;
IntrinsicEdManagerWindowGpuProgram* IntrinsicEd::_managerWindowGpuProgram =
    nullptr;
IntrinsicEdManagerWindowScript* IntrinsicEd::_managerWindowScript = nullptr;
IntrinsicEdManagerWindowAsset* IntrinsicEd::_managerWindowAsset = nullptr;
IntrinsicEdManagerWindowMesh* IntrinsicEd::_managerWindowMesh = nullptr;
IntrinsicEdManagerWindowMaterial* IntrinsicEd::_managerWindowMaterial = nullptr;
IntrinsicEdManagerWindowImage* IntrinsicEd::_managerWindowImage = nullptr;
IntrinsicEdManagerWindowPostEffect* IntrinsicEd::_managerWindowPostEffect =
    nullptr;
IntrinsicEd* IntrinsicEd::_mainWindow = nullptr;
IntrinsicEdViewport* IntrinsicEd::_viewport = nullptr;

_INTR_HASH_MAP(_INTR_STRING, _INTR_STRING) IntrinsicEd::_categoryToIconMapping;
_INTR_HASH_MAP(_INTR_STRING, _INTR_STRING) IntrinsicEd::_componentToIconMapping;

SDL_Window* _sdlMainWindow = nullptr;
SDL_Window* _sdlViewport = nullptr;

// Workaround: Make SDL work transparently with QT using WIN32
#if defined(_WIN32)
WNDPROC _qtWindowProc;
WNDPROC _sdlWindowProc;

LRESULT DummyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return 0u;
}

LRESULT FilterWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  const bool hasFocus =
      IntrinsicEd::_viewport && IntrinsicEd::_viewport->hasFocus();

  bool isFocusEvent = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN ||
                       msg == WM_KEYUP || msg == WM_SYSKEYUP ||
                       (msg >= WM_IME_SETCONTEXT && msg <= WM_IME_KEYUP)) ||
                      msg == WM_INPUT || msg == WM_SETCURSOR;
  bool passEventToSdl =
      (isFocusEvent && hasFocus) || (!isFocusEvent && msg != WM_PAINT);

  if (passEventToSdl)
  {
    CallWindowProc(_sdlWindowProc, hwnd, msg, wParam, lParam);
  }

  return CallWindowProc(_qtWindowProc, hwnd, msg, wParam, lParam);
}
#endif // _WIN32

IntrinsicEd::IntrinsicEd(QWidget* parent) : QMainWindow(parent)
{
  // Loading settings file
  Settings::Manager::loadSettings();

  // Init. event system
  Application::initEventSystem();

  _ui.setupUi(this);
  setWindowTitle(QString("IntrinsicEd - ") + _INTR_VERSION_STRING);

  // Init. Intrinsic
  Application::init(GetModuleHandle(NULL), (void*)_ui.viewPort->winId());

  // Activate editing game state
  GameStates::Manager::activateGameState(GameStates::GameState::kEditing);

  // Init. resources
  {
    Intrinsic::AssetManagement::Resources::AssetManager::init();
    Intrinsic::AssetManagement::Resources::AssetManager::loadFromMultipleFiles(
        "managers/assets/", ".asset.json");
  }

  _viewport = _ui.viewPort;

#if defined(_WIN32)
  _qtWindowProc = (WNDPROC)GetWindowLongPtr((HWND)winId(), GWLP_WNDPROC);
  SetWindowLongPtr((HWND)winId(), GWLP_WNDPROC, (LONG_PTR)DummyWindowProc);
#endif // _WIN32

  // Init. SDL
  int sdlResult =
      SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
  _INTR_ASSERT(sdlResult == 0u);

  _sdlViewport = SDL_CreateWindowFrom((void*)_ui.viewPort->winId());
  _INTR_ASSERT(_sdlViewport);

  _sdlMainWindow = SDL_CreateWindowFrom((void*)winId());
  _INTR_ASSERT(_sdlMainWindow);

#if defined(_WIN32)
  _sdlWindowProc = (WNDPROC)GetWindowLongPtr((HWND)winId(), GWLP_WNDPROC);
  SetWindowLongPtr((HWND)winId(), GWLP_WNDPROC, (LONG_PTR)FilterWindowProc);
#endif // _WIN32

  // Setup category/component mappings
  {
    _categoryToIconMapping["NodeLocalTransform"] = ":Icons/drawArrow";
    _categoryToIconMapping["NodeWorldTransform"] = ":Icons/globe";
    _categoryToIconMapping["Entity"] = ":Icons/lightbulb";
    _categoryToIconMapping["Mesh"] = ":Icons/user";
    _categoryToIconMapping["GpuProgram"] = ":Icons/calendar";
    _categoryToIconMapping["Resource"] = ":Icons/case";
    _categoryToIconMapping["PBR"] = ":Icons/lightbulb";
    _categoryToIconMapping["Emissive"] = ":Icons/lightbulb";
    _categoryToIconMapping["Textures"] = ":Icons/picture";
    _categoryToIconMapping["Translucency"] = ":Icons/picture";
    _categoryToIconMapping["UV"] = ":Icons/picture";
    _categoryToIconMapping["Transparency"] = ":Icons/picture";
    _categoryToIconMapping["Material"] = ":Icons/brush";
    _categoryToIconMapping["Camera"] = ":Icons/film";
    _categoryToIconMapping["CameraController"] = ":Icons/film";
    _categoryToIconMapping["CharacterController"] = ":Icons/user";
    _categoryToIconMapping["Swarm"] = ":Icons/users";
    _categoryToIconMapping["Player"] = ":Icons/user";
    _categoryToIconMapping["RigidBody"] = ":Icons/cube";
    _categoryToIconMapping["PostEffectVolume"] = ":Icons/picture";
    _categoryToIconMapping["Light"] = ":Icons/lightbulb";
    _categoryToIconMapping["IrradianceProbe"] = ":Icons/lightbulb";
  }

  {
    _componentToIconMapping["Camera"] = ":Icons/film";
    _componentToIconMapping["CameraController"] = ":Icons/film";
    _componentToIconMapping["CharacterController"] = ":Icons/user";
    _componentToIconMapping["Swarm"] = ":Icons/users";
    _componentToIconMapping["Player"] = ":Icons/user";
    _componentToIconMapping["Node"] = ":Icons/target";
    _componentToIconMapping["Mesh"] = ":Icons/user";
    _componentToIconMapping["Script"] = ":Icons/script";
    _componentToIconMapping["RigidBody"] = ":Icons/cube";
    _componentToIconMapping["PostEffectVolume"] = ":Icons/picture";
    _componentToIconMapping["Light"] = ":Icons/lightbulb";
    _componentToIconMapping["IrradianceProbe"] = ":Icons/lightbulb";
  }

  QObject::connect(_ui.actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
  QObject::connect(_ui.actionLoad_World, SIGNAL(triggered()), this,
                   SLOT(onLoadWorld()));
  QObject::connect(_ui.actionReload_Settings_And_Renderer_Config,
                   SIGNAL(triggered()), this,
                   SLOT(onReloadSettingsAndRendererConfig()));
  QObject::connect(_ui.actionSave_World, SIGNAL(triggered()), this,
                   SLOT(onSaveWorld()));
  QObject::connect(_ui.actionSave_Editor_Settings, SIGNAL(triggered()), this,
                   SLOT(onSaveEditorSettings()));
  QObject::connect(_ui.actionLoad_Editor_Settings, SIGNAL(triggered()), this,
                   SLOT(onLoadEditorSettings()));
  QObject::connect(_ui.actionFullscreen, SIGNAL(triggered()), this,
                   SLOT(onFullscreen()));

  QObject::connect(_ui.actionEditingModeDefault, SIGNAL(triggered()), this,
                   SLOT(onEditingModeDefault()));
  QObject::connect(_ui.actionEditingModeSelection, SIGNAL(triggered()), this,
                   SLOT(onEditingModeSelection()));
  QObject::connect(_ui.actionEditingModeTranslation, SIGNAL(triggered()), this,
                   SLOT(onEditingModeTranslation()));
  QObject::connect(_ui.actionEditingModeRotation, SIGNAL(triggered()), this,
                   SLOT(onEditingModeRotation()));
  QObject::connect(_ui.actionEditingModeScale, SIGNAL(triggered()), this,
                   SLOT(onEditingModeScale()));

  QObject::connect(_ui.actionMainGameState, SIGNAL(triggered()), this,
                   SLOT(onMainGameState()));
  QObject::connect(_ui.actionEditingGameState, SIGNAL(triggered()), this,
                   SLOT(onEditingGameState()));
  QObject::connect(_ui.actionBenchmarkGameState, SIGNAL(triggered()), this,
                   SLOT(onBenchmarkGameState()));

  QObject::connect(_ui.actionCreateCube, SIGNAL(triggered()), this,
                   SLOT(onCreateCube()));
  QObject::connect(_ui.actionCreateRigidBody, SIGNAL(triggered()), this,
                   SLOT(onCreateRigidBody()));
  QObject::connect(_ui.actionCreateRigidBody_Sphere, SIGNAL(triggered()), this,
                   SLOT(onCreateRigidBodySphere()));
  QObject::connect(_ui.actionCreateSphere, SIGNAL(triggered()), this,
                   SLOT(onCreateSphere()));
  QObject::connect(_ui.actionCreateLight, SIGNAL(triggered()), this,
                   SLOT(onCreateLight()));

  QObject::connect(_ui.actionShow_PhysX_Debug_Geometry, SIGNAL(triggered()),
                   this, SLOT(onDebugGeometryChanged()));
  QObject::connect(_ui.actionOpen_Microprofile, SIGNAL(triggered()), this,
                   SLOT(onOpenMicroprofile()));
  QObject::connect(_ui.actionShow_World_Bounding_Spheres, SIGNAL(triggered()),
                   this, SLOT(onDebugGeometryChanged()));
  QObject::connect(_ui.actionShow_Benchmark_Paths, SIGNAL(triggered()), this,
                   SLOT(onDebugGeometryChanged()));

  QObject::connect(_ui.actionShow_Create_Context_Menu, SIGNAL(triggered()),
                   this, SLOT(onShowCreateContextMenu()));
  QObject::connect(_ui.actionShow_Debug_Context_Menu, SIGNAL(triggered()), this,
                   SLOT(onShowDebugContextMenu()));
  QObject::connect(_ui.actionShow_Debug_Geometry_Context_Menu,
                   SIGNAL(triggered()), this,
                   SLOT(onShowDebugGeometryContextMenu()));

  QObject::connect(_ui.actionCompile_Shaders, SIGNAL(triggered()), this,
                   SLOT(onCompileShaders()));

  QObject::connect(_ui.actionRecompile_Shaders, SIGNAL(triggered()), this,
                   SLOT(onRecompileShaders()));

  _propertyView = new IntrinsicEdPropertyView();
  addDockWidget(Qt::RightDockWidgetArea, _propertyView);

  _nodeView = new IntrinsicEdNodeView();
  addDockWidget(Qt::LeftDockWidgetArea, _nodeView);

  _managerWindowGpuProgram = new IntrinsicEdManagerWindowGpuProgram(nullptr);
  _managerWindowScript = new IntrinsicEdManagerWindowScript(nullptr);
  _managerWindowPostEffect = new IntrinsicEdManagerWindowPostEffect(nullptr);
  _managerWindowAsset = new IntrinsicEdManagerWindowAsset(nullptr);
  _managerWindowMesh = new IntrinsicEdManagerWindowMesh(nullptr);
  _managerWindowMaterial = new IntrinsicEdManagerWindowMaterial(nullptr);
  _managerWindowImage = new IntrinsicEdManagerWindowImage(nullptr);

  QObject::connect(_ui.actionNode_View, SIGNAL(triggered()), _nodeView,
                   SLOT(show()));
  QObject::connect(_ui.actionProperty_View, SIGNAL(triggered()), _propertyView,
                   SLOT(show()));
  QObject::connect(_ui.actionGPU_Programs, SIGNAL(triggered()),
                   _managerWindowGpuProgram, SLOT(show()));
  QObject::connect(_ui.actionScripts, SIGNAL(triggered()), _managerWindowScript,
                   SLOT(show()));
  QObject::connect(_ui.actionAssets, SIGNAL(triggered()), _managerWindowAsset,
                   SLOT(show()));
  QObject::connect(_ui.actionMeshes, SIGNAL(triggered()), _managerWindowMesh,
                   SLOT(show()));
  QObject::connect(_ui.actionMaterials, SIGNAL(triggered()),
                   _managerWindowMaterial, SLOT(show()));
  QObject::connect(_ui.actionImages, SIGNAL(triggered()), _managerWindowImage,
                   SLOT(show()));
  QObject::connect(_ui.actionPostEffects, SIGNAL(triggered()),
                   _managerWindowPostEffect, SLOT(show()));

  // Editing toolbar
  {
    QLabel* label = new QLabel("Grid Size:");
    label->setMargin(3);
    label->setStyleSheet("font: 8pt");

    QDoubleSpinBox* gridSizeSpinBox = new QDoubleSpinBox();
    gridSizeSpinBox->setMinimum(0.01f);
    gridSizeSpinBox->setSingleStep(0.5f);
    gridSizeSpinBox->setToolTip("Grid Size");

    gridSizeSpinBox->setValue(GameStates::Editing::_gridSize);

    _ui.gridToolBar->addWidget(label);
    _ui.gridToolBar->addWidget(gridSizeSpinBox);

    QObject::connect(gridSizeSpinBox, SIGNAL(valueChanged(double)), this,
                     SLOT(onGridSizeChanged(double)));
  }

  {
    QLabel* label = new QLabel("Gizmo Size:");
    label->setMargin(3);
    label->setStyleSheet("font: 8pt");

    QDoubleSpinBox* gizmoSizeSpinBox = new QDoubleSpinBox();
    gizmoSizeSpinBox->setMinimum(0.05f);
    gizmoSizeSpinBox->setSingleStep(0.05f);
    gizmoSizeSpinBox->setToolTip("Gizmo Size");

    gizmoSizeSpinBox->setValue(GameStates::Editing::_gizmoSize);

    _ui.gridToolBar->addWidget(label);
    _ui.gridToolBar->addWidget(gizmoSizeSpinBox);

    QObject::connect(gizmoSizeSpinBox, SIGNAL(valueChanged(double)), this,
                     SLOT(onGizmoSizeChanged(double)));
  }

  {
    QLabel* label = new QLabel("Camera Speed:");
    label->setMargin(3);
    label->setStyleSheet("font: 8pt");

    QDoubleSpinBox* cameraSpeedSpinBox = new QDoubleSpinBox();
    cameraSpeedSpinBox->setMinimum(0.05f);
    cameraSpeedSpinBox->setMaximum(1000.0f);
    cameraSpeedSpinBox->setSingleStep(0.05f);
    cameraSpeedSpinBox->setToolTip("Camera Speed");

    cameraSpeedSpinBox->setValue(GameStates::Editing::_cameraSpeed);

    _ui.gridToolBar->addWidget(label);
    _ui.gridToolBar->addWidget(cameraSpeedSpinBox);

    QObject::connect(cameraSpeedSpinBox, SIGNAL(valueChanged(double)), this,
                     SLOT(onCameraSpeedChanged(double)));
  }

  _ui.gridToolBar->addSeparator();

  {

    QLabel* label = new QLabel("Time Mod.:");
    label->setMargin(3);
    label->setStyleSheet("font: 8pt");

    QDoubleSpinBox* timeModSpinBox = new QDoubleSpinBox();
    timeModSpinBox->setMinimum(0.0f);
    timeModSpinBox->setMaximum(10.0f);
    timeModSpinBox->setSingleStep(0.05f);
    timeModSpinBox->setToolTip("Time Modulator");

    timeModSpinBox->setValue(TaskManager::_timeModulator);

    _ui.gridToolBar->addWidget(label);
    _ui.gridToolBar->addWidget(timeModSpinBox);

    QObject::connect(timeModSpinBox, SIGNAL(valueChanged(double)), this,
                     SLOT(onTimeModChanged(double)));
  }

  // Context menus
  {
    _createContextMenu.addAction(_ui.actionCreateCube);
    _createContextMenu.addAction(_ui.actionCreateSphere);
    _createContextMenu.addSeparator();
    _createContextMenu.addAction(_ui.actionCreateLight);
    _createContextMenu.addSeparator();
    _createContextMenu.addAction(_ui.actionCreateRigidBody);
    _createContextMenu.addAction(_ui.actionCreateRigidBody_Sphere);

    _debugGeometryContextMenu.addAction(_ui.actionShow_World_Bounding_Spheres);
    _debugGeometryContextMenu.addAction(_ui.actionShow_Benchmark_Paths);
    _debugGeometryContextMenu.addSeparator();
    _debugGeometryContextMenu.addAction(_ui.actionShow_PhysX_Debug_Geometry);
  }

  onLoadEditorSettings();

  _settingsChangeWatch = new QFileSystemWatcher(this);
  QObject::connect(_settingsChangeWatch, SIGNAL(fileChanged(const QString&)),
                   this, SLOT(onSettingsFileChanged(const QString&)));
  updateSettingsChangeWatch();
  _settingsUpdatePending = false;

  _mainWindow = this;
}

IntrinsicEd::~IntrinsicEd() {}

void IntrinsicEd::onSaveEditorSettings()
{
  QSettings settings;
  settings.setValue("mainWindowGeometry", saveGeometry());
  settings.setValue("mainWindowState", saveState());
}

void IntrinsicEd::onLoadEditorSettings()
{
  QSettings settings;
  restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
  restoreState(settings.value("mainWindowState").toByteArray());
}

void IntrinsicEd::onExit() { exit(0); }

void IntrinsicEd::onLoadWorld()
{
  const QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open World"), QString("worlds"),
                                   tr("World File (*.world.json)"));

  if (fileName.size() > 0u)
  {
    World::load(fileName.toStdString().c_str());
    GameStates::Manager::activateGameState(GameStates::GameState::kEditing);
    _nodeView->populateNodeTree();
  }
}

void IntrinsicEd::onReloadSettingsAndRendererConfig()
{
  Settings::Manager::loadSettings();
  Vulkan::RenderSystem::onViewportChanged();
}

void IntrinsicEd::onSaveWorld()
{
  const QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save World"), QString("worlds"),
                                   tr("World File (*.world.json)"));

  if (fileName.size() > 0u)
  {
    World::save(fileName.toStdString().c_str());
  }
}

void IntrinsicEd::onFullscreen()
{
  _ui.viewPort->setParent(nullptr);
  _ui.viewPort->showFullScreen();
}

void IntrinsicEd::onEndFullscreen()
{
  _ui.centralWidget->layout()->addWidget(_ui.viewPort);
}

void IntrinsicEd::onEditingModeDefault()
{
  _ui.actionEditingModeDefault->setChecked(true);
  _ui.actionEditingModeSelection->setChecked(false);
  _ui.actionEditingModeRotation->setChecked(false);
  _ui.actionEditingModeScale->setChecked(false);
  _ui.actionEditingModeTranslation->setChecked(false);

  GameStates::Editing::_editingMode = GameStates::EditingMode::kDefault;
}

void IntrinsicEd::onEditingModeSelection()
{
  _ui.actionEditingModeDefault->setChecked(false);
  _ui.actionEditingModeSelection->setChecked(true);
  _ui.actionEditingModeRotation->setChecked(false);
  _ui.actionEditingModeScale->setChecked(false);
  _ui.actionEditingModeTranslation->setChecked(false);

  GameStates::Editing::_editingMode = GameStates::EditingMode::kSelection;
}

void IntrinsicEd::onEditingModeTranslation()
{
  _ui.actionEditingModeDefault->setChecked(false);
  _ui.actionEditingModeTranslation->setChecked(true);
  _ui.actionEditingModeRotation->setChecked(false);
  _ui.actionEditingModeScale->setChecked(false);
  _ui.actionEditingModeSelection->setChecked(false);

  GameStates::Editing::_editingMode = GameStates::EditingMode::kTranslation;
}

void IntrinsicEd::onEditingModeRotation()
{
  _ui.actionEditingModeDefault->setChecked(false);
  _ui.actionEditingModeRotation->setChecked(true);
  _ui.actionEditingModeTranslation->setChecked(false);
  _ui.actionEditingModeScale->setChecked(false);
  _ui.actionEditingModeSelection->setChecked(false);

  GameStates::Editing::_editingMode = GameStates::EditingMode::kRotation;
}

void IntrinsicEd::onEditingModeScale()
{
  _ui.actionEditingModeDefault->setChecked(false);
  _ui.actionEditingModeScale->setChecked(true);
  _ui.actionEditingModeTranslation->setChecked(false);
  _ui.actionEditingModeRotation->setChecked(false);
  _ui.actionEditingModeSelection->setChecked(false);

  GameStates::Editing::_editingMode = GameStates::EditingMode::kScale;
}

void IntrinsicEd::onMainGameState()
{
  _ui.actionEditingGameState->setChecked(false);
  _ui.actionMainGameState->setChecked(true);
  _ui.actionBenchmarkGameState->setChecked(false);

  GameStates::Manager::activateGameState(GameStates::GameState::kMain);
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void IntrinsicEd::onEditingGameState()
{
  _ui.actionEditingGameState->setChecked(true);
  _ui.actionMainGameState->setChecked(false);
  _ui.actionBenchmarkGameState->setChecked(false);

  GameStates::Manager::activateGameState(GameStates::GameState::kEditing);

  SDL_SetRelativeMouseMode(SDL_FALSE);
}

void IntrinsicEd::onBenchmarkGameState()
{
  _ui.actionBenchmarkGameState->setChecked(true);
  _ui.actionEditingGameState->setChecked(false);
  _ui.actionMainGameState->setChecked(false);

  GameStates::Manager::activateGameState(GameStates::GameState::kBenchmark);

  SDL_SetRelativeMouseMode(SDL_FALSE);
}

void IntrinsicEd::onCreateCube()
{
  Components::MeshRefArray meshComponentsToCreate;

  {
    Entity::EntityRef entityRef = Entity::EntityManager::createEntity(_N(Cube));
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);
    Components::MeshRef meshRef =
        Components::MeshManager::createMesh(entityRef);
    meshComponentsToCreate.push_back(meshRef);
    Components::MeshManager::resetToDefault(meshRef);

    Components::MeshManager::_descMeshName(meshRef) = _N(cube);

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Components::NodeManager::_position(nodeRef) =
        Components::NodeManager::_worldPosition(cameraNode) +
        Components::CameraManager::_forward(activeCamera) * 10.0f;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(meshComponentsToCreate);
}

void IntrinsicEd::onCreateRigidBody()
{
  Components::MeshRefArray meshComponentsToCreate;
  Components::RigidBodyRefArray rigidBodyComponentsToCreate;

  {
    Entity::EntityRef entityRef =
        Entity::EntityManager::createEntity(_N(RigidBody));
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);
    Components::MeshRef meshRef =
        Components::MeshManager::createMesh(entityRef);
    meshComponentsToCreate.push_back(meshRef);
    Components::MeshManager::resetToDefault(meshRef);
    Components::MeshManager::_descMeshName(meshRef) = _N(cube);

    Components::RigidBodyRef rigidBodyRef =
        Components::RigidBodyManager::createRigidBody(entityRef);
    rigidBodyComponentsToCreate.push_back(rigidBodyRef);
    Components::RigidBodyManager::resetToDefault(rigidBodyRef);
    Components::RigidBodyManager::_descRigidBodyType(rigidBodyRef) =
        Components::RigidBodyType::kBoxDynamic;

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Components::NodeManager::_position(nodeRef) =
        Components::NodeManager::_worldPosition(cameraNode) +
        Components::CameraManager::_forward(activeCamera) * 10.0f;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  Components::MeshManager::createResources(meshComponentsToCreate);
  Components::RigidBodyManager::createResources(rigidBodyComponentsToCreate);
}

void IntrinsicEd::onCreateRigidBodySphere()
{
  Components::MeshRefArray meshComponentsToCreate;
  Components::RigidBodyRefArray rigidBodyComponentsToCreate;

  {
    Entity::EntityRef entityRef =
        Entity::EntityManager::createEntity(_N(RigidBody));
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);
    Components::MeshRef meshRef =
        Components::MeshManager::createMesh(entityRef);
    meshComponentsToCreate.push_back(meshRef);
    Components::MeshManager::resetToDefault(meshRef);
    Components::MeshManager::_descMeshName(meshRef) = _N(sphere);

    Components::RigidBodyRef rigidBodyRef =
        Components::RigidBodyManager::createRigidBody(entityRef);
    rigidBodyComponentsToCreate.push_back(rigidBodyRef);
    Components::RigidBodyManager::resetToDefault(rigidBodyRef);
    Components::RigidBodyManager::_descRigidBodyType(rigidBodyRef) =
        Components::RigidBodyType::kSphereDynamic;

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Components::NodeManager::_position(nodeRef) =
        Components::NodeManager::_worldPosition(cameraNode) +
        Components::CameraManager::_forward(activeCamera) * 10.0f;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();

  Components::MeshManager::createResources(meshComponentsToCreate);
  Components::RigidBodyManager::createResources(rigidBodyComponentsToCreate);
}

void IntrinsicEd::onCreateSphere()
{
  Components::MeshRefArray meshComponentsToCreate;

  {
    Entity::EntityRef entityRef =
        Entity::EntityManager::createEntity(_N(Sphere));
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);
    Components::MeshRef meshRef =
        Components::MeshManager::createMesh(entityRef);
    meshComponentsToCreate.push_back(meshRef);
    Components::MeshManager::resetToDefault(meshRef);

    Components::MeshManager::_descMeshName(meshRef) = _N(sphere);

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Components::NodeManager::_position(nodeRef) =
        Components::NodeManager::_worldPosition(cameraNode) +
        Components::CameraManager::_forward(activeCamera) * 10.0f;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(meshComponentsToCreate);
}

void IntrinsicEd::onCreateLight()
{
  Components::LightRefArray lightComponentsToCreate;

  {
    Entity::EntityRef entityRef =
        Entity::EntityManager::createEntity(_N(Light));
    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);
    Components::LightRef lightRef =
        Components::LightManager::createLight(entityRef);
    lightComponentsToCreate.push_back(lightRef);
    Components::LightManager::resetToDefault(lightRef);

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Components::NodeManager::_position(nodeRef) =
        Components::NodeManager::_worldPosition(cameraNode) +
        Components::CameraManager::_forward(activeCamera) * 10.0f;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void IntrinsicEd::onGridSizeChanged(double p_Value)
{
  GameStates::Editing::_gridSize = (float)p_Value;
}

void IntrinsicEd::onGizmoSizeChanged(double p_Value)
{
  GameStates::Editing::_gizmoSize = (float)p_Value;
}

void IntrinsicEd::onCameraSpeedChanged(double p_Value)
{
  GameStates::Editing::_cameraSpeed = (float)p_Value;
}

void IntrinsicEd::onTimeModChanged(double p_Value)
{
  TaskManager::_timeModulator = p_Value;
}

void IntrinsicEd::onOpenMicroprofile()
{
  QDesktopServices::openUrl(QUrl("http://127.0.0.1:1338"));
}

void updateStatusBar(QStatusBar* p_StatusBar)
{
  static float timeSinceLastStatusBarUpdate = 0.0f;

  if (timeSinceLastStatusBarUpdate > 0.1f)
  {
    QColor color;

    static float smoothedDeltaT = 0.0f;
    smoothedDeltaT = (smoothedDeltaT + TaskManager::_lastDeltaT) / 2.0f;

    const float fps = std::round(1.0f / smoothedDeltaT);

    if (fps >= 60.0f)
      color.setRgb(0, 255, 0);
    else if (fps >= 30.0f)
      color.setRgb(255, 255, 0);
    else
      color.setRgb(255, 0, 0);

    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    p_StatusBar->setPalette(palette);

    _INTR_STRING statucBarText = StringUtil::toString<float>(fps) + " FPS / " +
                                 StringUtil::toString<float>(smoothedDeltaT) +
                                 " s";
    p_StatusBar->showMessage(statucBarText.c_str());
    timeSinceLastStatusBarUpdate = 0.0f;
  }
  else
  {
    timeSinceLastStatusBarUpdate += TaskManager::_lastDeltaT;
  }
}

void IntrinsicEd::closeEvent(QCloseEvent*)
{
  _viewport = nullptr;

  Application::shutdown();
}

void IntrinsicEd::onShowDebugContextMenu()
{
  _debugContextMenu.popup(QCursor::pos());
}

void IntrinsicEd::onShowDebugGeometryContextMenu()
{
  _debugGeometryContextMenu.popup(QCursor::pos());
}

void IntrinsicEd::onShowCreateContextMenu()
{
  _createContextMenu.popup(QCursor::pos());
}

void IntrinsicEd::onDebugGeometryChanged()
{
  if (_ui.actionShow_PhysX_Debug_Geometry->isChecked())
  {
    Physics::System::setDebugRenderingFlags(
        Physics::DebugRenderingFlags::kEnabled);
  }
  else
  {
    Physics::System::setDebugRenderingFlags(0u);
  }

  if (_ui.actionShow_World_Bounding_Spheres->isChecked())
  {
    Intrinsic::Renderer::Vulkan::RenderPass::Debug::_activeDebugStageFlags |=
        Intrinsic::Renderer::Vulkan::RenderPass::DebugStageFlags::
            kWorldBoundingSpheres;
  }
  else
  {
    Intrinsic::Renderer::Vulkan::RenderPass::Debug::_activeDebugStageFlags &=
        ~Intrinsic::Renderer::Vulkan::RenderPass::DebugStageFlags::
            kWorldBoundingSpheres;
  }

  if (_ui.actionShow_Benchmark_Paths->isChecked())
  {
    Intrinsic::Renderer::Vulkan::RenderPass::Debug::_activeDebugStageFlags |=
        Intrinsic::Renderer::Vulkan::RenderPass::DebugStageFlags::
            kBenchmarkPaths;
  }
  else
  {
    Intrinsic::Renderer::Vulkan::RenderPass::Debug::_activeDebugStageFlags &=
        ~Intrinsic::Renderer::Vulkan::RenderPass::DebugStageFlags::
            kBenchmarkPaths;
  }
}

void IntrinsicEd::onCompileShaders()
{
  Intrinsic::Renderer::Vulkan::Resources::GpuProgramManager::
      compileAllShaders();
}

void IntrinsicEd::onRecompileShaders()
{
  Intrinsic::Renderer::Vulkan::Resources::GpuProgramManager::compileAllShaders(
      true);
}

void IntrinsicEd::onSettingsFileChanged(const QString&)
{
  _settingsUpdatePending = true;
}

void IntrinsicEd::updateSettingsChangeWatch()
{
  QStringList files = _settingsChangeWatch->files();
  if (!files.empty())
    _settingsChangeWatch->removePaths(files);

  _settingsChangeWatch->addPath("settings.json");
  _settingsChangeWatch->addPath(QString("config/") +
                                Settings::Manager::_rendererConfig.c_str());
  _settingsChangeWatch->addPath(QString("config/") +
                                Settings::Manager::_materialPassConfig.c_str());
}

int IntrinsicEd::enterMainLoop()
{
  while (Application::_running)
  {
    qApp->processEvents();

    TaskManager::executeTasks();
    updateStatusBar(_ui.statusBar);
    _propertyView->updatePropertyView();

    if (_settingsUpdatePending)
    {
      Settings::Manager::loadSettings();
      updateSettingsChangeWatch();
      Vulkan::RenderSystem::onViewportChanged();
      _settingsUpdatePending = false;
    }
  }

  return 0;
}
