// Copyright 2017 Benjamin Glatzel
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

// Ui
#include "ui_IntrinsicEdPropertyView.h"

IntrinsicEdPropertyView::IntrinsicEdPropertyView(QWidget* parent)
    : QDockWidget(parent)
{
  _ui.setupUi(this);

  // Setup UI
  {
    _ui.autoCollapseButton->setCheckable(true);
  }

  QObject::connect(_ui.refreshToolButton, SIGNAL(clicked()), this,
                   SLOT(onRefreshProperties()));
  QObject::connect(_ui.collapseAllButton, SIGNAL(clicked()), this,
                   SLOT(onCollapseAll()));
  QObject::connect(_ui.expandAllButton, SIGNAL(clicked()), this,
                   SLOT(onExpandAll()));
  QObject::connect(_ui.autoCollapseButton, SIGNAL(toggled(bool)), this,
                   SLOT(onAutoCollapsePropertyCategories(bool)));

  // Setup JSON document
  _propertyDocument = rapidjson::Document(rapidjson::kArrayType);
  for (uint32_t i = 0; i < 16u; ++i)
  {
    _propertyDocument.PushBack(rapidjson::Value(rapidjson::kObjectType),
                               _propertyDocument.GetAllocator());
  }
}

IntrinsicEdPropertyView::~IntrinsicEdPropertyView() {}

void IntrinsicEdPropertyView::onRefreshProperties()
{
  clearAndUpdatePropertyView();
}

void IntrinsicEdPropertyView::onCollapseAll()
{
  for (auto it = _categoryCollapsedState.begin();
       it != _categoryCollapsedState.end(); ++it)
  {
    it->second = true;
  }

  clearAndUpdatePropertyView();
}

void IntrinsicEdPropertyView::onExpandAll()
{
  for (auto it = _categoryCollapsedState.begin();
       it != _categoryCollapsedState.end(); ++it)
  {
    it->second = false;
  }

  clearAndUpdatePropertyView();
}

void IntrinsicEdPropertyView::onAutoCollapsePropertyCategories(bool p_Checked)
{
  // Collapse all categories but the one that has been selected
  if (p_Checked)
  {
    for (auto it = _categoryCollapsedState.begin();
         it != _categoryCollapsedState.end(); ++it)
    {
      it->second = it->first != _currentCategory || _currentCategory == "";
    }

    clearAndUpdatePropertyView();
  }
}

void IntrinsicEdPropertyView::clearPropertyView()
{
  QLayout* layout = _ui.scrollAreaWidgetContents->layout();
  _INTR_ASSERT(layout);

  QLayoutItem* item;
  while ((item = layout->takeAt(0)) != nullptr)
  {
    delete item->widget();
    delete item;
  }
}

QFrame* IntrinsicEdPropertyView::createCategoryHeaderWidget(const char* p_Title,
                                                            bool p_Collapsed)
{
  QFrame* frame = new QFrame();
  frame->setLayout(new QHBoxLayout());
  frame->setFrameStyle(QFrame::Shape::StyledPanel | QFrame::Shadow::Raised);
  frame->setMinimumSize(0, 32);
  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  frame->layout()->setContentsMargins(QMargins(6, 2, 6, 2));

  QLabel* label = new QLabel(p_Title);
  label->setObjectName("title");

  QPushButton* button = new QPushButton();

  if (p_Collapsed)
  {
    button->setIcon(QIcon(":/Icons/icons/arrows/shrink.png"));
  }
  else
  {
    button->setIcon(QIcon(":/Icons/icons/arrows/expand.png"));
  }

  button->setStyleSheet(
      "QPushButton { border-style: outset; border-width: 0px; }");

  frame->layout()->addWidget(button);

  QObject::connect(button, SIGNAL(clicked()), this,
                   SLOT(onCategoryHeaderClicked()));

  // Create icon (if mapping is available)
  {
    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(IntrinsicEd::getPixmap(p_Title));
    frame->layout()->addWidget(iconLabel);
  }

  frame->layout()->addWidget(label);
  frame->layout()->addItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  frame->setStyleSheet(
      ".QFrame { border: 1px solid #2d3339; border-radius: 6px;"
      "background-color: qlineargradient(x1: 0 y1: 0, x2: 0 "
      "y2: 1, stop: 0 #22262a, stop: 1 #202428); }");

  return frame;
}

