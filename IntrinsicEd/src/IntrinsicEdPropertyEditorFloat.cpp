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
#include "ui_IntrinsicEdPropertyEditorFloat.h"

IntrinsicEdPropertyEditorFloat::IntrinsicEdPropertyEditorFloat(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent)
{
  _ui.setupUi(this);

  QObject::connect(_ui.value, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));

  const rapidjson::Value& prop = *_property;
  if (!prop.HasMember("min") || prop["min"].GetFloat() == -FLT_MAX)
  {
    delete _ui.valueSlider;
    _ui.valueSlider = nullptr;
  }
  else
  {
    const float min = prop["min"].GetFloat();
    const float max = prop["max"].GetFloat();

    _ui.value->setMinimum(min);
    _ui.value->setMaximum(max);

    _ui.valueSlider->setMinimum(0);
    _ui.valueSlider->setMaximum(1000);

    QObject::connect(_ui.valueSlider, SIGNAL(valueChanged(int)), this,
                     SLOT(onSliderValueChanged()));
  }

  updateFromProperty();
}

IntrinsicEdPropertyEditorFloat::~IntrinsicEdPropertyEditorFloat() {}

void IntrinsicEdPropertyEditorFloat::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.value->setReadOnly(true);
  }

  if (prop["value"].GetFloat() != _ui.value->value() && !_ui.value->hasFocus())
  {
    _ui.value->blockSignals(true);
    _ui.value->setValue(prop["value"].GetFloat());
    _ui.value->blockSignals(false);

    if (_ui.valueSlider)
    {
      const int sliderPos =
          (int)((_ui.value->value() - _ui.value->minimum()) /
                (_ui.value->maximum() - _ui.value->minimum()) * 1000.0f);
      _ui.valueSlider->setValue(sliderPos);
    }
  }

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorFloat::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  if (prop["value"].GetFloat() != _ui.value->value())
  {
    prop["value"].SetFloat((float)_ui.value->value());

    if (_ui.valueSlider)
    {
      const int sliderPos =
          (int)((_ui.value->value() - _ui.value->minimum()) /
                (_ui.value->maximum() - _ui.value->minimum()) * 1000.0f);
      _ui.valueSlider->setValue(sliderPos);
    }

    emit valueChanged(*_properties);
  }
}

void IntrinsicEdPropertyEditorFloat::onSliderValueChanged()
{
  const float value =
      _ui.value->minimum() + (_ui.valueSlider->value() / 1000.0f) *
                                 (_ui.value->maximum() - _ui.value->minimum());
  _ui.value->setValue(value);
}
