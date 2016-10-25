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
#include "ui_IntrinsicEdPropertyEditorRotation.h"

IntrinsicEdPropertyEditorRotation::IntrinsicEdPropertyEditorRotation(
    rapidjson::Document* p_Document, rapidjson::Value* p_CurrentProperties,
    rapidjson::Value* p_CurrentProperty, const char* p_PropertyName,
    QWidget* parent)
    : QWidget(parent), _properties(p_CurrentProperties),
      _property(p_CurrentProperty), _propertyName(p_PropertyName),
      _document(p_Document)
{
  _ui.setupUi(this);
  updateFromProperty();

  QObject::connect(_ui.yaw, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
  QObject::connect(_ui.pitch, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
  QObject::connect(_ui.roll, SIGNAL(valueChanged(double)), this,
                   SLOT(onValueChanged()));
  QObject::connect(_ui.sYaw, SIGNAL(valueChanged(int)), this,
                   SLOT(onSliderValueChanged()));
  QObject::connect(_ui.sPitch, SIGNAL(valueChanged(int)), this,
                   SLOT(onSliderValueChanged()));
  QObject::connect(_ui.sRoll, SIGNAL(valueChanged(int)), this,
                   SLOT(onSliderValueChanged()));
}

IntrinsicEdPropertyEditorRotation::~IntrinsicEdPropertyEditorRotation() {}

void IntrinsicEdPropertyEditorRotation::updateFromProperty()
{
  _INTR_ASSERT(_property);
  const rapidjson::Value& prop = *_property;

  if (prop["readOnly"].GetBool())
  {
    _ui.yaw->setReadOnly(true);
    _ui.pitch->setReadOnly(true);
    _ui.roll->setReadOnly(true);
    _ui.sYaw->setDisabled(true);
    _ui.sPitch->setDisabled(true);
    _ui.sRoll->setDisabled(true);
  }

  glm::vec3 euler;

  if (strcmp(prop["type"].GetString(), "quat") == 0u)
  {
    const glm::quat quat =
        glm::quat(prop["valueW"].GetFloat(), prop["valueX"].GetFloat(),
                  prop["valueY"].GetFloat(), prop["valueZ"].GetFloat());
    euler = glm::eulerAngles(quat);
  }
  else if (strcmp(prop["type"].GetString(), "vec3") == 0u)
  {
    euler =
        glm::vec3(glm::radians(glm::mod(prop["valueX"].GetFloat(), 360.0f)),
                  glm::radians(glm::mod(prop["valueY"].GetFloat(), 360.0f)),
                  glm::radians(glm::mod(prop["valueZ"].GetFloat(), 360.0f)));
  }

  _ui.yaw->setValue(glm::degrees(euler.x));
  _ui.pitch->setValue(glm::degrees(euler.y));
  _ui.roll->setValue(glm::degrees(euler.z));
  _ui.sYaw->setValue(glm::degrees(euler.x));
  _ui.sPitch->setValue(glm::degrees(euler.y));
  _ui.sRoll->setValue(glm::degrees(euler.z));

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorRotation::onValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  _ui.sYaw->setValue(_ui.yaw->value());
  _ui.sPitch->setValue(_ui.pitch->value());
  _ui.sRoll->setValue(_ui.roll->value());

  const glm::vec3 euler = glm::vec3(glm::radians(_ui.yaw->value()),
                                    glm::radians(_ui.pitch->value()),
                                    glm::radians(_ui.roll->value()));

  if (strcmp(prop["type"].GetString(), "quat") == 0u)
  {
    const glm::quat quat = glm::quat(euler);
    prop["valueX"].SetFloat(quat.x);
    prop["valueY"].SetFloat(quat.y);
    prop["valueZ"].SetFloat(quat.z);
    prop["valueW"].SetFloat(quat.w);
  }
  else if (strcmp(prop["type"].GetString(), "vec3") == 0u)
  {
    prop["valueX"].SetFloat(glm::degrees(euler.x));
    prop["valueY"].SetFloat(glm::degrees(euler.y));
    prop["valueZ"].SetFloat(glm::degrees(euler.z));
  }

  emit valueChanged(*_properties);
}

void IntrinsicEdPropertyEditorRotation::onSliderValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  _ui.yaw->setValue(_ui.sYaw->value());
  _ui.pitch->setValue(_ui.sPitch->value());
  _ui.roll->setValue(_ui.sRoll->value());

  onValueChanged();
}
