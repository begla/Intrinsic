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
#include "stdafx_editor.h"
#include "stdafx.h"

IntrinsicEdManagerWindowScript::IntrinsicEdManagerWindowScript(QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("Scripts");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(Script)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(Script)];
  _managerPath = "managers/scripts/";
  _managerExtension = ".script.json";
  _resourceName = "Script";
  _scriptPath = "scripts/";

  _scriptChangeWatch = new QFileSystemWatcher(this);
  QObject::connect(_scriptChangeWatch, SIGNAL(fileChanged(const QString&)),
                   this, SLOT(onScriptChanged(const QString&)));
  QObject::connect(this, SIGNAL(resourceTreePopulated()), this,
                   SLOT(onResourceTreePopulated()));

  onPopulateResourceTree();
}

void IntrinsicEdManagerWindowScript::onScriptChanged(const QString& p_FileName)
{
  new IntrinsicEdNotificationSimple(
      this, ("Script '" + p_FileName + "' changed...").toStdString().c_str());

  Resources::ScriptRefArray resourcesToRecreate;

  for (uint32_t i = 0u; i < Resources::ScriptManager::getActiveResourceCount();
       ++i)
  {
    Resources::ScriptRef ref =
        Resources::ScriptManager::getActiveResourceAtIndex(i);
    if (strcmp(p_FileName.toStdString().c_str(),
               (_scriptPath +
                Resources::ScriptManager::_descScriptFileName(ref).c_str())
                   .toStdString()
                   .c_str()) == 0u)
    {
      resourcesToRecreate.push_back(ref);
    }
  }

  Resources::ScriptManager::destroyResources(resourcesToRecreate);
  Resources::ScriptManager::createResources(resourcesToRecreate);
}

void IntrinsicEdManagerWindowScript::onResourceTreePopulated()
{
  QStringList files = _scriptChangeWatch->files();

  if (!files.empty())
    _scriptChangeWatch->removePaths(files);

  for (uint32_t i = 0u; i < Resources::ScriptManager::getActiveResourceCount();
       ++i)
  {
    Resources::ScriptRef ref =
        Resources::ScriptManager::getActiveResourceAtIndex(i);
    _scriptChangeWatch->addPath(
        _scriptPath +
        Resources::ScriptManager::_descScriptFileName(ref).c_str());
  }
}
