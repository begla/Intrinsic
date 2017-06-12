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
void ScriptManager::createResources(const ScriptRefArray& p_Scripts)
{
  for (uint32_t scriptIdx = 0u; scriptIdx < p_Scripts.size(); ++scriptIdx)
  {
    ScriptRef scriptRef = p_Scripts[scriptIdx];
    Resources::ScriptRef& scriptResRef = _script(scriptRef);

    scriptResRef = Resources::ScriptManager::_getResourceByName(
        _descScriptName(scriptRef));

    if (scriptResRef.isValid())
    {
      Resources::ScriptManager::callOnCreate(scriptResRef, scriptRef);
    }
  }
}

// <-

void ScriptManager::destroyResources(const ScriptRefArray& p_Scripts)
{
  for (uint32_t scriptIdx = 0u; scriptIdx < p_Scripts.size(); ++scriptIdx)
  {
    ScriptRef scriptRef = p_Scripts[scriptIdx];
    Resources::ScriptRef scriptResRef = _script(scriptRef);

    Resources::ScriptManager::callOnDestroy(scriptResRef, scriptRef);
    _script(scriptRef) = Resources::ScriptRef();
  }
}

// <-

void ScriptManager::tickScripts(ScriptRefArray& p_Scripts, float p_DeltaT)
{
  _INTR_PROFILE_CPU("Scripts", "Tick Scripts");

  for (uint32_t scriptIdx = 0u;
       scriptIdx < static_cast<uint32_t>(p_Scripts.size()); ++scriptIdx)
  {
    ScriptRef scriptRef = p_Scripts[scriptIdx];
    Resources::ScriptRef& scriptResRef = _script(scriptRef);

    if (scriptResRef.isValid())
    {
      Resources::ScriptManager::callTickScript(scriptResRef, scriptRef,
                                               p_DeltaT);
    }
  }
}
}
}
}
