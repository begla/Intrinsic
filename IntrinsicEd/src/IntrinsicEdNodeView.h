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
#include "ui_IntrinsicEdNodeView.h"

class IntrinsicEdNodeView : public QDockWidget
{
  Q_OBJECT

public:
  IntrinsicEdNodeView(QWidget* parent = 0);
  ~IntrinsicEdNodeView();

  void populateNodeTree();
  void onNodeCreatedOrDestroyed(Resources::EventRef p_Event);

private:
  Ui::InstrinsicEdNodeViewClass _ui;
};
