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
#include "ui_IntrinsicEdPropertyEditorUInt.h"

IntrinsicEdPropertyEditorUInt::IntrinsicEdPropertyEditorUInt(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent)
{
  _ui.setupUi(this);
  updateFromProperty();

  QObject::connect(_ui.value, SIGNAL(valueChanged(int)), this,
                   SLOT(onValueChanged()));
}

IntrinsicEdPropertyEditorUInt::~IntrinsicEdPropertyEditorUInt() {}

void IntrinsicEdPropertyEditorUInt::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.value->setReadOnly(true);
  }

  if (prop["value"].GetUint() != _ui.value->value() && !_ui.value->hasFocus())
  {
    _ui.value->blockSignals(true);
    _ui.value->setValue(prop["value"].GetUint());
    _ui.value->blockSignals(false);
  }

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorUInt::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  if (prop["value"].GetUint() != _ui.value->value())
  {
    prop["value"].SetUint((float)_ui.value->value());

    emit valueChanged(*_properties);
  }
}
