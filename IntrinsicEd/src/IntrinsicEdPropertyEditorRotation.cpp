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
    : IntrinsicEdPropertyEditorBase(p_Document, p_CurrentProperties,
                                    p_CurrentProperty, p_PropertyName, parent)
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
        glm::quat(prop["values"][3].GetFloat(), prop["values"][0].GetFloat(),
                  prop["values"][1].GetFloat(), prop["values"][2].GetFloat());
    euler = glm::eulerAngles(quat);
  }
  else if (strcmp(prop["type"].GetString(), "vec3") == 0u)
  {
    euler =
        glm::vec3(prop["values"][0].GetFloat(), prop["values"][1].GetFloat(),
                  prop["values"][2].GetFloat());
  }
  euler = glm::degrees(Math::wrapEuler(euler));

  bool changed = _ui.yaw->value() != euler.x || _ui.pitch->value() != euler.y ||
                 _ui.roll->value() != euler.z;

  if (changed)
  {
    _ui.yaw->blockSignals(true);
    _ui.pitch->blockSignals(true);
    _ui.roll->blockSignals(true);
    _ui.sYaw->blockSignals(true);
    _ui.sPitch->blockSignals(true);
    _ui.sRoll->blockSignals(true);
    _ui.yaw->setValue(euler.x);
    _ui.pitch->setValue(euler.y);
    _ui.roll->setValue(euler.z);
    _ui.sYaw->setValue(euler.x);
    _ui.sPitch->setValue(euler.y);
    _ui.sRoll->setValue(euler.z);
    _ui.yaw->blockSignals(false);
    _ui.pitch->blockSignals(false);
    _ui.roll->blockSignals(false);
    _ui.sYaw->blockSignals(false);
    _ui.sPitch->blockSignals(false);
    _ui.sRoll->blockSignals(false);
  }

  _ui.propertyTitle->setText(_propertyName.c_str());
}

void IntrinsicEdPropertyEditorRotation::onValueChanged()
{
  _ui.sYaw->blockSignals(true);
  _ui.sPitch->blockSignals(true);
  _ui.sRoll->blockSignals(true);
  _ui.sYaw->setValue(_ui.yaw->value());
  _ui.sPitch->setValue(_ui.pitch->value());
  _ui.sRoll->setValue(_ui.roll->value());
  _ui.sYaw->blockSignals(false);
  _ui.sPitch->blockSignals(false);
  _ui.sRoll->blockSignals(false);

  updateProperty();
}

void IntrinsicEdPropertyEditorRotation::updateProperty()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  const glm::vec3 euler = glm::vec3(glm::radians(_ui.yaw->value()),
                                    glm::radians(_ui.pitch->value()),
                                    glm::radians(_ui.roll->value()));

  bool changed = false;
  if (strcmp(prop["type"].GetString(), "quat") == 0u)
  {
    const glm::quat quat = glm::quat(euler);

    changed = prop["values"][0].GetFloat() != quat.x ||
              prop["values"][1].GetFloat() != quat.y ||
              prop["values"][2].GetFloat() != quat.z ||
              prop["values"][3].GetFloat() != quat.w;

    if (changed)
    {
      prop["values"][0].SetFloat(quat.x);
      prop["values"][1].SetFloat(quat.y);
      prop["values"][2].SetFloat(quat.z);
      prop["values"][3].SetFloat(quat.w);
    }
  }
  else if (strcmp(prop["type"].GetString(), "vec3") == 0u)
  {
    changed = prop["values"][0].GetFloat() != euler.x ||
              prop["values"][1].GetFloat() != euler.y ||
              prop["values"][2].GetFloat() != euler.z;

    if (changed)
    {
      prop["values"][0].SetFloat(euler.x);
      prop["values"][1].SetFloat(euler.y);
      prop["values"][2].SetFloat(euler.z);
    }
  }

  if (changed)
    emit valueChanged(*_properties);
}

void IntrinsicEdPropertyEditorRotation::onSliderValueChanged()
{
  _INTR_ASSERT(_property);
  rapidjson::Value& prop = *_property;

  _ui.yaw->blockSignals(true);
  _ui.pitch->blockSignals(true);
  _ui.roll->blockSignals(true);
  _ui.yaw->setValue(_ui.sYaw->value());
  _ui.pitch->setValue(_ui.sPitch->value());
  _ui.roll->setValue(_ui.sRoll->value());
  _ui.yaw->blockSignals(false);
  _ui.pitch->blockSignals(false);
  _ui.roll->blockSignals(false);

  updateProperty();
}
