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

  // <-

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
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        ScriptData, _INTR_MAX_SCRIPT_COUNT>::_compileDescriptor(p_Ref,
                                                                p_Properties,
                                                                p_Document);

    p_Properties.AddMember("scriptFileName",
                           _INTR_CREATE_PROP(p_Document, _N(Script), _N(string),
                                             _descScriptFileName(p_Ref), false,
                                             false),
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

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<ScriptData, _INTR_MAX_SCRIPT_COUNT>::
        _saveToSingleFile(p_FileName, compileDescriptor);
  }

  // <-

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<ScriptData, _INTR_MAX_SCRIPT_COUNT>::
        _loadFromSingleFile(p_FileName, initFromDescriptor, resetToDefault);
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

  static void callTickScript(ScriptRef p_Script, uint32_t p_InstanceId,
                             float p_DeltaT);
  static void callOnCreate(ScriptRef p_Script, uint32_t p_InstanceId);
  static void callOnDestroy(ScriptRef p_Script, uint32_t p_InstanceId);

  // <-

  // Getter/Setter
  // Intrinsic

  // <-

  // Description
  _INTR_INLINE static _INTR_STRING& _descScriptFileName(ScriptRef p_Ref)
  {
    return _data.descScriptFileName[p_Ref._id];
  }
};
}
}
}
