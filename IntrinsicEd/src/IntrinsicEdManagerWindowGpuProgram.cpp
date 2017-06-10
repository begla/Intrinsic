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

IntrinsicEdManagerWindowGpuProgram::IntrinsicEdManagerWindowGpuProgram(
    QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("GPU Programs");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(GpuProgram)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(GpuProgram)];
  _managerPath = "managers/gpu_programs/";
  _managerExtension = ".gpu_program.json";
  _resourceName = "GpuProgram";
  _shaderAssetPath = "assets/shaders/";

  _shaderChangeWatch = new QFileSystemWatcher(this);
  QObject::connect(_shaderChangeWatch, SIGNAL(fileChanged(const QString&)),
                   this, SLOT(onShaderChanged(const QString&)));
  QObject::connect(this, SIGNAL(resourceTreePopulated()), this,
                   SLOT(onResourceTreePopulated()));
  QObject::connect(&_shaderRecompileTimer, SIGNAL(timeout()),
                   SLOT(onCompileQueuedShaders()));

  _shaderRecompileTimer.setSingleShot(true);

  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowGpuProgram::onShaderChanged(
    const QString& p_FileName)
{
  // Recompile shaders
  for (uint32_t i = 0u;
       i < RendererV::Resources::GpuProgramManager::getActiveResourceCount();
       ++i)
  {
    RendererV::Resources::GpuProgramRef ref =
        RendererV::Resources::GpuProgramManager::getActiveResourceAtIndex(i);
    if (strcmp(
            p_FileName.toStdString().c_str(),
            (_shaderAssetPath +
             RendererV::Resources::GpuProgramManager::_descGpuProgramName(ref)
                 .c_str())
                .toStdString()
                .c_str()) == 0u)
    {
      if (std::find(_shadersToRecompile.begin(), _shadersToRecompile.end(),
                    ref) == _shadersToRecompile.end())
      {
        _shadersToRecompile.push_back(ref);
      }
    }
  }

  _shaderRecompileTimer.start(100);
}

void IntrinsicEdManagerWindowGpuProgram::onResourceTreePopulated()
{
  QStringList files = _shaderChangeWatch->files();

  if (!files.empty())
    _shaderChangeWatch->removePaths(files);

  for (uint32_t i = 0u;
       i < RendererV::Resources::GpuProgramManager::getActiveResourceCount();
       ++i)
  {
    RendererV::Resources::GpuProgramRef ref =
        RendererV::Resources::GpuProgramManager::getActiveResourceAtIndex(i);
    _shaderChangeWatch->addPath(
        _shaderAssetPath +
        RendererV::Resources::GpuProgramManager::_descGpuProgramName(ref)
            .c_str());
  }
}

void IntrinsicEdManagerWindowGpuProgram::onCompileQueuedShaders()
{
  RendererV::Resources::GpuProgramManager::compileShaders(_shadersToRecompile);
  _shadersToRecompile.clear();

  onResourceTreePopulated();
}