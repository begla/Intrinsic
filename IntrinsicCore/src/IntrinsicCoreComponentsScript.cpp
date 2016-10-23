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

    scriptResRef =
        Resources::ScriptManager::getResourceByName(_descScriptName(scriptRef));
    Resources::ScriptManager::callOnCreate(scriptResRef, scriptRef._id);
  }
}

void ScriptManager::destroyResources(const ScriptRefArray& p_Scripts)
{
  for (uint32_t scriptIdx = 0u; scriptIdx < p_Scripts.size(); ++scriptIdx)
  {
    ScriptRef scriptRef = p_Scripts[scriptIdx];
    Resources::ScriptRef scriptResRef = _script(scriptRef);

    Resources::ScriptManager::callOnDestroy(scriptResRef, scriptRef._id);
    _script(scriptRef) = Resources::ScriptRef();
  }
}

void ScriptManager::tickScripts(ScriptRefArray& p_Scripts, float p_DeltaT)
{
  _INTR_PROFILE_CPU("Components", "Tick scripts");

  for (uint32_t scriptIdx = 0u;
       scriptIdx < static_cast<uint32_t>(p_Scripts.size()); ++scriptIdx)
  {
    ScriptRef scriptRef = p_Scripts[scriptIdx];
    Resources::ScriptRef& scriptResRef = _script(scriptRef);

    if (scriptResRef.isValid())
    {
      Resources::ScriptManager::callTickScript(scriptResRef, scriptRef._id,
                                               p_DeltaT);
    }
  }
}
}
}
}
