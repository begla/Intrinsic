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
#include "ui_IntrinsicEdPropertyEditorVec3.h"

class IntrinsicEdPropertyEditorVec3 : public IntrinsicEdPropertyEditorBase
{
  Q_OBJECT

public:
  IntrinsicEdPropertyEditorVec3(rapidjson::Document* p_Document,
                                rapidjson::Value* p_CurrentProperties,
                                rapidjson::Value* p_CurrentProperty,
                                const char* p_PropertyName,
                                QWidget* parent = 0);
  ~IntrinsicEdPropertyEditorVec3();

public slots:
  void onValueChanged();
  void updateFromProperty();

signals:
  void valueChanged(rapidjson::Value& p_Properties);

private:
  Ui::IntrinsicEdPropertyEditorVec3Class _ui;
};
