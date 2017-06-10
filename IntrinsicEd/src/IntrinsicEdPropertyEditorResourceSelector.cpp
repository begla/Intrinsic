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

// Ui
#include "ui_IntrinsicEdPropertyEditorResourceSelector.h"

IntrinsicEdPropertyEditorResourceSelector::
    IntrinsicEdPropertyEditorResourceSelector(
        rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
        rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
        const char* p_ResourceManagerName, QWidget* parent)
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent),
      _resourceManagerName(p_ResourceManagerName)
{
  _ui.setupUi(this);
  updateComboBoxItems();
  updateFromProperty();

  QObject::connect(_ui.comboBox, SIGNAL(currentIndexChanged(int)), this,
                   SLOT(onValueChanged()));
}

IntrinsicEdPropertyEditorResourceSelector::
    ~IntrinsicEdPropertyEditorResourceSelector()
{
}

void IntrinsicEdPropertyEditorResourceSelector::updateComboBoxItems()
{
  _ui.comboBox->blockSignals(true);
  _ui.comboBox->clear();

  const Dod::Resources::ResourceManagerEntry& managerEntry =
      Application::_resourceManagerMapping[_resourceManagerName];
  const Dod::PropertyCompilerEntry& propCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_resourceManagerName];

  QStringList completionItems;
  _ui.comboBox->addItem("None");
  completionItems.append("None");
  for (uint32_t i = 0u; i < managerEntry.getActiveResourceCountFunction(); ++i)
  {
    Dod::Ref resourceRef = managerEntry.getActiveResourceAtIndexFunction(i);

    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
    propCompilerEntry.compileFunction(resourceRef, true, properties,
                                      *_document);

    _INTR_STRING propertyName = properties["name"]["value"].GetString();

    _ui.comboBox->addItem(propertyName.c_str());
    completionItems.append(propertyName.c_str());

    _INTR_ASSERT(_property);
    rapidjson::Value& prop = *_property;
    if (prop["value"] == propertyName.c_str())
    {
      _ui.comboBox->setCurrentIndex(i + 1);
    }
  }

  QCompleter* completer = new QCompleter(completionItems);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setFilterMode(Qt::MatchFlag::MatchContains);
  _ui.comboBox->setCompleter(completer);

  _ui.comboBox->blockSignals(false);
}

void IntrinsicEdPropertyEditorResourceSelector::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.comboBox->setEditable(false);
  }

  _ui.comboBox->blockSignals(true);
  for (uint32_t i = 0u; i < (uint32_t)_ui.comboBox->count(); ++i)
  {
    const _INTR_STRING itemText =
        _ui.comboBox->itemText(i).toStdString().c_str();

    if (prop["value"] == itemText.c_str())
    {
      if (_ui.comboBox->currentIndex() != i)
      {
        const char* tttt = prop["value"].GetString();
        _ui.comboBox->setCurrentIndex(i);
        break;
      }
    }
  }
  _ui.comboBox->blockSignals(false);

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorResourceSelector::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  QString currentItemText =
      _ui.comboBox->itemText(_ui.comboBox->currentIndex());

  bool changed = false;
  if (_ui.comboBox->currentIndex() != 0)
  {
    if (prop["value"] != currentItemText.toStdString().c_str())
    {
      prop["value"].SetString(currentItemText.toStdString().c_str(),
                              _document->GetAllocator());
      changed = true;
    }
  }
  else
  {
    if (prop["value"] != "")
    {
      prop["value"].SetString("");
      changed = true;
    }
  }

  if (changed)
    emit valueChanged(*_properties);
}
