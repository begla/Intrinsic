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
#include "ui_IntrinsicEdPropertyEditorFlags.h"

IntrinsicEdPropertyEditorFlags::IntrinsicEdPropertyEditorFlags(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent)
{
  _ui.setupUi(this);

  delete _ui.removeMe1;
  _ui.removeMe1 = nullptr;
  delete _ui.removeMe2;
  _ui.removeMe2 = nullptr;

  // Update flag items
  {
    const rapidjson::Value& prop = *_property;

    for (uint32_t i = 0u; i < prop["flagItems"].Size(); ++i)
    {
      QWidget* widgetToAddTo = i % 2u == 0u ? _ui.left : _ui.right;
      QCheckBox* checkBox = new QCheckBox();
      widgetToAddTo->layout()->addWidget(checkBox);
      _checkBoxes.push_back(checkBox);

      QObject::connect(checkBox, SIGNAL(stateChanged(int)), this,
                       SLOT(onValueChanged()));
    }
  }

  updateFromProperty();
}

IntrinsicEdPropertyEditorFlags::~IntrinsicEdPropertyEditorFlags() {}

void IntrinsicEdPropertyEditorFlags::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  // Update read only
  {
    bool readOnly = prop["readOnly"].GetBool();
    for (uint32_t i = 0u; i < _checkBoxes.size(); ++i)
    {
      _checkBoxes[i]->setCheckable(!readOnly);
    }
  }

  {
    for (uint32_t i = 0u; i < _checkBoxes.size(); ++i)
    {
      QCheckBox* checkBox = _checkBoxes[i];
      checkBox->setText(prop["flagItems"][i].GetString());
      checkBox->blockSignals(true);

      checkBox->setChecked(false);
      for (uint32_t j = 0u; j < prop["value"].Size(); ++j)
      {
        if (prop["flagItems"][i] == prop["value"][j])
        {
          checkBox->setChecked(true);
          break;
        }
      }

      checkBox->blockSignals(false);
    }
  }

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorFlags::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  prop["value"].Clear();
  for (uint32_t i = 0u; i < _checkBoxes.size(); ++i)
  {
    QCheckBox* checkBox = _checkBoxes[i];

    if (checkBox->isChecked())
    {
      rapidjson::Value val = rapidjson::Value();
      val.SetString(checkBox->text().toStdString().c_str(),
                    _document->GetAllocator());
      prop["value"].PushBack(val, _document->GetAllocator());
    }
  }

  emit valueChanged(*_properties);
}
