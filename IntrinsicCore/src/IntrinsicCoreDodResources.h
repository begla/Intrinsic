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

#pragma once

namespace Intrinsic
{
namespace Core
{
namespace Dod
{
namespace Resources
{
// Function ptr typedefs
typedef Ref (*ManagerCreateFunction)(const Name&);
typedef void (*SaveToSingleFileFunction)(const char*);
typedef void (*SaveToMultipleFilesFunction)(const char*, const char*);
typedef void (*LoadFromSingleFileFunction)(const char*);
typedef void (*LoadFromMultipleFilesFunction)(const char*, const char*);
typedef uint8_t& (*GetResourceFlagsFunction)(Ref);

// <-

namespace ResourceFlags
{
enum Flags
{
  kResourceVolatile = 0x01u
};
}

// <-

struct ResourceDataBase
{
  ResourceDataBase(uint32_t p_Size)
  {
    name.resize(p_Size);
    resourceFlags.resize(p_Size);
  }

  _INTR_ARRAY(Name) name;
  _INTR_ARRAY(uint8_t) resourceFlags;
};

// <-

// Resource manager interface
struct ResourceManagerEntry : ManagerEntry
{
  ResourceManagerEntry()
      : ManagerEntry(), createFunction(nullptr),
        saveToSingleFileFunction(nullptr), loadFromSingleFileFunction(nullptr),
        saveToMultipleFilesFunction(nullptr),
        loadFromMultipleFilesFunction(nullptr),
        getResourceFlagsFunction(nullptr)
  {
  }

  ManagerCreateFunction createFunction;
  SaveToSingleFileFunction saveToSingleFileFunction;
  SaveToMultipleFilesFunction saveToMultipleFilesFunction;
  LoadFromSingleFileFunction loadFromSingleFileFunction;
  LoadFromMultipleFilesFunction loadFromMultipleFilesFunction;
  GetResourceFlagsFunction getResourceFlagsFunction;
};

// <-

// Resource manager base class
template <class DataType, uint32_t IdCount>
struct ResourceManagerBase : Dod::ManagerBase<IdCount, DataType>
{
  _INTR_INLINE static Ref getResourceByName(const Name& p_Name)
  {
    auto resourceIt = _nameResourceMap.find(p_Name);

    if (resourceIt == _nameResourceMap.end())
    {
      _INTR_LOG_WARNING("Resource '%s' not found - falling back to default "
                        "resource '%s'...",
                        p_Name.getString().c_str(),
                        _defaultResourceName.getString().c_str());
      resourceIt = _nameResourceMap.find(_defaultResourceName);
    }

    return resourceIt->second;
  }

  // <-

  _INTR_INLINE static Ref _getResourceByName(const Name& p_Name)
  {
    auto resourceIt = _nameResourceMap.find(p_Name);

    if (resourceIt == _nameResourceMap.end())
    {
      return Dod::Ref();
    }

    return resourceIt->second;
  }

  // <-

  _INTR_INLINE static void addResourceFlags(Ref p_Ref, uint8_t p_Flags)
  {
    _data.resourceFlags[p_Ref._id] |= p_Flags;
  }
  _INTR_INLINE static void removeResourceFlags(Ref p_Ref, uint8_t p_Flags)
  {
    _data.resourceFlags[p_Ref._id] &= ~p_Flags;
  }
  _INTR_INLINE static bool hasResourceFlags(Ref p_Ref, uint8_t p_Flags)
  {
    return (_data.resourceFlags[p_Ref._id] & p_Flags) == p_Flags;
  }

  // Getter/Setter
  // ->

  _INTR_INLINE static Name& _name(Ref p_Ref) { return _data.name[p_Ref._id]; }
  _INTR_INLINE static uint8_t& _resourceFlags(Ref p_Ref)
  {
    return _data.resourceFlags[p_Ref._id];
  }

  static _INTR_HASH_MAP(Name, Ref) _nameResourceMap;
  static DataType _data;
  static Name _defaultResourceName;

protected:
  _INTR_INLINE static void _initResourceManager()
  {
    Dod::ManagerBase<IdCount, DataType>::_initManager();
  }

  // <-

  _INTR_INLINE static Ref _createResource(const Name& p_Name)
  {
    Ref ref = Dod::ManagerBase<IdCount, DataType>::allocate();
    _data.name[ref._id] = p_Name;
    _nameResourceMap[p_Name] = ref;
    return ref;
  }

  // <-

  _INTR_INLINE static void _destroyResource(Ref p_Ref)
  {
    _nameResourceMap.erase(_name(p_Ref));
    Dod::ManagerBase<IdCount, DataType>::release(p_Ref);
  }

