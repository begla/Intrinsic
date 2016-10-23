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
#include "ui_IntrinsicEdNodeView.h"
IntrinsicEdNodeView::IntrinsicEdNodeView(QWidget* parent) : QDockWidget(parent)
{
  _ui.setupUi(this);

  QObject::connect(_ui.refreshToolButton, SIGNAL(clicked()), _ui.treeWidget,
                   SLOT(onPopulateNodeTree()));
  QObject::connect(_ui.createRootNodeButton, SIGNAL(clicked()), _ui.treeWidget,
                   SLOT(onCreateRootNode()));
  QObject::connect(_ui.createNodeToolButton, SIGNAL(clicked()), _ui.treeWidget,
                   SLOT(onCreateNode()));

  Resources::EventManager::connect(
      _N(NodeCreated), std::bind(&IntrinsicEdNodeView::onNodeCreatedOrDestroyed,
                                 this, std::placeholders::_1));
  Resources::EventManager::connect(
      _N(NodeDestroyed),
      std::bind(&IntrinsicEdNodeView::onNodeCreatedOrDestroyed, this,
                std::placeholders::_1));
}

IntrinsicEdNodeView::~IntrinsicEdNodeView() {}

void IntrinsicEdNodeView::populateNodeTree()
{
  _ui.treeWidget->onPopulateNodeTree();
}

void IntrinsicEdNodeView::onNodeCreatedOrDestroyed(Resources::EventRef p_Event)
{
  _INTR_STRING countLabel =
      StringUtil::toString<uint32_t>(
          (uint32_t)Components::NodeManager::_activeRefs.size()) +
      " Nodes";
  _ui.nodeCountLabel->setText(countLabel.c_str());
}
