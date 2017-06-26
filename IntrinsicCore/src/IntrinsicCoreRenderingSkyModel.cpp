// Copyright 2017 Benjamin Glatzel
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

namespace
{
#include "ArHosekSkyModelData_RGB.h"
}

namespace Intrinsic
{
namespace Core
{
namespace Rendering
{
// Static members
double* SkyModel::datasetsRGB[] = {datasetRGB1, datasetRGB2, datasetRGB3};

double* SkyModel::datasetsRGBRad[] = {datasetRGBRad1, datasetRGBRad2,
                                      datasetRGBRad3};

SkyModel::ArHosekSkyModelState
SkyModel::createSkyModelStateRGB(double p_Turbidity, double p_Albedo,
                                 double p_Elevation)
{
  SkyModel::ArHosekSkyModelState state;

  state.solarRadius = TERRESTRIAL_SOLAR_RADIUS;
  state.turbidity = p_Turbidity;
  state.albedo = p_Albedo;
  state.elevation = p_Elevation;

  for (uint32_t channel = 0; channel < 3u; ++channel)
  {
    cookConfiguration(datasetsRGB[channel], state.configs[channel], p_Turbidity,
                      p_Albedo, p_Elevation);

    state.radiances[channel] = cookRadianceConfiguration(
        datasetsRGBRad[channel], p_Turbidity, p_Albedo, p_Elevation);
  }

  return state;
}
}
}
}