void IntrinsicEdPropertyView::clearAndUpdatePropertyView()
{
  if (_propertyDocument.Empty())
  {
    return;
  }

  clearPropertyView();
  _propertyEntries.clear();

  const char* categories[16] = {};
  uint32_t categoryCount = 0u;

  // Collect categories and compile properties
  for (uint32_t ci = 0u; ci < _currentPropertyCompilerEntries.size(); ++ci)
  {
    rapidjson::Value& currentProperties = _propertyDocument[ci];
    currentProperties.RemoveAllMembers();

    Dod::PropertyCompilerEntry& compilerEntry =
        _currentPropertyCompilerEntries[ci];

    // Compile properties
    {
      compilerEntry.compileFunction(compilerEntry.ref, true, currentProperties,
                                    _propertyDocument);
    }

    // Collect categories
    for (auto it = currentProperties.MemberBegin();
         it != currentProperties.MemberEnd(); ++it)
    {
      const rapidjson::Value& prop = it->value;
      if (!prop.IsObject() || !prop.HasMember("cat") ||
          prop["internal"].GetBool())
      {
        continue;
      }

      const char* cat = prop["cat"].GetString();

      bool found = false;
      for (uint32_t i = 0; i < categoryCount; ++i)
      {
        if (strcmp(categories[i], cat) == 0u)
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        categories[categoryCount++] = cat;
      }
    }
  }

  // Create category headers and add property editors
  for (uint32_t ci = 0u; ci < categoryCount; ++ci)
  {
    bool collapsedState = false;

    if (!_ui.autoCollapseButton->isChecked())
    {
      auto collapsedStateIt = _categoryCollapsedState.find(categories[ci]);
      if (collapsedStateIt != _categoryCollapsedState.end())
      {
        collapsedState = collapsedStateIt->second;
      }
      else
      {
        _categoryCollapsedState[categories[ci]] = false;
      }
    }
    else
    {
      collapsedState = _currentCategory != categories[ci];
    }

    QFrame* header = createCategoryHeaderWidget(categories[ci], collapsedState);
    _ui.scrollAreaWidgetContents->layout()->addWidget(header);

    if (collapsedState)
    {
      continue;
    }

    for (uint32_t pi = 0u; pi < _currentPropertyCompilerEntries.size(); ++pi)
    {
      rapidjson::Value& currentProperties = _propertyDocument[pi];

      for (auto it = currentProperties.MemberBegin();
           it != currentProperties.MemberEnd(); ++it)
      {
        const rapidjson::Value& prop = it->value;
        if (!prop.IsObject() || !prop.HasMember("cat"))
        {
          continue;
        }

        const char* cat = prop["cat"].GetString();

        if (prop["internal"].GetBool() || strcmp(categories[ci], cat) != 0u)
        {
          continue;
        }

        const char* editor = prop["editor"].GetString();

        IntrinsicEdPropertyEditorBase* propertyEditor = nullptr;
        if (strcmp(editor, "vec2") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorVec2(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "vec3") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorVec3(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "vec4") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorVec4(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "string") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorString(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "enum") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorEnum(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "float") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorFloat(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "rotation") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorRotation(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "meshSelector") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorResourceSelector(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString(), "Mesh");
        }
        else if (strcmp(editor, "scriptSelector") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorResourceSelector(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString(), "Script");
        }
        else if (strcmp(editor, "flags") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorFlags(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }
        else if (strcmp(editor, "color") == 0u)
        {
          propertyEditor = new IntrinsicEdPropertyEditorColor(
              &_propertyDocument, &currentProperties, &it->value,
              it->name.GetString());
        }

        if (propertyEditor)
        {
          QObject::connect(propertyEditor,
                           SIGNAL(valueChanged(rapidjson::Value&)), this,
                           SLOT(onValueChanged(rapidjson::Value&)));

          _ui.scrollAreaWidgetContents->layout()->addWidget(propertyEditor);
          _propertyEntries.push_back(
              {propertyEditor, pi, it->name.GetString()});
        }
      }
    }
  }

  QSpacerItem* spacer =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  _ui.scrollAreaWidgetContents->layout()->addItem(spacer);
}

void IntrinsicEdPropertyView::updatePropertyView()
{
  for (uint32_t ci = 0u; ci < _currentPropertyCompilerEntries.size(); ++ci)
  {
    rapidjson::Value& currentProperties = _propertyDocument[ci];
    currentProperties.RemoveAllMembers();

    Dod::PropertyCompilerEntry& compilerEntry =
        _currentPropertyCompilerEntries[ci];

    // Compile properties
    {
      compilerEntry.compileFunction(compilerEntry.ref, true, currentProperties,
                                    _propertyDocument);
    }
  }

  for (uint32_t i = 0u; i < _propertyEntries.size(); ++i)
  {
    const PropertyEntry& entry = _propertyEntries[i];
    rapidjson::Value& currentProperties =
        _propertyDocument[entry.propertyCompilerEntryIdx];

    entry.propertyEditor->init(&_propertyDocument, &currentProperties,
                               &currentProperties[entry.propertyName.c_str()],
                               entry.propertyName.c_str());
    entry.propertyEditor->updateFromProperty();
  }
}

void IntrinsicEdPropertyView::clearPropertySet()
{
  _currentPropertyCompilerEntries.clear();
  _currentManagerEntries.clear();
}

void IntrinsicEdPropertyView::addPropertySet(
    const Dod::PropertyCompilerEntry& p_Entry,
    const Dod::ManagerEntry& p_ManagerEntry)
{
  _currentPropertyCompilerEntries.push_back(p_Entry);
  _currentManagerEntries.push_back(p_ManagerEntry);
}

void IntrinsicEdPropertyView::onValueChanged(rapidjson::Value& p_Properties)
{
  for (uint32_t i = 0u; i < _propertyDocument.Size(); ++i)
  {
    rapidjson::Value& currentProperties = _propertyDocument[i];
    if (currentProperties == p_Properties)
    {
      Dod::PropertyCompilerEntry& entry = _currentPropertyCompilerEntries[i];
      Dod::ManagerEntry& managerEntry = _currentManagerEntries[i];

      entry.initFunction(entry.ref, true, p_Properties);

      // Recreate resources if available
      if (managerEntry.destroyResourcesFunction &&
          managerEntry.createResourcesFunction)
      {
        Dod::RefArray refs = {entry.ref};
        managerEntry.destroyResourcesFunction(refs);
        managerEntry.createResourcesFunction(refs);
      }

      if (managerEntry.onPropertyUpdateFinishedFunction)
      {
        managerEntry.onPropertyUpdateFinishedFunction(entry.ref);
      }
    }
  }
}

void IntrinsicEdPropertyView::onCategoryHeaderClicked()
{
  QPushButton* categoryHeaderButton = (QPushButton*)QObject::sender();
  const QFrame* categoryHeader = (QFrame*)categoryHeaderButton->parent();

  for (uint32_t i = 0; i < (uint32_t)categoryHeader->layout()->count(); ++i)
  {
    QLayoutItem* layoutItem = categoryHeader->layout()->itemAt(i);

    if (layoutItem->widget()->objectName() == "title")
    {
      QLabel* label = (QLabel*)layoutItem->widget();
      _INTR_STRING cat = _INTR_STRING(label->text().toStdString().c_str());

      auto collapsedStateIt = _categoryCollapsedState.find(cat);
      if (collapsedStateIt != _categoryCollapsedState.end())
      {
        collapsedStateIt->second = !collapsedStateIt->second;
      }
      else
      {
        _categoryCollapsedState[cat] = true;
      }

      if (_categoryCollapsedState[cat])
      {
        categoryHeaderButton->setIcon(QIcon(":/Icons/roundRight"));
      }
      else
      {
        categoryHeaderButton->setIcon(QIcon(":/Icons/roundDown"));
      }

      _currentCategory = cat;
      break;
    }
  }

  clearAndUpdatePropertyView();
}