  // <-

  _INTR_INLINE static void _compileDescriptor(Ref p_Ref, bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties,
                                              rapidjson::Document& p_Document)
  {
    p_Properties.AddMember("name",
                           _INTR_CREATE_PROP(p_Document, p_GenerateDesc,
                                             _N(Resource), _N(string),
                                             _name(p_Ref), false, false),
                           p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void _initFromDescriptor(Ref p_Ref,
                                               rapidjson::Value& p_Properties)
  {
    _name(p_Ref) = JsonHelper::readPropertyName(p_Properties["name"]);
    _onNameChanged(p_Ref);
  }

  // <-

  _INTR_INLINE static void _onNameChanged(Ref p_Ref)
  {
    for (auto it = _nameResourceMap.begin(); it != _nameResourceMap.end();)
    {
      if (it->second == p_Ref)
      {
        it = _nameResourceMap.erase(it);
      }
      else
      {
        ++it;
      }
    }

    _nameResourceMap[_name(p_Ref)] = p_Ref;
  }

  template <
      class WriterType = rapidjson::PrettyWriter<rapidjson::FileWriteStream>>
  _INTR_INLINE static void
  _saveToSingleFile(const char* p_FileName,
                    ManagerCompileDescriptorFunction p_CompileFunction)
  {
    rapidjson::Document resources = rapidjson::Document(rapidjson::kArrayType);

    for (uint32_t i = 0;
         i < Dod::ManagerBase<IdCount, DataType>::_activeRefs.size(); ++i)
    {
      Ref ref = Dod::ManagerBase<IdCount, DataType>::_activeRefs[i];
      const Name& name = _name(ref);

      // Don't save volatile resources
      if (hasResourceFlags(ref, ResourceFlags::kResourceVolatile))
      {
        continue;
      }

      rapidjson::Value resource = rapidjson::Value(rapidjson::kObjectType);
      rapidjson::Value nameValue;
      nameValue.SetString(name.getString().c_str(), resources.GetAllocator());

      resource.AddMember("name", nameValue, resources.GetAllocator());

      rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
      p_CompileFunction(ref, false, properties, resources);

      resource.AddMember("properties", properties, resources.GetAllocator());

      resources.PushBack(resource, resources.GetAllocator());
    }

    FILE* fp = fopen(p_FileName, "wb");

    if (fp == nullptr)
    {
      _INTR_LOG_WARNING("Failed to save resources to file '%s'...", p_FileName);
      return;
    }

    char* writeBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
    {
      rapidjson::FileWriteStream os(fp, writeBuffer, 65536u);
      WriterType writer(os);
      resources.Accept(writer);
      fclose(fp);
    }
    Tlsf::MainAllocator::free(writeBuffer);
  }

  // <-

  template <
      class WriterType = rapidjson::PrettyWriter<rapidjson::FileWriteStream>>
  _INTR_INLINE static void
  _saveToMultipleFiles(const char* p_Path, const char* p_Extension,
                       ManagerCompileDescriptorFunction p_CompileFunction)
  {
    // Delete removed files
    {
      tinydir_dir dir;
      if (tinydir_open(&dir, p_Path) == -1)
      {
        _INTR_LOG_ERROR("Directory not found while saving resources to "
                        "multiple files...");
        return;
      }

      while (dir.has_next)
      {
        tinydir_file file;
        if (tinydir_readfile(&dir, &file) == -1)
        {
          _INTR_LOG_ERROR("Failed to read file in directory...");
          tinydir_next(&dir);
          continue;
        }

        _INTR_STRING resourceName, extension;
        StringUtil::extractFileNameAndExtension(file.path, resourceName,
                                                extension);

        // Ignore files not matching the extension
        if (extension != p_Extension)
        {
          tinydir_next(&dir);
          continue;
        }

        if (!_getResourceByName(resourceName).isValid())
        {
          std::remove(file.path);
        }

        tinydir_next(&dir);
      }

      tinydir_close(&dir);
    }

    for (uint32_t i = 0;
         i < Dod::ManagerBase<IdCount, DataType>::_activeRefs.size(); ++i)
    {
      Ref ref = Dod::ManagerBase<IdCount, DataType>::_activeRefs[i];
      _saveToMultipleFilesSingleResource(ref, p_Path, p_Extension,
                                         p_CompileFunction);
    }
  }

  // <-

  template <
      class WriterType = rapidjson::PrettyWriter<rapidjson::FileWriteStream>>
  _INTR_INLINE static void _saveToMultipleFilesSingleResource(
      Dod::Ref p_Ref, const char* p_Path, const char* p_Extension,
      ManagerCompileDescriptorFunction p_CompileFunction)
  {
    const Name& name = _name(p_Ref);

    // Don't save volatile resources
    if (hasResourceFlags(p_Ref, ResourceFlags::kResourceVolatile))
    {
      return;
    }

    rapidjson::Document resource = rapidjson::Document(rapidjson::kObjectType);
    rapidjson::Value nameValue;
    nameValue.SetString(name.getString().c_str(), resource.GetAllocator());

    resource.AddMember("name", nameValue, resource.GetAllocator());

    rapidjson::Value properties = rapidjson::Value(rapidjson::kObjectType);
    p_CompileFunction(p_Ref, false, properties, resource);

    resource.AddMember("properties", properties, resource.GetAllocator());

    _INTR_STRING fileName =
        _INTR_STRING(p_Path) + name.getString() + _INTR_STRING(p_Extension);

    FILE* fp = fopen(fileName.c_str(), "wb");

    if (fp == nullptr)
    {
      _INTR_LOG_WARNING("Failed to save resources to file '%s'...", fileName);
      return;
    }

    {
      char* writeBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
      rapidjson::FileWriteStream os(fp, writeBuffer, 65536u);
      WriterType writer(os);
      resource.Accept(writer);
      fclose(fp);
      Tlsf::MainAllocator::free(writeBuffer);
    }
  }

  // <-

  _INTR_INLINE static void
  _loadFromSingleFile(const char* p_FileName,
                      ManagerInitFromDescriptorFunction p_InitFunction,
                      ManagerResetToDefaultFunction p_ResetToDefaultFunction)
  {
    rapidjson::Document resources;
    {
      FILE* fp = fopen(p_FileName, "rb");

      if (fp == nullptr)
      {
        _INTR_LOG_WARNING("Failed to load resources from file '%s'...",
                          p_FileName);
        return;
      }

      char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);
      {
        rapidjson::FileReadStream is(fp, readBuffer, 65536u);
        resources.ParseStream(is);
        fclose(fp);
      }
      Tlsf::MainAllocator::free(readBuffer);
    }

    for (uint32_t i = 0u; i < resources.Size(); ++i)
    {
      rapidjson::Value& resource = resources[i];

      Ref ref = _createResource(resource["name"].GetString());
      p_ResetToDefaultFunction(ref);
      p_InitFunction(ref, resource["properties"]);
    }
  }

  // <-

  _INTR_INLINE static void
  _loadFromMultipleFiles(const char* p_Path, const char* p_Extension,
                         ManagerInitFromDescriptorFunction p_InitFunction,
                         ManagerResetToDefaultFunction p_ResetToDefaultFunction)
  {
    char* readBuffer = (char*)Tlsf::MainAllocator::allocate(65536u);

    tinydir_dir dir;
    if (tinydir_open(&dir, p_Path) == -1)
    {
      _INTR_LOG_ERROR("Directory not found while loading resources from "
                      "multiple files...");
      return;
    }

    while (dir.has_next)
    {
      tinydir_file file;
      if (tinydir_readfile(&dir, &file) == -1)
      {
        _INTR_LOG_ERROR("Failed to read file in directory...");
        tinydir_next(&dir);
        continue;
      }

      _INTR_STRING resourceName, extension;
      StringUtil::extractFileNameAndExtension(file.path, resourceName,
                                              extension);

      // Ignore files not matching the extension
      if (extension.find(p_Extension) == std::string::npos)
      {
        tinydir_next(&dir);
        continue;
      }

      FILE* fp = fopen(file.path, "rb");

      if (fp == nullptr)
      {
        _INTR_LOG_WARNING("Failed to load resources from file '%s'...",
                          resourceName.c_str());
        return;
      }

      rapidjson::Document resource;

      {
        rapidjson::FileReadStream is(fp, readBuffer, 65536u);
        resource.ParseStream(is);
      }

      fclose(fp);

      Ref ref = _createResource(resource["name"].GetString());
      p_ResetToDefaultFunction(ref);
      p_InitFunction(ref, resource["properties"]);

      tinydir_next(&dir);
    }

    tinydir_close(&dir);

    Tlsf::MainAllocator::free(readBuffer);
  }
};

template <class DataType, uint32_t IdCount>
DataType ResourceManagerBase<DataType, IdCount>::_data;
template <class DataType, uint32_t IdCount>
_INTR_HASH_MAP(Name, Ref)
ResourceManagerBase<DataType, IdCount>::_nameResourceMap;
template <class DataType, uint32_t IdCount>
Name ResourceManagerBase<DataType, IdCount>::_defaultResourceName;
}
}
}
}
