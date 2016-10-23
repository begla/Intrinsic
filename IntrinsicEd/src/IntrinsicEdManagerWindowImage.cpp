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

IntrinsicEdManagerWindowImage::IntrinsicEdManagerWindowImage(QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("Images");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(Image)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(Image)];
  _resourceIcon = QIcon(":/Icons/picture");
  _managerFilePath = "managers/Image.manager.json";
  _resourceName = "Image";

  QObject::connect(this, SIGNAL(resourceTreePopulated()), this,
                   SLOT(onResourceTreePopulated()));

  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowImage::onResourceTreePopulated() {}
