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

// Defines
#define _INTR_MAX_ASSET_COUNT 4096u

// FBX
#undef snprintf
#include "fbxsdk.h"

// Core related includes
#include "stdafx.h"

// Asset Management related includes
#include "IntrinsicAssetManagementHelperFbx.h"
#include "IntrinsicAssetManagementResourcesAsset.h"
#include "IntrinsicAssetManagementImporterFbx.h"
#include "IntrinsicAssetManagementImporterTexture.h"
