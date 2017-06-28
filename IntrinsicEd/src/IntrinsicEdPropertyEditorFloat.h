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

#pragma once

// UI related includes
#include "ui_IntrinsicEdPropertyEditorFloat.h"

class IntrinsicEdPropertyEditorFloat : public IntrinsicEdPropertyEditorBase
{
  Q_OBJECT

public:
  IntrinsicEdPropertyEditorFloat(rapidjson::Document* p_Document,
                                 rapidjson::Value* p_CurrentProperties,
                                 rapidjson::Value* p_CurrentProperty,
                                 const char* p_PropertyName,
                                 QWidget* parent = 0);
  ~IntrinsicEdPropertyEditorFloat();

public slots:
  void onValueChanged();
  void onSliderValueChanged();

signals:
  void valueChanged(rapidjson::Value& p_Properties);

private:
  void updateFromProperty();

  Ui::IntrinsicEdPropertyEditorFloatClass _ui;
};
