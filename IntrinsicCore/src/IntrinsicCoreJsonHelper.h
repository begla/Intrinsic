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
namespace JsonHelper
{
_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const glm::vec2& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value values = rapidjson::Value(rapidjson::kArrayType);
  values.PushBack(p_Value.x, p_Doc.GetAllocator());
  values.PushBack(p_Value.y, p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    rapidjson::Value propertyCat =
        rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
    rapidjson::Value propertyEditor =
        rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "vec2", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    property.AddMember("values", values, p_Doc.GetAllocator());
  }
  else
  {
    property = values;
  }

  return property;
}

_INTR_INLINE glm::vec2 readPropertyVec2(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return glm::vec2(p_Property["values"][0].GetFloat(),
                     p_Property["values"][1].GetFloat());
  return glm::vec2(p_Property[0].GetFloat(), p_Property[1].GetFloat());
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const glm::vec3& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value values = rapidjson::Value(rapidjson::kArrayType);
  values.PushBack(p_Value.x, p_Doc.GetAllocator());
  values.PushBack(p_Value.y, p_Doc.GetAllocator());
  values.PushBack(p_Value.z, p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    rapidjson::Value propertyCat =
        rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
    rapidjson::Value propertyEditor =
        rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "vec3", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    property.AddMember("values", values, p_Doc.GetAllocator());
  }
  else
  {
    property = values;
  }

  return property;
}

_INTR_INLINE glm::vec3 readPropertyVec3(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return glm::vec3(p_Property["values"][0].GetFloat(),
                     p_Property["values"][1].GetFloat(),
                     p_Property["values"][2].GetFloat());
  return glm::vec3(p_Property[0].GetFloat(), p_Property[1].GetFloat(),
                   p_Property[2].GetFloat());
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const glm::vec4& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value values = rapidjson::Value(rapidjson::kArrayType);
  values.PushBack(p_Value.x, p_Doc.GetAllocator());
  values.PushBack(p_Value.y, p_Doc.GetAllocator());
  values.PushBack(p_Value.z, p_Doc.GetAllocator());
  values.PushBack(p_Value.w, p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    rapidjson::Value propertyCat =
        rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
    rapidjson::Value propertyEditor =
        rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "vec4", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    property.AddMember("values", values, p_Doc.GetAllocator());
  }
  else
  {
    property = values;
  }

  return property;
}

_INTR_INLINE glm::vec4 readPropertyVec4(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return glm::vec4(
        p_Property["values"][0].GetFloat(), p_Property["values"][1].GetFloat(),
        p_Property["values"][2].GetFloat(), p_Property["values"][3].GetFloat());
  return glm::vec4(p_Property[0].GetFloat(), p_Property[1].GetFloat(),
                   p_Property[2].GetFloat(), p_Property[3].GetFloat());
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const IBL::SH9& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value values = rapidjson::Value(rapidjson::kArrayType);

  glm::vec3* rawCoeffs = (glm::vec3*)&p_Value;

  for (uint32_t i = 0u; i < 9u; ++i)
  {
    const glm::vec3& coeff = rawCoeffs[i];

    values.PushBack(coeff.x, p_Doc.GetAllocator());
    values.PushBack(coeff.y, p_Doc.GetAllocator());
    values.PushBack(coeff.z, p_Doc.GetAllocator());
  }

  if (p_GenerateDesc)
  {
    rapidjson::Value propertyCat =
        rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
    rapidjson::Value propertyEditor =
        rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "sh", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    property.AddMember("values", values, p_Doc.GetAllocator());
  }
  else
  {
    property = values;
  }

  return property;
}

