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
#include "ui_IntrinsicEdPropertyEditorEnum.h"

IntrinsicEdPropertyEditorEnum::IntrinsicEdPropertyEditorEnum(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : QWidget(parent), _property(p_CurrentProperty),
      _properties(p_CurrentProperties), _propertyName(p_PropertyName),
      _document(p_Document)
{
  _ui.setupUi(this);
  updateFromProperty();

  QObject::connect(_ui.comboBox, SIGNAL(currentIndexChanged(int)), this,
                   SLOT(onValueChanged()));
}

IntrinsicEdPropertyEditorEnum::~IntrinsicEdPropertyEditorEnum() {}

void IntrinsicEdPropertyEditorEnum::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.comboBox->setEditable(false);
  }

  _ui.comboBox->clear();
  for (uint32_t i = 0u; i < prop["enumItems"].Size(); ++i)
  {
    _ui.comboBox->addItem(prop["enumItems"][i].GetString());
  }

  _ui.comboBox->setCurrentIndex(prop["value"].GetUint());
  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorEnum::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  prop["value"].SetUint((uint32_t)_ui.comboBox->currentIndex());

  emit valueChanged(*_properties);
}
