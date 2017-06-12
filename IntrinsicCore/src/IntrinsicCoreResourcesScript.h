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
namespace Resources
{
typedef Dod::Ref ScriptRef;
typedef _INTR_ARRAY(ScriptRef) ScriptRefArray;

struct ScriptData : Dod::Resources::ResourceDataBase
{
  ScriptData() : Dod::Resources::ResourceDataBase(_INTR_MAX_SCRIPT_COUNT)
  {
    descScriptFileName.resize(_INTR_MAX_SCRIPT_COUNT);
  }

  // Description
  _INTR_ARRAY(_INTR_STRING) descScriptFileName;
};

struct ScriptManager
    : Dod::Resources::ResourceManagerBase<ScriptData, _INTR_MAX_SCRIPT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static ScriptRef createScript(const Name& p_Name)
  {
    ScriptRef ref = Dod::Resources::ResourceManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COUNT>::_createResource(p_Name);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(ScriptRef p_Ref)
  {
    _descScriptFileName(p_Ref) = "";
  }

  // <-

  _INTR_INLINE static void destroyScript(ScriptRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COUNT>::_destroyResource(p_Ref);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(ScriptRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COUNT>::_compileDescriptor(p_Ref,
                                                                p_GenerateDesc,
                                                                p_Properties,
                                                                p_Document);

    p_Properties.AddMember(
        "scriptFileName",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Script), _N(string),
                          _descScriptFileName(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(ScriptRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COUNT>::_initFromDescriptor(p_Ref,
                                                                 p_Properties);

    if (p_Properties.HasMember("scriptFileName"))
      _descScriptFileName(p_Ref) =
          JsonHelper::readPropertyString(p_Properties["scriptFileName"]);
  }

  // <-

  _INTR_INLINE static void saveToMultipleFiles(const char* p_Path,
                                               const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<ScriptData, _INTR_MAX_SCRIPT_COUNT>::
        _saveToMultipleFiles<
            rapidjson::PrettyWriter<rapidjson::FileWriteStream>>(
            p_Path, p_Extension, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromMultipleFiles(const char* p_Path,
                                                 const char* p_Extension)
  {
    Dod::Resources::ResourceManagerBase<ScriptData, _INTR_MAX_SCRIPT_COUNT>::
        _loadFromMultipleFiles(p_Path, p_Extension, initFromDescriptor,
                               resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  // <-

  static void createResources(ScriptRef p_Script)
  {
    ScriptRefArray scripts;
    scripts.push_back(p_Script);
    createResources(scripts);
  }

  // <-

  static void createResources(const ScriptRefArray& p_Scripts);

  // <-

  static void destroyResources(ScriptRef p_Script)
  {
    ScriptRefArray scripts;
    scripts.push_back(p_Script);
    destroyResources(scripts);
  }

  // <-

  static void destroyResources(const ScriptRefArray& p_Scripts);

  // <-

  static void callTickScript(ScriptRef p_ScriptRef, Dod::Ref p_ScriptCompRef,
                             float p_DeltaT);
  static void callOnCreate(ScriptRef p_ScriptRef, Dod::Ref p_ScriptCompRef);
  static void callOnDestroy(ScriptRef p_Script, Dod::Ref p_ScriptCompRef);

  // Description
  _INTR_INLINE static _INTR_STRING& _descScriptFileName(ScriptRef p_Ref)
  {
    return _data.descScriptFileName[p_Ref._id];
  }
};
}
}
}
