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

// UI related includes
#include "ui_IntrinsicEdPropertyView.h"

class IntrinsicEdPropertyView : public QDockWidget
{
  Q_OBJECT

  typedef _INTR_ARRAY(Dod::PropertyCompilerEntry) CompilerEntryArray;
  typedef _INTR_ARRAY(Dod::ManagerEntry) ManagerEntryArray;

public:
  IntrinsicEdPropertyView(QWidget* parent = 0);
  ~IntrinsicEdPropertyView();

  void addPropertySet(const Dod::PropertyCompilerEntry& p_Entry,
                      const Dod::ManagerEntry& p_ManagerEntry);
  void clearPropertySet();

  void clearPropertyView();
  void clearAndUpdatePropertyView();

public slots:
  void onValueChanged(rapidjson::Value& p_Properties);
  void onRefreshProperties();
  void onCollapseAll();
  void onExpandAll();
  void onAutoCollapsePropertyCategories(bool p_Checked);
  void onCategoryHeaderClicked();

private:
  QFrame* createCategoryHeaderWidget(const char* p_Title, bool p_Collapsed);

  Ui::IntrinsicEdPropertyViewClass _ui;
  CompilerEntryArray _currentPropertyCompilerEntries;
  ManagerEntryArray _currentManagerEntries;
  rapidjson::Document _propertyDocument;

  _INTR_HASH_MAP(_INTR_STRING, bool) _categoryCollapsedState;
  _INTR_STRING _currentCategory;
};
