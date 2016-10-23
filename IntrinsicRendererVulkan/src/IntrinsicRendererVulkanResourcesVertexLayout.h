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
namespace Renderer
{
namespace Vulkan
{
namespace Resources
{
typedef Dod::Ref VertexLayoutRef;
typedef Dod::RefArray VertexLayoutRefArray;

struct VertexLayoutData : Dod::Resources::ResourceDataBase
{
  VertexLayoutData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_VERTEX_LAYOUT_COUNT)
  {
    descVertexBindings.resize(_INTR_MAX_VERTEX_LAYOUT_COUNT);
    descVertexAttributes.resize(_INTR_MAX_VERTEX_LAYOUT_COUNT);

    vkVertexInputBindingDescs.resize(_INTR_MAX_VERTEX_LAYOUT_COUNT);
    vkVertexInputAttributeDescs.resize(_INTR_MAX_VERTEX_LAYOUT_COUNT);
    vkPipelineVertexInputStateCreateInfo.resize(_INTR_MAX_VERTEX_LAYOUT_COUNT);
  }

  _INTR_ARRAY(_INTR_ARRAY(VertexBinding)) descVertexBindings;
  _INTR_ARRAY(_INTR_ARRAY(VertexAttribute)) descVertexAttributes;

  // GPU resources
  _INTR_ARRAY(_INTR_ARRAY(VkVertexInputBindingDescription))
  vkVertexInputBindingDescs;
  _INTR_ARRAY(_INTR_ARRAY(VkVertexInputAttributeDescription))
  vkVertexInputAttributeDescs;
  _INTR_ARRAY(VkPipelineVertexInputStateCreateInfo)
  vkPipelineVertexInputStateCreateInfo;
};

struct VertexLayoutManager
    : Dod::Resources::ResourceManagerBase<VertexLayoutData,
                                          _INTR_MAX_VERTEX_LAYOUT_COUNT>
{
  _INTR_INLINE static void init()
  {
    _INTR_LOG_INFO("Inititializing Vertex Layout Manager...");

    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_initResourceManager();
  }

  _INTR_INLINE static VertexLayoutRef createVertexLayout(const Name& p_Name)
  {
    VertexLayoutRef ref = Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_createResource(p_Name);
    return ref;
  }

  _INTR_INLINE static void resetToDefault(VertexLayoutRef p_Ref)
  {
    _descVertexAttributes(p_Ref).clear();
  }

  _INTR_INLINE static void destroyVertexLayout(VertexLayoutRef p_Ref)
  {
    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_destroyResource(p_Ref);
  }

  _INTR_INLINE static void compileDescriptor(VertexLayoutRef p_Ref,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_compileDescriptor(p_Ref, p_Properties,
                                                           p_Document);
  }

  _INTR_INLINE static void initFromDescriptor(VertexLayoutRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_initFromDescriptor(p_Ref,
                                                            p_Properties);
  }

  _INTR_INLINE static void saveToSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_saveToSingleFile(p_FileName,
                                                          compileDescriptor);
  }

  _INTR_INLINE static void loadFromSingleFile(const char* p_FileName)
  {
    Dod::Resources::ResourceManagerBase<
        VertexLayoutData,
        _INTR_MAX_VERTEX_LAYOUT_COUNT>::_loadFromSingleFile(p_FileName,
                                                            initFromDescriptor,
                                                            resetToDefault);
  }

  // <-

  _INTR_INLINE static void createAllResources()
  {
    destroyResources(_activeRefs);
    createResources(_activeRefs);
  }

  static void createResources(const VertexLayoutRefArray& p_VertexLayouts);

  _INTR_INLINE static void
  destroyResources(const VertexLayoutRefArray& p_VertexLayouts)
  {
    for (uint32_t i = 0u; i < p_VertexLayouts.size(); ++i)
    {
      VertexLayoutRef vertexLayoutRef = p_VertexLayouts[i];

      _vkVertexInputAttributeDescs(vertexLayoutRef).clear();
      _vkVertexInputBindingDescs(vertexLayoutRef).clear();
    }
  }

  // Getter/Setter
  // Intrinsic

  _INTR_INLINE static _INTR_ARRAY(VertexAttribute) &
      _descVertexAttributes(VertexLayoutRef p_Ref)
  {
    return _data.descVertexAttributes[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(VertexBinding) &
      _descVertexBindings(VertexLayoutRef p_Ref)
  {
    return _data.descVertexBindings[p_Ref._id];
  }

  // GPU Resources
  _INTR_INLINE static _INTR_ARRAY(VkVertexInputAttributeDescription) &
      _vkVertexInputAttributeDescs(VertexLayoutRef p_Ref)
  {
    return _data.vkVertexInputAttributeDescs[p_Ref._id];
  }
  _INTR_INLINE static _INTR_ARRAY(VkVertexInputBindingDescription) &
      _vkVertexInputBindingDescs(VertexLayoutRef p_Ref)
  {
    return _data.vkVertexInputBindingDescs[p_Ref._id];
  }
  _INTR_INLINE static VkPipelineVertexInputStateCreateInfo&
  _vkPipelineVertexInputStateCreateInfo(VertexLayoutRef p_Ref)
  {
    return _data.vkPipelineVertexInputStateCreateInfo[p_Ref._id];
  }
};
}
}
}
}
