// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
