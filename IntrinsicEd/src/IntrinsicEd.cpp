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

using namespace RVResources;

IntrinsicEdNodeView* IntrinsicEd::_nodeView = nullptr;
IntrinsicEdPropertyView* IntrinsicEd::_propertyView = nullptr;
IntrinsicEdPrefabsBrowser* IntrinsicEd::_prefabsBrowser = nullptr;
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

QDockWidget* _editingView = nullptr;

_INTR_ARRAY(QIcon) IntrinsicEd::_icons;
_INTR_ARRAY(QPixmap) IntrinsicEd::_pixmaps;
_INTR_HASH_MAP(_INTR_STRING, uint32_t) IntrinsicEd::_stringToIconMapping;
_INTR_HASH_MAP(_INTR_STRING, uint32_t) IntrinsicEd::_stringToPixmapMapping;

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
    AssetManagement::Resources::AssetManager::init();
    AssetManagement::Resources::AssetManager::loadFromMultipleFiles(
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
    _INTR_HASH_MAP(_INTR_STRING, _INTR_STRING) stringToIconPathMapping;

    stringToIconPathMapping["Camera"] =
        ":Icons/icons/essential/photo-camera-1.png";
    stringToIconPathMapping["CameraController"] =
        ":Icons/icons/essential/controls-1.png";
    stringToIconPathMapping["CharacterController"] =
        ":Icons/icons/cad/cylinder.png";
    stringToIconPathMapping["Swarm"] = ":Icons/icons/birds/bird-2.png";
    stringToIconPathMapping["Player"] =
        ":Icons/icons/tech/game-controller-5.png";
    stringToIconPathMapping["RootNode"] = ":/Icons/icons/cad/circle-1.png";
    stringToIconPathMapping["Node"] = ":/Icons/icons/cad/cube-1.png";
    stringToIconPathMapping["Mesh"] = ":/Icons/icons/cad/cone.png";
    stringToIconPathMapping["Script"] =
        ":/Icons/icons/business/051-certificate.png";
    stringToIconPathMapping["RigidBody"] = ":/Icons/icons/cad/corner.png";
    stringToIconPathMapping["PostEffectVolume"] =
        ":/Icons/icons/weather/rainbow-2.png";
    stringToIconPathMapping["PostEffect"] =
        ":/Icons/icons/weather/rainbow-2.png";
    stringToIconPathMapping["Light"] = ":Icons/icons/various/idea.png";
    stringToIconPathMapping["IrradianceProbe"] =
        ":/Icons/icons/weather/clouds-and-sun.png";
    stringToIconPathMapping["NodeLocalTransform"] =
        ":/Icons/icons/cad/rotate-1.png";
    stringToIconPathMapping["NodeWorldTransform"] =
        ":/Icons/icons/cad/rotate-1.png";
    stringToIconPathMapping["Entity"] = ":/Icons/icons/essential/archive.png";
    stringToIconPathMapping["Mesh"] = ":/Icons/icons/cad/cone.png";
    stringToIconPathMapping["GpuProgram"] =
        ":/Icons/icons/business/051-coding-1.png";
    stringToIconPathMapping["Resource"] = ":/Icons/icons/essential/archive.png";
    stringToIconPathMapping["Asset"] = ":/Icons/icons/essential/archive-3.png";
    stringToIconPathMapping["PBR"] = ":Icons/icons/various/idea.png";
    stringToIconPathMapping["Emissive"] = ":Icons/icons/various/idea.png";
    stringToIconPathMapping["Textures"] = ":/Icons/icons/essential/picture.png";
    stringToIconPathMapping["Texture"] = ":/Icons/icons/essential/picture.png";
    stringToIconPathMapping["Image"] = ":/Icons/icons/essential/picture.png";
    stringToIconPathMapping["Translucency"] = ":Icons/icons/various/idea.png";
    stringToIconPathMapping["UV"] = ":/Icons/icons/essential/picture.png";
    stringToIconPathMapping["Transparency"] = ":/Icons/icons/geometry/cube.png";
    stringToIconPathMapping["Material"] = ":/Icons/icons/essential/layers.png";
    stringToIconPathMapping["Lighting"] = ":Icons/icons/various/idea.png";
    stringToIconPathMapping["VolumetricLighting"] =
        ":/Icons/icons/weather/clouds.png";
    stringToIconPathMapping["Water"] = ":/Icons/icons/weather/raindrops.png";
    stringToIconPathMapping["DepthOfField"] =
        ":Icons/icons/essential/photo-camera-1.png";
    stringToIconPathMapping["Decal"] = ":Icons/icons/essential/picture-2.png";
    stringToIconPathMapping["Folder"] = ":Icons/icons/essential/folder-8.png";
    stringToIconPathMapping["File"] = ":Icons/icons/essential/file.png";
    stringToIconPathMapping["Prefab"] = ":/Icons/icons/essential/compose.png";

    for (auto it = stringToIconPathMapping.begin();
         it != stringToIconPathMapping.end(); ++it)
    {
      _icons.push_back(QIcon(it->second.c_str()));
      _stringToIconMapping[it->first] = (uint32_t)_icons.size() - 1u;
    }
    for (auto it = stringToIconPathMapping.begin();
         it != stringToIconPathMapping.end(); ++it)
    {
      const QPixmap pixmap = QPixmap(it->second.c_str());
      _pixmaps.push_back(pixmap.scaledToWidth(20u, Qt::SmoothTransformation));
      _stringToPixmapMapping[it->first] = (uint32_t)_pixmaps.size() - 1u;
    }
  }

  QObject::connect(_ui.actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
  QObject::connect(_ui.actionLoad_World, SIGNAL(triggered()), this,
                   SLOT(onLoadWorld()));
  QObject::connect(_ui.actionReload_World, SIGNAL(triggered()), this,
                   SLOT(onReloadWorld()));
  QObject::connect(_ui.actionReload_Settings_And_Renderer_Config,
                   SIGNAL(triggered()), this,
                   SLOT(onReloadSettingsAndRendererConfig()));
  QObject::connect(_ui.actionSave_World, SIGNAL(triggered()), this,
                   SLOT(onSaveWorld()));
  QObject::connect(_ui.actionSave_World_As, SIGNAL(triggered()), this,
                   SLOT(onSaveWorldAs()));
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
  QObject::connect(_ui.actionCreateIrradProbe, SIGNAL(triggered()), this,
                   SLOT(onCreateIrradProbe()));
  QObject::connect(_ui.actionCreateDecal, SIGNAL(triggered()), this,
                   SLOT(onCreateDecal()));

  QObject::connect(_ui.actionShow_PhysX_Debug_Geometry, SIGNAL(triggered()),
                   this, SLOT(onDebugGeometryChanged()));
  QObject::connect(_ui.actionOpen_Microprofile, SIGNAL(triggered()), this,
                   SLOT(onOpenMicroprofile()));
  QObject::connect(_ui.actionShow_World_Bounding_Spheres, SIGNAL(triggered()),
                   this, SLOT(onDebugGeometryChanged()));
  QObject::connect(_ui.actionShow_Benchmark_Paths, SIGNAL(triggered()), this,
                   SLOT(onDebugGeometryChanged()));
  QObject::connect(_ui.actionWireframe_Rendering, SIGNAL(triggered()), this,
                   SLOT(onDebugGeometryChanged()));

  QObject::connect(_ui.actionShow_Debug_Geometry_Context_Menu,
                   SIGNAL(triggered()), this,
                   SLOT(onShowDebugGeometryContextMenu()));

  QObject::connect(_ui.actionCompile_Shaders, SIGNAL(triggered()), this,
                   SLOT(onCompileShaders()));

  QObject::connect(_ui.actionRecompile_Shaders, SIGNAL(triggered()), this,
                   SLOT(onRecompileShaders()));

  _editingView = new QDockWidget();
  _editingView->setObjectName("editingView");
  addDockWidget(Qt::RightDockWidgetArea, _editingView);

  _propertyView = new IntrinsicEdPropertyView();
  tabifyDockWidget(_editingView, _propertyView);

  _prefabsBrowser = new IntrinsicEdPrefabsBrowser();
  tabifyDockWidget(_propertyView, _prefabsBrowser);

  _nodeView = new IntrinsicEdNodeView();
  tabifyDockWidget(_prefabsBrowser, _nodeView);

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

  // Setup editing view
  {
    QGridLayout* editingViewLayout = new QGridLayout();
    QWidget* editingViewContainer = new QWidget();
    _editingView->setWidget(editingViewContainer);

    editingViewContainer->setLayout(editingViewLayout);
    _editingView->setWindowTitle("Editing View");

    {
      QLabel* label = new QLabel("Grid Size:");
      label->setMargin(3);

      QDoubleSpinBox* gridSizeSpinBox = new QDoubleSpinBox();
      gridSizeSpinBox->setMinimum(0.01f);
      gridSizeSpinBox->setSingleStep(0.5f);
      gridSizeSpinBox->setToolTip("Grid Size");

      gridSizeSpinBox->setValue(GameStates::Editing::_gridSize);

      editingViewLayout->addWidget(label, 0, 0);
      editingViewLayout->addWidget(gridSizeSpinBox, 0, 1);

      QObject::connect(gridSizeSpinBox, SIGNAL(valueChanged(double)), this,
                       SLOT(onGridSizeChanged(double)));
    }

    {
      QLabel* label = new QLabel("Gizmo Size:");
      label->setMargin(3);

      QDoubleSpinBox* gizmoSizeSpinBox = new QDoubleSpinBox();
      gizmoSizeSpinBox->setMinimum(0.05f);
      gizmoSizeSpinBox->setSingleStep(0.05f);
      gizmoSizeSpinBox->setToolTip("Gizmo Size");

      gizmoSizeSpinBox->setValue(GameStates::Editing::_gizmoSize);

      editingViewLayout->addWidget(label, 1, 0);
      editingViewLayout->addWidget(gizmoSizeSpinBox, 1, 1);

      QObject::connect(gizmoSizeSpinBox, SIGNAL(valueChanged(double)), this,
                       SLOT(onGizmoSizeChanged(double)));
    }

    {
      QLabel* label = new QLabel("Camera Speed:");
      label->setMargin(3);

      QDoubleSpinBox* cameraSpeedSpinBox = new QDoubleSpinBox();
      cameraSpeedSpinBox->setMinimum(0.05f);
      cameraSpeedSpinBox->setMaximum(1000.0f);
      cameraSpeedSpinBox->setSingleStep(0.05f);
      cameraSpeedSpinBox->setToolTip("Camera Speed");

      cameraSpeedSpinBox->setValue(GameStates::Editing::_cameraSpeed);

      editingViewLayout->addWidget(label, 2, 0);
      editingViewLayout->addWidget(cameraSpeedSpinBox, 2, 1);

      QObject::connect(cameraSpeedSpinBox, SIGNAL(valueChanged(double)), this,
                       SLOT(onCameraSpeedChanged(double)));
    }

    {
      QLabel* label = new QLabel("Day/Night:");
      label->setMargin(3);

      _dayNightSlider = new QSlider(Qt::Horizontal);
      _dayNightSlider->setMinimum(0);
      _dayNightSlider->setMaximum(100);
      _dayNightSlider->setSingleStep(1);
      _dayNightSlider->setToolTip("Day/Night");
      _dayNightSlider->setValue(World::_currentTime);

      editingViewLayout->addWidget(label, 3, 0);
      editingViewLayout->addWidget(_dayNightSlider, 3, 1);

      QObject::connect(_dayNightSlider, SIGNAL(valueChanged(int)), this,
                       SLOT(onDayNightSliderChanged(int)));
    }

    {
      QLabel* label = new QLabel("Time Mod.:");
      label->setMargin(3);

      QDoubleSpinBox* timeModSpinBox = new QDoubleSpinBox();
      timeModSpinBox->setMinimum(0.0f);
      timeModSpinBox->setMaximum(999.0f);
      timeModSpinBox->setSingleStep(0.1f);
      timeModSpinBox->setToolTip("Time Modulator");

      timeModSpinBox->setValue(TaskManager::_timeModulator);

      editingViewLayout->addWidget(label, 4, 0);
      editingViewLayout->addWidget(timeModSpinBox, 4, 1);

      QObject::connect(timeModSpinBox, SIGNAL(valueChanged(double)), this,
                       SLOT(onTimeModChanged(double)));
    }

    editingViewLayout->setRowStretch(5, 1);
  }

  // Context menus
  {
    _debugGeometryContextMenu.addAction(_ui.actionWireframe_Rendering);
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
  QSettings* settings = new QSettings("IntrinsicEd.ini", QSettings::IniFormat);
  settings->setValue("mainWindowGeometry", saveGeometry());
  settings->setValue("mainWindowState", saveState());
}

void IntrinsicEd::onLoadEditorSettings()
{
  QSettings* settings = new QSettings("IntrinsicEd.ini", QSettings::IniFormat);
  restoreGeometry(settings->value("mainWindowGeometry").toByteArray());
  restoreState(settings->value("mainWindowState").toByteArray());
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

void IntrinsicEd::onReloadWorld()
{
  World::load(World::_filePath);
  GameStates::Manager::activateGameState(GameStates::GameState::kEditing);
  _nodeView->populateNodeTree();
}

void IntrinsicEd::onReloadSettingsAndRendererConfig()
{
  Settings::Manager::loadSettings();
  RV::RenderSystem::onViewportChanged();
}

void IntrinsicEd::onSaveWorld() { World::save(World::_filePath); }

void IntrinsicEd::onSaveWorldAs()
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

namespace
{
_INTR_INLINE Entity::EntityRef
spawnDefaultEntity(const Name& p_Name,
                   const glm::quat& p_InitialOrientation = glm::quat())
{
  Entity::EntityRef entityRef = Entity::EntityManager::createEntity(p_Name);
  {

    Components::NodeRef nodeRef =
        Components::NodeManager::createNode(entityRef);
    Components::NodeManager::attachChild(World::getRootNode(), nodeRef);

    Components::CameraRef activeCamera = World::getActiveCamera();
    Components::NodeRef cameraNode =
        Components::NodeManager::getComponentForEntity(
            Components::CameraManager::_entity(activeCamera));

    Math::Ray worldRay = {Components::NodeManager::_worldPosition(cameraNode),
                          Components::CameraManager::_forward(activeCamera)};

    // Spawn the object close to the ground
    physx::PxRaycastHit hit;
    if (PhysxHelper::raycast(worldRay, hit, 50.0f, physx::PxQueryFlag::eSTATIC))
    {
      Components::NodeManager::_position(nodeRef) =
          Components::NodeManager::_worldPosition(cameraNode) +
          Components::CameraManager::_forward(activeCamera) * hit.distance *
              0.9f;
    }
    else
    {
      Components::NodeManager::_position(nodeRef) =
          Components::NodeManager::_worldPosition(cameraNode) +
          Components::CameraManager::_forward(activeCamera) * 10.0f;
    }

    Components::NodeManager::_orientation(nodeRef) = p_InitialOrientation;
    GameStates::Editing::_currentlySelectedEntity = entityRef;
  }

  return entityRef;
}

_INTR_INLINE Dod::Ref addComponentToEntity(Entity::EntityRef p_EntityRef,
                                           const Name& p_ComponentName)
{
  Dod::Components::ComponentManagerEntry& entry =
      Application::_componentManagerMapping[p_ComponentName];

  _INTR_ASSERT(entry.createFunction);
  Dod::Ref compRef = entry.createFunction(p_EntityRef);

  if (entry.resetToDefaultFunction)
  {
    entry.resetToDefaultFunction(compRef);
  }

  return compRef;
}
}

void IntrinsicEd::onCreateCube()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(Cube));
  Dod::Ref compRef = addComponentToEntity(entityRef, _N(Mesh));
  Components::MeshManager::_descMeshName(compRef) = _N(cube);

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(compRef);
}

