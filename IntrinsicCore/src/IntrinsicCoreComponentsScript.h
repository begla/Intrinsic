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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Components
{
// Typedefs
typedef Dod::Ref ScriptRef;
typedef _INTR_ARRAY(ScriptRef) ScriptRefArray;

struct ScriptData : Dod::Components::ComponentDataBase
{
  ScriptData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_SCRIPT_COMPONENT_COUNT)
  {
    descScriptName.resize(_INTR_MAX_SCRIPT_COMPONENT_COUNT);

    script.resize(_INTR_MAX_SCRIPT_COMPONENT_COUNT);
  }

  // Description
  _INTR_ARRAY(Name) descScriptName;

  // Resources
  _INTR_ARRAY(Dod::Ref) script;
};

struct ScriptManager
    : Dod::Components::ComponentManagerBase<ScriptData,
                                            _INTR_MAX_SCRIPT_COMPONENT_COUNT>
{
  static void init()
  {
    _INTR_LOG_INFO("Inititializing Script Component Manager...");

    Dod::Components::ComponentManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COMPONENT_COUNT>::_initComponentManager();

    Dod::Components::ComponentManagerEntry scriptEntry;
    {
      scriptEntry.createFunction = Components::ScriptManager::createScript;
      scriptEntry.destroyFunction = Components::ScriptManager::destroyScript;
      scriptEntry.createResourcesFunction =
          Components::ScriptManager::createResources;
      scriptEntry.destroyResourcesFunction =
          Components::ScriptManager::destroyResources;
      scriptEntry.getComponentForEntityFunction =
          Components::ScriptManager::getComponentForEntity;
      scriptEntry.resetToDefaultFunction =
          Components::ScriptManager::resetToDefault;

      Application::_componentManagerMapping[_N(Script)] = scriptEntry;
      Application::_orderedComponentManagers.push_back(scriptEntry);
    }

    Dod::PropertyCompilerEntry propCompilerScript;
    {
      propCompilerScript.compileFunction =
          Components::ScriptManager::compileDescriptor;
      propCompilerScript.initFunction =
          Components::ScriptManager::initFromDescriptor;
      propCompilerScript.ref = Dod::Ref();
      Application::_componentPropertyCompilerMapping[_N(Script)] =
          propCompilerScript;
    }
  }

  // <-

  _INTR_INLINE static ScriptRef createScript(Entity::EntityRef p_ParentEntity)
  {
    ScriptRef ref = Dod::Components::ComponentManagerBase<
        ScriptData,
        _INTR_MAX_SCRIPT_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(ScriptRef p_Ref)
  {
    _descScriptName(p_Ref) = "";
  }

  // <-

  _INTR_INLINE static void destroyScript(ScriptRef p_Script)
  {
    // Destroy the actual resource
    Dod::Components::ComponentManagerBase<
        ScriptData,
        _INTR_MAX_SCRIPT_COMPONENT_COUNT>::_destroyComponent(p_Script);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(ScriptRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "scriptName", _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Script),
                                        _N(scriptSelector),
                                        _descScriptName(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(ScriptRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("scriptName"))
      _descScriptName(p_Ref) =
          JsonHelper::readPropertyName(p_Properties["scriptName"]);
  }

  // <-

  static void createResources(const ScriptRefArray& p_Scripts);
  static void destroyResources(const ScriptRefArray& p_Scripts);

  // <-

  static void tickScripts(ScriptRefArray& p_Scripts, float p_DeltaT);

  // Description
  _INTR_INLINE static Name& _descScriptName(ScriptRef p_Ref)
  {
    return _data.descScriptName[p_Ref._id];
  }

  // Resources
  _INTR_INLINE static Dod::Ref& _script(ScriptRef p_Ref)
  {
    return _data.script[p_Ref._id];
  }
};
}
}
}
