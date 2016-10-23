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

IntrinsicEdManagerWindowGpuProgram::IntrinsicEdManagerWindowGpuProgram(
    QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("GPU Programs");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(GpuProgram)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(GpuProgram)];
  _resourceIcon = QIcon(":/Icons/calendar");
  _managerFilePath = "managers/GpuProgram.manager.json";
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
       i < Vulkan::Resources::GpuProgramManager::getActiveResourceCount(); ++i)
  {
    Vulkan::Resources::GpuProgramRef ref =
        Vulkan::Resources::GpuProgramManager::getActiveResourceAtIndex(i);
    if (strcmp(p_FileName.toStdString().c_str(),
               (_shaderAssetPath +
                Vulkan::Resources::GpuProgramManager::_descGpuProgramName(ref)
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
       i < Vulkan::Resources::GpuProgramManager::getActiveResourceCount(); ++i)
  {
    Vulkan::Resources::GpuProgramRef ref =
        Vulkan::Resources::GpuProgramManager::getActiveResourceAtIndex(i);
    _shaderChangeWatch->addPath(
        _shaderAssetPath +
        Vulkan::Resources::GpuProgramManager::_descGpuProgramName(ref).c_str());
  }
}

void IntrinsicEdManagerWindowGpuProgram::onCompileQueuedShaders()
{
  Vulkan::Resources::GpuProgramManager::compileShaders(_shadersToRecompile);
  _shadersToRecompile.clear();

  onResourceTreePopulated();
}