void IntrinsicEd::onCreateSphere()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(Sphere));
  Dod::Ref compRef = addComponentToEntity(entityRef, _N(Mesh));

  Components::MeshManager::_descMeshName(compRef) = _N(sphere);

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(compRef);
}

void IntrinsicEd::onCreateRigidBody()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(RigidCube));
  Dod::Ref meshCompRef = addComponentToEntity(entityRef, _N(Mesh));
  Dod::Ref rigidBodyRef = addComponentToEntity(entityRef, _N(RigidBody));

  Components::MeshManager::_descMeshName(meshCompRef) = _N(cube);
  Components::RigidBodyManager::_descRigidBodyType(rigidBodyRef) =
      Components::RigidBodyType::kBoxDynamic;

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(meshCompRef);
  Components::RigidBodyManager::createResources(rigidBodyRef);
}

void IntrinsicEd::onCreateRigidBodySphere()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(RigidSphere));
  Dod::Ref meshCompRef = addComponentToEntity(entityRef, _N(Mesh));
  Dod::Ref rigidBodyRef = addComponentToEntity(entityRef, _N(RigidBody));

  Components::MeshManager::_descMeshName(meshCompRef) = _N(sphere);
  Components::RigidBodyManager::_descRigidBodyType(rigidBodyRef) =
      Components::RigidBodyType::kSphereDynamic;

  Components::NodeManager::rebuildTreeAndUpdateTransforms();
  Components::MeshManager::createResources(meshCompRef);
  Components::RigidBodyManager::createResources(rigidBodyRef);
}

