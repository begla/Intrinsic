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

// Core related includes
#include "stdafx.h"

// Qt related includes
#include <QtWidgets>
#include <QtWidgets/QMainWindow>

// ->Ed related includes
#include "IntrinsicEdViewport.h"
#include "IntrinsicEd.h"
#include "IntrinsicEdNodeViewTreeWidget.h"
#include "IntrinsicEdNodeView.h"
#include "IntrinsicEdPropertyEditorBase.h"
#include "IntrinsicEdPropertyView.h"
#include "IntrinsicEdPropertyEditorVec2.h"
#include "IntrinsicEdPropertyEditorVec3.h"
#include "IntrinsicEdPropertyEditorRotation.h"
#include "IntrinsicEdPropertyEditorVec4.h"
#include "IntrinsicEdPropertyEditorColor.h"
#include "IntrinsicEdPropertyEditorFloat.h"
#include "IntrinsicEdPropertyEditorString.h"
#include "IntrinsicEdPropertyEditorEnum.h"
#include "IntrinsicEdPropertyEditorFlags.h"
#include "IntrinsicEdPropertyEditorResourceSelector.h"
#include "IntrinsicEdNotificationBase.h"
#include "IntrinsicEdNotificationSimple.h"
#include "IntrinsicEdManagerWindowBase.h"
#include "IntrinsicEdManagerWindowGpuProgram.h"
#include "IntrinsicEdManagerWindowScript.h"
#include "IntrinsicEdManagerWindowAsset.h"
#include "IntrinsicEdManagerWindowImage.h"
#include "IntrinsicEdManagerWindowMesh.h"
#include "IntrinsicEdManagerWindowMaterial.h"
#include "IntrinsicEdManagerWindowPostEffect.h"