_INTR_INLINE IBL::SH9 readPropertySH(const rapidjson::Value& p_Property)
{
  IBL::SH9 coeffs;
  glm::vec3* rawCoeffs = (glm::vec3*)&coeffs;

  if (p_Property.IsObject())
  {
    for (uint32_t i = 0u; i < 9u; ++i)
    {
      rawCoeffs[i] = glm::vec3(p_Property["values"][i * 3u].GetFloat(),
                               p_Property["values"][i * 3u + 1u].GetFloat(),
                               p_Property["values"][i * 3u + 2u].GetFloat());
    }
  }
  else
  {
    for (uint32_t i = 0u; i < 9u; ++i)
    {
      rawCoeffs[i] = glm::vec3(p_Property[i * 3u].GetFloat(),
                               p_Property[i * 3u + 1u].GetFloat(),
                               p_Property[i * 3u + 2u].GetFloat());
    }
  }

  return coeffs;
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const glm::quat& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value values = rapidjson::Value(rapidjson::kArrayType);
  values.PushBack(p_Value.x, p_Doc.GetAllocator());
  values.PushBack(p_Value.y, p_Doc.GetAllocator());
  values.PushBack(p_Value.z, p_Doc.GetAllocator());
  values.PushBack(p_Value.w, p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    rapidjson::Value propertyCat =
        rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
    rapidjson::Value propertyEditor =
        rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "quat", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    property.AddMember("values", values, p_Doc.GetAllocator());
  }
  else
  {
    property = values;
  }

  return property;
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor, float p_Value,
               bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "float", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());
    property.AddMember("value", p_Value, p_Doc.GetAllocator());
  }
  else
  {
    property.SetFloat(p_Value);
  }

  return property;
}

_INTR_INLINE glm::quat readPropertyQuat(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return glm::quat(
        p_Property["values"][3].GetFloat(), p_Property["values"][0].GetFloat(),
        p_Property["values"][1].GetFloat(), p_Property["values"][2].GetFloat());

  return glm::quat(p_Property[3].GetFloat(), p_Property[0].GetFloat(),
                   p_Property[1].GetFloat(), p_Property[2].GetFloat());
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const Name& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  rapidjson::Value propertyValue =
      rapidjson::Value(p_Value.getString().c_str(), p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "name", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());
    property.AddMember("value", propertyValue, p_Doc.GetAllocator());
  }
  else
  {
    property = propertyValue;
  }

  return property;
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor,
               const _INTR_STRING& p_Value, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  rapidjson::Value propertyValue =
      rapidjson::Value(p_Value.c_str(), p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "name", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());
    property.AddMember("value", propertyValue, p_Doc.GetAllocator());
  }
  else
  {
    property = propertyValue;
  }

  return property;
}

_INTR_INLINE rapidjson::Value
createProperty(rapidjson::Document& p_Doc, bool p_GenerateDesc,
               const Name& p_Category, const Name& p_Editor, uint32_t p_Value,
               bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "name", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());
    property.AddMember("value", p_Value, p_Doc.GetAllocator());
  }
  else
  {
    property.SetUint(p_Value);
  }

  return property;
}

_INTR_INLINE rapidjson::Value
createPropertyEnum(rapidjson::Document& p_Doc, bool p_GenerateDesc,
                   const Name& p_Category, const Name& p_Editor,
                   uint32_t p_Value, const _INTR_STRING& p_EnumItems,
                   bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "name", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    rapidjson::Value enumItems = rapidjson::Value(rapidjson::kArrayType);
    {
      _INTR_ARRAY(_INTR_STRING) items;
      StringUtil::split(p_EnumItems, ",", items);

      for (uint32_t i = 0u; i < items.size(); ++i)
      {
        rapidjson::Value item =
            rapidjson::Value(items[i].c_str(), p_Doc.GetAllocator());
        enumItems.PushBack(item, p_Doc.GetAllocator());
      }
    }

    property.AddMember("enumItems", enumItems, p_Doc.GetAllocator());
    property.AddMember("value", p_Value, p_Doc.GetAllocator());
  }
  else
  {
    property.SetUint(p_Value);
  }

  return property;
}

