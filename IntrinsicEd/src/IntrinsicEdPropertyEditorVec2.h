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

#pragma once

// UI related includes
#include "ui_IntrinsicEdPropertyEditorVec2.h"

class IntrinsicEdPropertyEditorVec2 : public QWidget
{
  Q_OBJECT

public:
  IntrinsicEdPropertyEditorVec2(rapidjson::Document* p_Document,
                                rapidjson::Value* p_CurrentProperties,
                                rapidjson::Value* p_CurrentProperty,
                                const char* p_PropertyName,
                                QWidget* parent = 0);
  ~IntrinsicEdPropertyEditorVec2();

public slots:
  void onValueChanged();

signals:
  void valueChanged(rapidjson::Value& p_Properties);

private:
  void updateFromProperty();

  Ui::IntrinsicEdPropertyEditorVec2Class _ui;
  rapidjson::Value* _property;
  rapidjson::Value* _properties;
  rapidjson::Document* _document;
  _INTR_STRING _propertyName;
};
