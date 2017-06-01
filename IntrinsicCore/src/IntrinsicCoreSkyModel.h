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

/// Based on 'An Analytic Model for Full Spectral Sky-Dome Radiance'
/// ACM Transactions on Graphics (Proceedings of ACM SIGGRAPH 2012)
namespace Intrinsic
{
namespace Core
{
namespace SkyModel
{
#include "ArHosekSkyModelData_RGB.h"
#define TERRESTRIAL_SOLAR_RADIUS (glm::radians<double>(0.51) / 2.0)

typedef double* ArHosekSkyModelDataset;
typedef double* ArHosekSkyModelRadianceDataset;
typedef double ArHosekSkyModelConfiguration[9];

typedef struct ArHosekSkyModelState
{
  ArHosekSkyModelConfiguration configs[3];
  double radiances[3];
  double turbidity;
  double solarRadius;
  double emissionCorrectionFactorSky[3];
  double emissionCorrectionFactorSun[3];
  double albedo;
  double elevation;
} ArHosekSkyModelState;

// <-

_INTR_INLINE void cookConfiguration(ArHosekSkyModelDataset p_Dataset,
                                    ArHosekSkyModelConfiguration p_Config,
                                    double p_Turbidity, double p_Albedo,
                                    double p_SolarElevation)
{
  double* elevMatrix;

  int intTurbidity = (int)p_Turbidity;
  double turbidityRem = p_Turbidity - (double)intTurbidity;

  p_SolarElevation =
      pow(p_SolarElevation / (glm::pi<double>() / 2.0), (1.0 / 3.0));

  // alb 0 low turb

  elevMatrix = p_Dataset + (9 * 6 * (intTurbidity - 1));

  for (uint32_t i = 0; i < 9; ++i)
  {
    // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    p_Config[i] = (1.0 - p_Albedo) * (1.0 - turbidityRem) *
                  (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[i] +
                   5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
                       elevMatrix[i + 9] +
                   10.0 * pow(1.0 - p_SolarElevation, 3.0) *
                       pow(p_SolarElevation, 2.0) * elevMatrix[i + 18] +
                   10.0 * pow(1.0 - p_SolarElevation, 2.0) *
                       pow(p_SolarElevation, 3.0) * elevMatrix[i + 27] +
                   5.0 * (1.0 - p_SolarElevation) * pow(p_SolarElevation, 4.0) *
                       elevMatrix[i + 36] +
                   pow(p_SolarElevation, 5.0) * elevMatrix[i + 45]);
  }

  // alb 1 low turb
  elevMatrix = p_Dataset + (9 * 6 * 10 + 9 * 6 * (intTurbidity - 1));
  for (uint32_t i = 0; i < 9; ++i)
  {
    // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    p_Config[i] += (p_Albedo) * (1.0 - turbidityRem) *
                   (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[i] +
                    5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
                        elevMatrix[i + 9] +
                    10.0 * pow(1.0 - p_SolarElevation, 3.0) *
                        pow(p_SolarElevation, 2.0) * elevMatrix[i + 18] +
                    10.0 * pow(1.0 - p_SolarElevation, 2.0) *
                        pow(p_SolarElevation, 3.0) * elevMatrix[i + 27] +
                    5.0 * (1.0 - p_SolarElevation) *
                        pow(p_SolarElevation, 4.0) * elevMatrix[i + 36] +
                    pow(p_SolarElevation, 5.0) * elevMatrix[i + 45]);
  }

  if (intTurbidity == 10)
    return;

  // alb 0 high turb
  elevMatrix = p_Dataset + (9 * 6 * (intTurbidity));
  for (uint32_t i = 0; i < 9; ++i)
  {
    // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    p_Config[i] += (1.0 - p_Albedo) * (turbidityRem) *
                   (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[i] +
                    5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
                        elevMatrix[i + 9] +
                    10.0 * pow(1.0 - p_SolarElevation, 3.0) *
                        pow(p_SolarElevation, 2.0) * elevMatrix[i + 18] +
                    10.0 * pow(1.0 - p_SolarElevation, 2.0) *
                        pow(p_SolarElevation, 3.0) * elevMatrix[i + 27] +
                    5.0 * (1.0 - p_SolarElevation) *
                        pow(p_SolarElevation, 4.0) * elevMatrix[i + 36] +
                    pow(p_SolarElevation, 5.0) * elevMatrix[i + 45]);
  }

  // alb 1 high turb
  elevMatrix = p_Dataset + (9 * 6 * 10 + 9 * 6 * (intTurbidity));
  for (uint32_t i = 0; i < 9; ++i)
  {
    // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
    p_Config[i] += (p_Albedo) * (turbidityRem) *
                   (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[i] +
                    5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
                        elevMatrix[i + 9] +
                    10.0 * pow(1.0 - p_SolarElevation, 3.0) *
                        pow(p_SolarElevation, 2.0) * elevMatrix[i + 18] +
                    10.0 * pow(1.0 - p_SolarElevation, 2.0) *
                        pow(p_SolarElevation, 3.0) * elevMatrix[i + 27] +
                    5.0 * (1.0 - p_SolarElevation) *
                        pow(p_SolarElevation, 4.0) * elevMatrix[i + 36] +
                    pow(p_SolarElevation, 5.0) * elevMatrix[i + 45]);
  }
}

// <-

_INTR_INLINE double
cookRadianceConfiguration(ArHosekSkyModelRadianceDataset p_DataSet,
                          double p_Turbidity, double p_Albedo,
                          double p_SolarElevation)
{
  double* elevMatrix;

  int intTurbidity = (int)p_Turbidity;
  double turbidityRem = p_Turbidity - (double)intTurbidity;
  double res;

  p_SolarElevation =
      pow(p_SolarElevation / (glm::pi<double>() / 2.0), (1.0 / 3.0));

  // alb 0 low turb
  elevMatrix = p_DataSet + (6 * (intTurbidity - 1));
  // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res = (1.0 - p_Albedo) * (1.0 - turbidityRem) *
        (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[0] +
         5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
             elevMatrix[1] +
         10.0 * pow(1.0 - p_SolarElevation, 3.0) * pow(p_SolarElevation, 2.0) *
             elevMatrix[2] +
         10.0 * pow(1.0 - p_SolarElevation, 2.0) * pow(p_SolarElevation, 3.0) *
             elevMatrix[3] +
         5.0 * (1.0 - p_SolarElevation) * pow(p_SolarElevation, 4.0) *
             elevMatrix[4] +
         pow(p_SolarElevation, 5.0) * elevMatrix[5]);

  // alb 1 low turb
  elevMatrix = p_DataSet + (6 * 10 + 6 * (intTurbidity - 1));
  // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (p_Albedo) * (1.0 - turbidityRem) *
         (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[0] +
          5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
              elevMatrix[1] +
          10.0 * pow(1.0 - p_SolarElevation, 3.0) * pow(p_SolarElevation, 2.0) *
              elevMatrix[2] +
          10.0 * pow(1.0 - p_SolarElevation, 2.0) * pow(p_SolarElevation, 3.0) *
              elevMatrix[3] +
          5.0 * (1.0 - p_SolarElevation) * pow(p_SolarElevation, 4.0) *
              elevMatrix[4] +
          pow(p_SolarElevation, 5.0) * elevMatrix[5]);
  if (intTurbidity == 10)
    return res;

  // alb 0 high turb
  elevMatrix = p_DataSet + (6 * (intTurbidity));
  // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (1.0 - p_Albedo) * (turbidityRem) *
         (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[0] +
          5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
              elevMatrix[1] +
          10.0 * pow(1.0 - p_SolarElevation, 3.0) * pow(p_SolarElevation, 2.0) *
              elevMatrix[2] +
          10.0 * pow(1.0 - p_SolarElevation, 2.0) * pow(p_SolarElevation, 3.0) *
              elevMatrix[3] +
          5.0 * (1.0 - p_SolarElevation) * pow(p_SolarElevation, 4.0) *
              elevMatrix[4] +
          pow(p_SolarElevation, 5.0) * elevMatrix[5]);

  // alb 1 high turb
  elevMatrix = p_DataSet + (6 * 10 + 6 * (intTurbidity));
  // (1-t).^3* A1 + 3*(1-t).^2.*t * A2 + 3*(1-t) .* t .^ 2 * A3 + t.^3 * A4;
  res += (p_Albedo) * (turbidityRem) *
         (pow(1.0 - p_SolarElevation, 5.0) * elevMatrix[0] +
          5.0 * pow(1.0 - p_SolarElevation, 4.0) * p_SolarElevation *
              elevMatrix[1] +
          10.0 * pow(1.0 - p_SolarElevation, 3.0) * pow(p_SolarElevation, 2.0) *
              elevMatrix[2] +
          10.0 * pow(1.0 - p_SolarElevation, 2.0) * pow(p_SolarElevation, 3.0) *
              elevMatrix[3] +
          5.0 * (1.0 - p_SolarElevation) * pow(p_SolarElevation, 4.0) *
              elevMatrix[4] +
          pow(p_SolarElevation, 5.0) * elevMatrix[5]);

  return res;
}

_INTR_INLINE glm::vec3
calculateSkyModelRadiance(const ArHosekSkyModelState& p_SkyModelState,
                          const glm::vec3& p_Theta, const glm::vec3& p_Gamma)
{
  glm::vec3 configuration[9];
  for (uint32_t i = 0; i < 9; ++i)
    configuration[i] =
        glm::vec3(p_SkyModelState.configs[0][i], p_SkyModelState.configs[1][i],
                  p_SkyModelState.configs[2][i]);

  const glm::vec3 expM = glm::exp(configuration[4] * p_Gamma);
  const glm::vec3 rayM = glm::cos(p_Gamma) * glm::cos(p_Gamma);
  const glm::vec3 mieM =
      (glm::vec3(1.0f) + glm::cos(p_Gamma) * glm::cos(p_Gamma)) /
      glm::pow((1.0f + configuration[8] * configuration[8] -
                2.0f * configuration[8] * glm::cos(p_Gamma)),
               glm::vec3(1.5));
  const glm::vec3 zenith = glm::sqrt(glm::cos(p_Theta));

  return (glm::vec3(1.0f) +
          configuration[0] * glm::exp(configuration[1] /
                                      (glm::cos(p_Theta) + glm::vec3(0.01f)))) *
         (configuration[2] + configuration[3] * expM + configuration[5] * rayM +
          configuration[6] * mieM + configuration[7] * zenith);
}

_INTR_INLINE Irradiance::SH9
project(const ArHosekSkyModelState& p_SkyModelState,
        const glm::vec3& p_LightDir, uint32_t p_SampleCount)
{
  static _INTR_ARRAY(glm::vec3) randomRaySamples;
  while (randomRaySamples.size() < p_SampleCount)
  {
    randomRaySamples.push_back(
        glm::normalize(glm::vec3(Math::calcRandomFloatMinMax(-1.0f, 1.0f),
                                 Math::calcRandomFloatMinMax(-1.0f, 1.0f),
                                 Math::calcRandomFloatMinMax(-1.0f, 1.0f))));
  }

  Irradiance::SH9 result;
  float weightSum = 0.0f;
  for (uint32_t i = 0u; i < p_SampleCount; ++i)
  {
    const glm::vec3 randomRaySample = randomRaySamples[i];

    const glm::vec3 theta = glm::vec3(glm::acos(glm::max(
        glm::dot(randomRaySample, glm::vec3(0.0f, 1.0f, 0.0f)), 0.0001f)));
    const glm::vec3 gamma = glm::vec3(
        glm::acos(glm::max(glm::dot(randomRaySample, p_LightDir), 0.0001f)));

    const glm::vec3 sample =
        calculateSkyModelRadiance(p_SkyModelState, theta, gamma) *
        glm::vec3(p_SkyModelState.radiances[0], p_SkyModelState.radiances[1],
                  p_SkyModelState.radiances[2]);
    result += Irradiance::project(randomRaySample, sample);
    weightSum += 1.0f;
  }

  result *= glm::pi<float>() / weightSum;
  return result;
}

_INTR_INLINE ArHosekSkyModelState createSkyModelState(double p_Turbidity,
                                                      double p_Albedo,
                                                      double p_Elevation)
{
  ArHosekSkyModelState state;

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