_INTR_INLINE rapidjson::Value createPropertyFlags(
    rapidjson::Document& p_Doc, bool p_GenerateDesc, const Name& p_Category,
    const Name& p_Editor, const _INTR_ARRAY(Name) & p_Value,
    const _INTR_STRING& p_FlagItems, bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category.getString().c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor.getString().c_str(), p_Doc.GetAllocator());

  // Create a list of selected flags
  rapidjson::Value value = rapidjson::Value(rapidjson::kArrayType);
  {
    for (uint32_t i = 0u; i < p_Value.size(); ++i)
    {
      value.PushBack(rapidjson::Value(p_Value[i].getString().c_str(),
                                      p_Doc.GetAllocator()),
                     p_Doc.GetAllocator());
    }
  }

  if (p_GenerateDesc)
  {
    property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
    property.AddMember("type", "name", p_Doc.GetAllocator());
    property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
    property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
    property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

    rapidjson::Value flagItems = rapidjson::Value(rapidjson::kArrayType);
    {
      _INTR_ARRAY(_INTR_STRING) items;
      StringUtil::split(p_FlagItems, ",", items);

      for (uint32_t i = 0u; i < items.size(); ++i)
      {
        rapidjson::Value item =
            rapidjson::Value(items[i].c_str(), p_Doc.GetAllocator());
        flagItems.PushBack(item, p_Doc.GetAllocator());
      }
    }

    property.AddMember("flagItems", flagItems, p_Doc.GetAllocator());
    property.AddMember("value", value, p_Doc.GetAllocator());
  }
  else
  {
    property = value;
  }

  return property;
}

_INTR_INLINE Name readPropertyName(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return Name(p_Property["value"].GetString());
  return Name(p_Property.GetString());
}

_INTR_INLINE _INTR_STRING readPropertyString(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return _INTR_STRING(p_Property["value"].GetString());
  return _INTR_STRING(p_Property.GetString());
}

_INTR_INLINE void readPropertyFlagsNameArray(const rapidjson::Value& p_Property,
                                             _INTR_ARRAY(Name) & p_Strings)
{
  if (p_Property.IsObject())
  {
    const rapidjson::Value& strings = p_Property["value"];
    for (uint32_t i = 0u; i < strings.Size(); ++i)
      p_Strings.push_back(strings[i].GetString());
    return;
  }

  for (uint32_t i = 0u; i < p_Property.Size(); ++i)
    p_Strings.push_back(p_Property[i].GetString());
}

_INTR_INLINE uint32_t readPropertyEnumUint(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return p_Property["value"].GetUint();
  return p_Property.GetUint();
}

_INTR_INLINE uint32_t readPropertyUint(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return p_Property["value"].GetUint();
  return p_Property.GetUint();
}

_INTR_INLINE float readPropertyFloat(const rapidjson::Value& p_Property)
{
  if (p_Property.IsObject())
    return p_Property["value"].GetFloat();
  return p_Property.GetFloat();
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec2& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kArrayType);

  vector.PushBack(p_Vector.x, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.y, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec3& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kArrayType);

  vector.PushBack(p_Vector.x, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.y, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.z, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec4& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kArrayType);

  vector.PushBack(p_Vector.x, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.y, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.z, p_Doc.GetAllocator());
  vector.PushBack(p_Vector.w, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE glm::vec2 readVec2(const rapidjson::Value& p_Vector)
{
  return glm::vec2(p_Vector[0].GetFloat(), p_Vector[1].GetFloat());
}

_INTR_INLINE glm::vec3 readVec3(const rapidjson::Value& p_Vector)
{
  return glm::vec3(p_Vector[0].GetFloat(), p_Vector[1].GetFloat(),
                   p_Vector[2].GetFloat());
}

_INTR_INLINE glm::vec4 readVec4(const rapidjson::Value& p_Vector)
{
  return glm::vec4(p_Vector[0].GetFloat(), p_Vector[1].GetFloat(),
                   p_Vector[2].GetFloat(), p_Vector[3].GetFloat());
}
}
}
}
