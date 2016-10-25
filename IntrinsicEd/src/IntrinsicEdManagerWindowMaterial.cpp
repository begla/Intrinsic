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

IntrinsicEdManagerWindowMaterial::IntrinsicEdManagerWindowMaterial(
    QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("Materials");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(Material)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(Material)];
  _resourceIcon = QIcon(":/Icons/brush");
  _managerFilePath = "managers/Material.manager.json";
  _resourceName = "Material";

  QObject::connect(this, SIGNAL(resourceTreePopulated()), this,
                   SLOT(onResourceTreePopulated()));

  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowMaterial::onResourceTreePopulated() {}
