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
