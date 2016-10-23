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
#include "stdafx_editor.h"
#include "stdafx.h"

IntrinsicEdManagerWindowScript::IntrinsicEdManagerWindowScript(QWidget* parent)
    : IntrinsicEdManagerWindowBase(parent)
{
  setWindowTitle("Scripts");
  _propertyCompilerEntry =
      Application::_resourcePropertyCompilerMapping[_N(Script)];
  _resourceManagerEntry = Application::_resourceManagerMapping[_N(Script)];
  _resourceIcon = QIcon(":/Icons/calendar");
  _managerFilePath = "managers/Script.manager.json";
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