void IntrinsicEd::onCreateLight()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(Light));
  Dod::Ref compRef = addComponentToEntity(entityRef, _N(Light));
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void IntrinsicEd::onCreateIrradProbe()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(_N(IrradianceProbe));
  Dod::Ref compRef = addComponentToEntity(entityRef, _N(IrradianceProbe));
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void IntrinsicEd::onCreateDecal()
{
  Entity::EntityRef entityRef = spawnDefaultEntity(
      _N(Decal), glm::quat(glm::vec3(-glm::half_pi<float>(), 0.0f, 0.0f)));
  Dod::Ref compRef = addComponentToEntity(entityRef, _N(Decal));
  Components::NodeManager::rebuildTreeAndUpdateTransforms();
}

void IntrinsicEd::onGridSizeChanged(double p_Value)
{
  GameStates::Editing::_gridSize = (float)p_Value;
}

void IntrinsicEd::onDayNightSliderChanged(int p_Value)
{
  World::_currentTime = p_Value * 0.01f;
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
    const float ms = std::round(smoothedDeltaT * 1000.0f);

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
                                 StringUtil::toString<float>(ms) + " ms";
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

void IntrinsicEd::onShowDebugGeometryContextMenu()
{
  _debugGeometryContextMenu.popup(QCursor::pos());
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
    RV::RenderPass::Debug::_activeDebugStageFlags |=
        RV::RenderPass::DebugStageFlags::kWorldBoundingSpheres;
  }
  else
  {
    RV::RenderPass::Debug::_activeDebugStageFlags &=
        ~RV::RenderPass::DebugStageFlags::kWorldBoundingSpheres;
  }

  if (_ui.actionShow_Benchmark_Paths->isChecked())
  {
    RV::RenderPass::Debug::_activeDebugStageFlags |=
        RV::RenderPass::DebugStageFlags::kBenchmarkPaths;
  }
  else
  {
    RV::RenderPass::Debug::_activeDebugStageFlags &=
        ~RV::RenderPass::DebugStageFlags::kBenchmarkPaths;
  }

  if (_ui.actionWireframe_Rendering->isChecked())
  {
    RV::RenderPass::Debug::_activeDebugStageFlags |=
        RV::RenderPass::DebugStageFlags::kWireframeRendering;
  }
  else
  {
    RV::RenderPass::Debug::_activeDebugStageFlags &=
        ~RV::RenderPass::DebugStageFlags::kWireframeRendering;
  }
}

void IntrinsicEd::onCompileShaders() { GpuProgramManager::compileAllShaders(); }

void IntrinsicEd::onRecompileShaders()
{
  GpuProgramManager::compileAllShaders(true);
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

void IntrinsicEd::updateUI()
{
  updateStatusBar(_ui.statusBar);
  if (!_dayNightSlider->hasFocus())
  {
    _dayNightSlider->blockSignals(true);
    _dayNightSlider->setValue((int)(World::_currentTime * 100.0f));
    _dayNightSlider->blockSignals(false);
  }
}

int IntrinsicEd::enterMainLoop()
{
  while (Application::_running)
  {
    qApp->processEvents();

    updateUI();
    TaskManager::executeTasks();
    _propertyView->updatePropertyView();

    if (_settingsUpdatePending)
    {
      Settings::Manager::loadSettings();
      updateSettingsChangeWatch();
      RV::RenderSystem::onViewportChanged();
      _settingsUpdatePending = false;
    }
  }

  return 0;
}
