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
namespace JsonHelper
{
_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const glm::vec2& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "vec2", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("valueX", p_Value.x, p_Doc.GetAllocator());
  property.AddMember("valueY", p_Value.y, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE glm::vec2 readPropertyVec2(const rapidjson::Value& p_Property)
{
  return glm::vec2(p_Property["valueX"].GetFloat(),
                   p_Property["valueY"].GetFloat());
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const glm::vec3& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "vec3", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("valueX", p_Value.x, p_Doc.GetAllocator());
  property.AddMember("valueY", p_Value.y, p_Doc.GetAllocator());
  property.AddMember("valueZ", p_Value.z, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE glm::vec3 readPropertyVec3(const rapidjson::Value& p_Property)
{
  return glm::vec3(p_Property["valueX"].GetFloat(),
                   p_Property["valueY"].GetFloat(),
                   p_Property["valueZ"].GetFloat());
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const glm::vec4& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "vec4", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("valueX", p_Value.x, p_Doc.GetAllocator());
  property.AddMember("valueY", p_Value.y, p_Doc.GetAllocator());
  property.AddMember("valueZ", p_Value.z, p_Doc.GetAllocator());
  property.AddMember("valueW", p_Value.z, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE glm::vec4 readPropertyVec4(const rapidjson::Value& p_Property)
{
  return glm::vec4(
      p_Property["valueX"].GetFloat(), p_Property["valueY"].GetFloat(),
      p_Property["valueZ"].GetFloat(), p_Property["valueW"].GetFloat());
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const glm::quat& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "quat", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("valueX", p_Value.x, p_Doc.GetAllocator());
  property.AddMember("valueY", p_Value.y, p_Doc.GetAllocator());
  property.AddMember("valueZ", p_Value.z, p_Doc.GetAllocator());
  property.AddMember("valueW", p_Value.w, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             float p_Value, bool p_ReadOnly,
                                             bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "float", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("value", p_Value, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE glm::quat readPropertyQuat(const rapidjson::Value& p_Property)
{
  return glm::quat(
      p_Property["valueW"].GetFloat(), p_Property["valueX"].GetFloat(),
      p_Property["valueY"].GetFloat(), p_Property["valueZ"].GetFloat());
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const Name& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "name", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  rapidjson::Value propertyValue =
      rapidjson::Value(p_Value._string.c_str(), p_Doc.GetAllocator());
  property.AddMember("value", propertyValue, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             const _INTR_STRING& p_Value,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "name", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  rapidjson::Value propertyValue =
      rapidjson::Value(p_Value.c_str(), p_Doc.GetAllocator());
  property.AddMember("value", propertyValue, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             uint32_t p_Value, bool p_ReadOnly,
                                             bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "name", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  property.AddMember("value", p_Value, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE rapidjson::Value createProperty(rapidjson::Document& p_Doc,
                                             const Name& p_Category,
                                             const Name& p_Editor,
                                             uint32_t p_Value,
                                             const _INTR_STRING& p_EnumItems,
                                             bool p_ReadOnly, bool p_Internal)
{
  rapidjson::Value property = rapidjson::Value(rapidjson::kObjectType);
  rapidjson::Value propertyCat =
      rapidjson::Value(p_Category._string.c_str(), p_Doc.GetAllocator());
  rapidjson::Value propertyEditor =
      rapidjson::Value(p_Editor._string.c_str(), p_Doc.GetAllocator());

  property.AddMember("cat", propertyCat, p_Doc.GetAllocator());
  property.AddMember("type", "name", p_Doc.GetAllocator());
  property.AddMember("editor", propertyEditor, p_Doc.GetAllocator());
  property.AddMember("readOnly", p_ReadOnly, p_Doc.GetAllocator());
  property.AddMember("internal", p_Internal, p_Doc.GetAllocator());

  rapidjson::Value enumItems = rapidjson::Value(rapidjson::kArrayType);
  _INTR_ARRAY(_INTR_STRING) items;
  StringUtil::split(p_EnumItems, ",", items);

  for (uint32_t i = 0u; i < items.size(); ++i)
  {
    rapidjson::Value item =
        rapidjson::Value(items[i].c_str(), p_Doc.GetAllocator());
    enumItems.PushBack(item, p_Doc.GetAllocator());
  }

  property.AddMember("enumItems", enumItems, p_Doc.GetAllocator());
  property.AddMember("value", p_Value, p_Doc.GetAllocator());

  return property;
}

_INTR_INLINE Name readPropertyName(const rapidjson::Value& p_Property)
{
  return Name(p_Property["value"].GetString());
}

_INTR_INLINE _INTR_STRING readPropertyString(const rapidjson::Value& p_Property)
{
  return _INTR_STRING(p_Property["value"].GetString());
}

_INTR_INLINE uint32_t readPropertyEnum(const rapidjson::Value& p_Property)
{
  return p_Property["value"].GetUint();
}

_INTR_INLINE uint32_t readPropertyUint(const rapidjson::Value& p_Property)
{
  return p_Property["value"].GetUint();
}

_INTR_INLINE float readPropertyFloat(const rapidjson::Value& p_Property)
{
  return p_Property["value"].GetFloat();
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec2& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kObjectType);

  vector.AddMember("x", p_Vector.x, p_Doc.GetAllocator());
  vector.AddMember("y", p_Vector.y, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec3& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kObjectType);

  vector.AddMember("x", p_Vector.x, p_Doc.GetAllocator());
  vector.AddMember("y", p_Vector.y, p_Doc.GetAllocator());
  vector.AddMember("z", p_Vector.z, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE rapidjson::Value createVec(rapidjson::Document& p_Doc,
                                        const glm::vec4& p_Vector)
{
  rapidjson::Value vector = rapidjson::Value(rapidjson::kObjectType);

  vector.AddMember("x", p_Vector.x, p_Doc.GetAllocator());
  vector.AddMember("y", p_Vector.y, p_Doc.GetAllocator());
  vector.AddMember("z", p_Vector.z, p_Doc.GetAllocator());
  vector.AddMember("w", p_Vector.w, p_Doc.GetAllocator());

  return vector;
}

_INTR_INLINE glm::vec2 readVec2(const rapidjson::Value& p_Vector)
{
  return glm::vec2(p_Vector["x"].GetFloat(), p_Vector["y"].GetFloat());
}

_INTR_INLINE glm::vec3 readVec3(const rapidjson::Value& p_Vector)
{
  return glm::vec3(p_Vector["x"].GetFloat(), p_Vector["y"].GetFloat(),
                   p_Vector["z"].GetFloat());
}

_INTR_INLINE glm::vec4 readVec4(const rapidjson::Value& p_Vector)
{
  return glm::vec4(p_Vector["x"].GetFloat(), p_Vector["y"].GetFloat(),
                   p_Vector["z"].GetFloat(), p_Vector["w"].GetFloat());
}
}
}
}
