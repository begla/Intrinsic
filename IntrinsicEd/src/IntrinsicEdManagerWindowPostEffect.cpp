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
#include "stdafx_editor.h"
#include "stdafx.h"

IntrinsicEdManagerWindowPostEffect::IntrinsicEdManagerWindowPostEffect(
    QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("PostEffects");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(PostEffect)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(PostEffect)];
  _resourceIcon = QIcon(":/Icons/calendar");
  _managerPath = "managers/post_effects/";
  _managerExtension = ".post_effect.json";
  _resourceName = "PostEffect";

  onPopulateResourceTree();
}