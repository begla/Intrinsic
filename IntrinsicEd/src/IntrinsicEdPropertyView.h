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
