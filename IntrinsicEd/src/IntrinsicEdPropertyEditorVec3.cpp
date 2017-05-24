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
#include "ui_IntrinsicEdPropertyEditorVec3.h"

IntrinsicEdPropertyEditorVec3::IntrinsicEdPropertyEditorVec3(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent)
{
  _ui.setupUi(this);
  updateFromProperty();

  QObject::connect(_ui.x, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
  QObject::connect(_ui.y, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
  QObject::connect(_ui.z, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
}

IntrinsicEdPropertyEditorVec3::~IntrinsicEdPropertyEditorVec3() {}

void IntrinsicEdPropertyEditorVec3::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.x->setReadOnly(true);
    _ui.y->setReadOnly(true);
    _ui.z->setReadOnly(true);
  }

  const bool hasFocus =
      _ui.x->hasFocus() || _ui.y->hasFocus() || _ui.z->hasFocus();
  const bool changed = prop["values"][0].GetFloat() != _ui.x->value() ||
                       prop["values"][1].GetFloat() != _ui.y->value() ||
                       prop["values"][2].GetFloat() != _ui.z->value();

  if (changed && !hasFocus)
  {
    _ui.x->blockSignals(true);
    _ui.y->blockSignals(true);
    _ui.z->blockSignals(true);
    _ui.x->setValue(prop["values"][0].GetFloat());
    _ui.y->setValue(prop["values"][1].GetFloat());
    _ui.z->setValue(prop["values"][2].GetFloat());
    _ui.x->blockSignals(false);
    _ui.y->blockSignals(false);
    _ui.z->blockSignals(false);
  }

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorVec3::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  bool changed = prop["values"][0].GetFloat() != _ui.x->value() ||
                 prop["values"][1].GetFloat() != _ui.y->value() ||
                 prop["values"][2].GetFloat() != _ui.z->value();

  if (changed)
  {
    prop["values"][0].SetFloat((float)_ui.x->value());
    prop["values"][1].SetFloat((float)_ui.y->value());
    prop["values"][2].SetFloat((float)_ui.z->value());

    emit valueChanged(*_properties);
  }
}
