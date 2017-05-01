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

#pragma once

class IntrinsicEdPropertyEditorBase : public QWidget
{
  Q_OBJECT

public:
  IntrinsicEdPropertyEditorBase(rapidjson::Document* p_Document,
                                rapidjson::Value* p_CurrentProperties,
                                rapidjson::Value* p_CurrentProperty,
                                const char* p_PropertyName, QWidget* parent)
      : QWidget(parent)
  {
    init(p_Document, p_CurrentProperties, p_CurrentProperty, p_PropertyName);
  }
  ~IntrinsicEdPropertyEditorBase() {}

  void init(rapidjson::Document* p_Document,
            rapidjson::Value* p_CurrentProperties,
            rapidjson::Value* p_CurrentProperty, const char* p_PropertyName)
  {
    _document = p_Document;
    _properties = p_CurrentProperties;
    _property = p_CurrentProperty;
    _propertyName = p_PropertyName;
  }

  virtual void updateFromProperty() = 0;

protected:
  rapidjson::Value* _property;
  rapidjson::Value* _properties;
  rapidjson::Document* _document;
  _INTR_STRING _propertyName;
};
