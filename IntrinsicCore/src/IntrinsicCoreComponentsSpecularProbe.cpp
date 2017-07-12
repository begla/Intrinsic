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

namespace Intrinsic
{
namespace Core
{
namespace Components
{
void SpecularProbeManager::init()
{
  _INTR_LOG_INFO("Inititializing SpecularProbe Component Manager...");

  Dod::Components::ComponentManagerBase<
      SpecularProbeData,
      _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry specularProbeEntry;
  {
    specularProbeEntry.createFunction =
        SpecularProbeManager::createSpecularProbe;
    specularProbeEntry.destroyFunction =
        SpecularProbeManager::destroySpecularProbe;
    specularProbeEntry.getComponentForEntityFunction =
        SpecularProbeManager::getComponentForEntity;
    specularProbeEntry.resetToDefaultFunction =
        SpecularProbeManager::resetToDefault;
    specularProbeEntry.createResourcesFunction =
        SpecularProbeManager::createResources;
    specularProbeEntry.destroyResourcesFunction =
        SpecularProbeManager::destroyResources;

    Application::_componentManagerMapping[_N(SpecularProbe)] =
        specularProbeEntry;
    Application::_orderedComponentManagers.push_back(specularProbeEntry);
  }

  Dod::PropertyCompilerEntry propCompilerSpecularProbe;
  {
    propCompilerSpecularProbe.compileFunction =
        Components::SpecularProbeManager::compileDescriptor;
    propCompilerSpecularProbe.initFunction =
        Components::SpecularProbeManager::initFromDescriptor;
    propCompilerSpecularProbe.ref = Dod::Ref();
    Application::_componentPropertyCompilerMapping[_N(SpecularProbe)] =
        propCompilerSpecularProbe;
  }
}

// <-

void SpecularProbeManager::createResources(
    const SpecularProbeRefArray& p_Probes)
{
  ImageRefArray imagesToCreate;

  for (SpecularProbeRef probeRef : p_Probes)
  {
    _flags(probeRef) = 0u;
    for (Name& flag : _descFlags(probeRef))
    {
      if (flag == _N(ParallaxCorrected))
        _flags(probeRef) |= SpecularProbeFlags::kParallaxCorrected;
    }

    for (uint32_t i = 0u; i < _descSpecularTextureNames(probeRef).size(); ++i)
    {
      Name specularTextureName = _descSpecularTextureNames(probeRef)[i];

      ImageRef imageToCreate = ImageManager::createImage(specularTextureName);
      {
        ImageManager::addResourceFlags(
            imageToCreate, Dod::Resources::ResourceFlags::kResourceVolatile);

        ImageManager::_descFileName(imageToCreate) =
            specularTextureName.getString();
        ImageManager::_descDirPath(imageToCreate) = "media/specular_probes/";
        ImageManager::_descImageType(imageToCreate) =
            R::ImageType::kTextureFromFile;
        ImageManager::_descImageFormat(imageToCreate) = R::Format::kBC6UFloat;
      }
      imagesToCreate.push_back(imageToCreate);
    }
  }

  ImageManager::createResources(imagesToCreate);
}

// <-

void SpecularProbeManager::destroyResources(
    const SpecularProbeRefArray& p_Probes)
{
  ImageRefArray imagesToDestroy;

  for (SpecularProbeRef probeRef : p_Probes)
  {
    for (uint32_t i = 0u; i < _descSpecularTextureNames(probeRef).size(); ++i)
    {
      Name specularTextureName = _descSpecularTextureNames(probeRef)[i];
      ImageRef imageRef = ImageManager::_getResourceByName(specularTextureName);
      if (imageRef.isValid())
      {
        imagesToDestroy.push_back(imageRef);
      }
    }
  }

  ImageManager::destroyImagesAndResources(imagesToDestroy);
}
}
}
}
