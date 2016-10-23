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
namespace
{
struct PerInstanceDataUpdateParallelTaskSet : enki::ITaskSet
{
  virtual ~PerInstanceDataUpdateParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Components", "Mesh Inst. Data Updt.");

    Dod::Ref frustumRef =
        Renderer::Vulkan::DefaultRenderProcess::_activeFrustums[_frustumIdx];
    glm::mat4& viewMatrix =
        Resources::FrustumManager::_descViewMatrix(frustumRef);
    glm::mat4& viewProjectionMatrix =
        Resources::FrustumManager::_viewProjectionMatrix(frustumRef);
    Math::FrustumCorners& frustumCornersWS =
        Resources::FrustumManager::_frustumCornersWorldSpace(frustumRef);

    for (uint32_t meshIdx = p_Range.start; meshIdx < p_Range.end; ++meshIdx)
    {
      MeshRef meshCompRef =
          Renderer::Vulkan::RenderSystem::_visibleMeshComponents[_frustumIdx]
                                                                [meshIdx];
      Entity::EntityRef entityRef = MeshManager::_entity(meshCompRef);
      Components::NodeRef nodeRef =
          Components::NodeManager::getComponentForEntity(entityRef);

      const float distToCamera = glm::distance(
          Components::NodeManager::_worldPosition(nodeRef),
          Resources::FrustumManager::_frustumWorldPosition(frustumRef));

      // Fill per instance data and memcpy to buffer memory
      MeshPerInstanceDataVertex& perInstanceDataVertex =
          Components::MeshManager::_perInstanceDataVertex(meshCompRef);
      {
        perInstanceDataVertex.worldMatrix =
            Components::NodeManager::_worldMatrix(nodeRef);
        perInstanceDataVertex.normalMatrix =
            viewMatrix * Components::NodeManager::_normalMatrix(nodeRef);
        perInstanceDataVertex.worldViewProjMatrix =
            viewProjectionMatrix * perInstanceDataVertex.worldMatrix;
        perInstanceDataVertex.data0.w = TaskManager::_totalTimePassed;
        perInstanceDataVertex.data0.y = distToCamera;
      }

      MeshPerInstanceDataFragment& perInstanceDataFragment =
          Components::MeshManager::_perInstanceDataFragment(meshCompRef);
      {
        perInstanceDataFragment.data0.x = 1.0f; // Emissive factor
        perInstanceDataFragment.data0.y = distToCamera;
        perInstanceDataFragment.data0.z = (float)nodeRef._id;
        perInstanceDataFragment.data0.w = TaskManager::_totalTimePassed;

        // TODO: Move this to a more fitting place
        if (GameStates::Manager::getActiveGameState() ==
                GameStates::GameState::kEditing &&
            GameStates::Editing::_currentlySelectedEntity == entityRef)
        {
          perInstanceDataFragment.data0.x *=
              1.0f + abs(sin(TaskManager::_totalTimePassed * 2.0f));
        }
      }
    }
  };

  uint32_t _frustumIdx;
} _perInstanceDataUpdateTaskSet;

// <-

struct UniformUpdateParallelTaskSet : enki::ITaskSet
{
  virtual ~UniformUpdateParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Components", "Mesh Uniform Data Updt.");

    Renderer::Vulkan::Resources::DrawCallRefArray& drawCalls = *_drawCalls;

    for (uint32_t dcIdx = p_Range.start; dcIdx < p_Range.end; ++dcIdx)
    {
      Renderer::Vulkan::Resources::DrawCallRef dcRef = drawCalls[dcIdx];
      MeshRef meshCompRef =
          Renderer::Vulkan::Resources::DrawCallManager::_descMeshComponent(
              dcRef);
      _INTR_ASSERT(meshCompRef.isValid());

      MeshPerInstanceDataVertex& vertData =
          Components::MeshManager::_perInstanceDataVertex(meshCompRef);
      MeshPerInstanceDataFragment& fragData =
          Components::MeshManager::_perInstanceDataFragment(meshCompRef);

      Renderer::Vulkan::Resources::DrawCallManager::allocateUniformMemory(
          dcRef);
      Renderer::Vulkan::Resources::DrawCallManager::updateUniformMemory(
          dcRef, &vertData, sizeof(MeshPerInstanceDataVertex), &fragData,
          sizeof(MeshPerInstanceDataFragment));
    }
  }

  Renderer::Vulkan::Resources::DrawCallRefArray* _drawCalls;
};

// <-

struct DrawCallCollectionParallelTaskSet : enki::ITaskSet
{
  virtual ~DrawCallCollectionParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Components", "Collect Visible Mesh Draw Calls");

    auto& visibleDrawCallsPerMaterialPass = Renderer::Vulkan::RenderSystem::
        _visibleDrawCallsPerMaterialPass[_frustumIdx];
    auto& drawCallsPerMaterialPass = Renderer::Vulkan::Resources::
        DrawCallManager::_drawCallsPerMaterialPass[_materialPassIdx];

    for (uint32_t drawCallIdx = 0u;
         drawCallIdx < drawCallsPerMaterialPass.size(); ++drawCallIdx)
    {
      Renderer::Vulkan::Resources::DrawCallRef drawCallRef =
          drawCallsPerMaterialPass[drawCallIdx];
      Core::Components::MeshRef meshComponentRef =
          Renderer::Vulkan::Resources::DrawCallManager::_descMeshComponent(
              drawCallRef);

      if (meshComponentRef.isValid())
      {
        Core::Components::NodeRef nodeComponentRef =
            Core::Components::MeshManager::_node(meshComponentRef);

        if ((Core::Components::NodeManager::_visibilityMask(nodeComponentRef) &
             (1u << _frustumIdx)) > 0u)
        {
          Renderer::Vulkan::Resources::DrawCallManager::updateSortingHash(
              drawCallRef,
              Core::Components::MeshManager::_perInstanceDataVertex(
                  meshComponentRef)
                  .data0.y);
          visibleDrawCallsPerMaterialPass[_materialPassIdx].push_back(
              drawCallRef);
        }
      }
    }
  }

  uint32_t _frustumIdx;
  uint8_t _materialPassIdx;
};

// <-

struct MeshCollectionParallelTaskSet : enki::ITaskSet
{
  virtual ~MeshCollectionParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("Components", "Collect Visible Mesh Components");

    auto& visibleMeshComponentsPerMaterialPass = Renderer::Vulkan::
        RenderSystem::_visibleMeshComponentsPerMaterialPass[_frustumIdx];
    auto& visibleMeshComponents =
        Renderer::Vulkan::RenderSystem::_visibleMeshComponents[_frustumIdx];

    for (uint32_t meshCompId = 0u;
         meshCompId < Core::Components::MeshManager::getActiveResourceCount();
         ++meshCompId)
    {
      Core::Components::MeshRef meshComponentRef =
          Core::Components::MeshManager::getActiveResourceAtIndex(meshCompId);
      Core::Components::NodeRef nodeComponentRef =
          Core::Components::MeshManager::_node(meshComponentRef);

      if ((Core::Components::NodeManager::_visibilityMask(nodeComponentRef) &
           (1u << _frustumIdx)) > 0u)
      {
        if (Core::Components::MeshManager::_drawCalls(
                meshComponentRef)[_materialPassIdx]
                .size() > 0u)
        {
          visibleMeshComponentsPerMaterialPass[_materialPassIdx].push_back(
              meshComponentRef);
        }

        if (_materialPassIdx == 0u)
        {
          visibleMeshComponents.push_back(meshComponentRef);
        }
      }
    }
  }

  uint8_t _materialPassIdx;
  uint32_t _frustumIdx;
};
}

// <-

MeshData::MeshData()
    : Dod::Components::ComponentDataBase(_INTR_MAX_MESH_COMPONENT_COUNT)
{
  descMeshName.resize(_INTR_MAX_MESH_COMPONENT_COUNT);

  perInstanceDataVertex.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  perInstanceDataFragment.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  drawCalls.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  node.resize(_INTR_MAX_MESH_COMPONENT_COUNT);

  for (uint32_t i = 0u; i < _INTR_MAX_MESH_COMPONENT_COUNT; ++i)
  {
    drawCalls[i].resize(Renderer::Vulkan::MaterialPass::kCount);
  }
}

// <-

void MeshManager::resetToDefault(MeshRef p_Mesh) { _descMeshName(p_Mesh) = ""; }

void MeshManager::init()
{
  _INTR_LOG_INFO("Inititializing Mesh Component Manager...");

  Dod::Components::ComponentManagerBase<
      MeshData, _INTR_MAX_MESH_COMPONENT_COUNT>::_initComponentManager();

  Dod::Components::ComponentManagerEntry meshEntry;
  {
    meshEntry.createFunction = Components::MeshManager::createMesh;
    meshEntry.destroyFunction = Components::MeshManager::destroyMesh;
    meshEntry.createResourcesFunction =
        Components::MeshManager::createResources;
    meshEntry.destroyResourcesFunction =
        Components::MeshManager::destroyResources;
    meshEntry.getComponentForEntityFunction =
        Components::MeshManager::getComponentForEntity;
    meshEntry.resetToDefaultFunction = Components::MeshManager::resetToDefault;

    Application::_componentManagerMapping[_N(Mesh)] = meshEntry;
    Application::_orderedComponentManagers.push_back(meshEntry);
  }

  Dod::PropertyCompilerEntry propCompilerMesh;
  {
    propCompilerMesh.compileFunction =
        Components::MeshManager::compileDescriptor;
    propCompilerMesh.initFunction = Components::MeshManager::initFromDescriptor;
    propCompilerMesh.ref = Dod::Ref();

    Application::_componentPropertyCompilerMapping[_N(Mesh)] = propCompilerMesh;
  }
}

// <-

void MeshManager::createResources(const MeshRefArray& p_Meshes)
{
  Renderer::Vulkan::Resources::DrawCallRefArray drawCallsToCreate;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshCompRef = p_Meshes[meshIdx];
    Name& meshName = _descMeshName(meshCompRef);
    DrawCallArray& drawCalls = _drawCalls(meshCompRef);

    Resources::MeshRef meshRef =
        Resources::MeshManager::getResourceByName(meshName);
    const uint32_t subMeshCount =
        (uint32_t)Resources::MeshManager::_descIndicesPerSubMesh(meshRef)
            .size();

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      Renderer::Vulkan::Resources::MaterialRef matToUse =
          Renderer::Vulkan::Resources::MaterialManager::getResourceByName(
              Resources::MeshManager::_descMaterialNamesPerSubMesh(
                  meshRef)[subMeshIdx]);

      const uint32_t matPassMask =
          Renderer::Vulkan::Resources::MaterialManager::_descMaterialPassMask(
              matToUse);

      for (uint32_t matPassIdx = 0u;
           matPassIdx < Renderer::Vulkan::MaterialPass::kCount; ++matPassIdx)
      {
        if ((matPassMask & (1u << matPassIdx)) == 0u)
        {
          continue;
        }

        Renderer::Vulkan::Resources::DrawCallRef drawCallMesh =
            Renderer::Vulkan::Resources::DrawCallManager::createDrawCallForMesh(
                _N(_MeshComponent), meshRef, matToUse, matPassIdx,
                sizeof(MeshPerInstanceDataVertex),
                sizeof(MeshPerInstanceDataFragment));

        Renderer::Vulkan::Resources::DrawCallManager::_descMeshComponent(
            drawCallMesh) = meshCompRef;

        drawCallsToCreate.push_back(drawCallMesh);
        drawCalls[matPassIdx].push_back(drawCallMesh);
      }
    }

    // Create references
    {
      _node(meshCompRef) =
          NodeManager::getComponentForEntity(_entity(meshCompRef));
    }
  }

  Renderer::Vulkan::Resources::DrawCallManager::createResources(
      drawCallsToCreate);
}

// <-

void MeshManager::destroyResources(const MeshRefArray& p_Meshes)
{
  _INTR_ARRAY(Dod::Ref) dcsToDestroy;

  for (uint32_t mIdx = 0u; mIdx < p_Meshes.size(); ++mIdx)
  {
    MeshRef meshRef = p_Meshes[mIdx];
    DrawCallArray& drawCallsPerMaterialPass = _drawCalls(meshRef);

    _node(meshRef) = Dod::Ref();

    for (uint32_t matPassIdx = 0u; matPassIdx < drawCallsPerMaterialPass.size();
         ++matPassIdx)
    {
      for (uint32_t dcIdx = 0u;
           dcIdx < drawCallsPerMaterialPass[matPassIdx].size(); ++dcIdx)
      {
        Renderer::Vulkan::Resources::DrawCallRef dcRef =
            drawCallsPerMaterialPass[matPassIdx][dcIdx];
        dcsToDestroy.push_back(dcRef);
      }

      drawCallsPerMaterialPass[matPassIdx].clear();
    }
  }

  Renderer::Vulkan::Resources::DrawCallManager::destroyDrawCallsAndResources(
      dcsToDestroy);
}

// <-

void MeshManager::updatePerInstanceData(uint32_t p_FrustumIdx)
{
  _INTR_PROFILE_CPU("Components", "Updt. Per Instance Data");

  _perInstanceDataUpdateTaskSet.m_SetSize =
      (uint32_t)
          Renderer::Vulkan::RenderSystem::_visibleMeshComponents[p_FrustumIdx]
              .size();
  _perInstanceDataUpdateTaskSet._frustumIdx = p_FrustumIdx;

  Application::_scheduler.AddTaskSetToPipe(&_perInstanceDataUpdateTaskSet);
  Application::_scheduler.WaitforTaskSet(&_perInstanceDataUpdateTaskSet);
}

// <-

void MeshManager::updateUniformData(Dod::RefArray& p_DrawCalls)
{
  static UniformUpdateParallelTaskSet uniformUpdateTaskSet;

  _INTR_PROFILE_CPU("Components", "Queue Mesh Uniform Data Updt.");

  uniformUpdateTaskSet._drawCalls = &p_DrawCalls;
  uniformUpdateTaskSet.m_SetSize = (uint32_t)p_DrawCalls.size();

  Application::_scheduler.AddTaskSetToPipe(&uniformUpdateTaskSet);
  Application::_scheduler.WaitforTaskSet(&uniformUpdateTaskSet);
}

// <-

void MeshManager::collectDrawCallsAndMeshComponents()
{
  static _INTR_ARRAY(MeshCollectionParallelTaskSet) meshCollectionTaskSets;
  static _INTR_ARRAY(DrawCallCollectionParallelTaskSet)
      drawCallCollectionTaskSets;

  _INTR_PROFILE_CPU("Components",
                    "Collect Visible Mesh Components And Draw Calls");

  meshCollectionTaskSets.resize(Renderer::Vulkan::MaterialPass::kCount *
                                _INTR_MAX_FRUSTUMS_PER_FRAME_COUNT);
  drawCallCollectionTaskSets.resize(Renderer::Vulkan::MaterialPass::kCount *
                                    _INTR_MAX_FRUSTUMS_PER_FRAME_COUNT);

  for (uint32_t frustIdx = 0u;
       frustIdx <
       Renderer::Vulkan::DefaultRenderProcess::_activeFrustums.size();
       ++frustIdx)
  {
    for (uint32_t materialPassIdx = 0u;
         materialPassIdx < Renderer::Vulkan::MaterialPass::kCount;
         ++materialPassIdx)
    {
      Renderer::Vulkan::RenderSystem::_visibleDrawCallsPerMaterialPass
          [frustIdx][materialPassIdx]
              .reserve(_INTR_MAX_DRAW_CALL_COUNT);
      Renderer::Vulkan::RenderSystem::_visibleDrawCallsPerMaterialPass
          [frustIdx][materialPassIdx]
              .clear();

      Renderer::Vulkan::RenderSystem::_visibleMeshComponentsPerMaterialPass
          [frustIdx][materialPassIdx]
              .reserve(_INTR_MAX_DRAW_CALL_COUNT);
      Renderer::Vulkan::RenderSystem::_visibleMeshComponentsPerMaterialPass
          [frustIdx][materialPassIdx]
              .clear();
    }

    Renderer::Vulkan::RenderSystem::_visibleMeshComponents[frustIdx].reserve(
        _INTR_MAX_MESH_COMPONENT_COUNT);
    Renderer::Vulkan::RenderSystem::_visibleMeshComponents[frustIdx].clear();
  }

  uint32_t currentJobIdx = 0u;
  for (uint32_t frustIdx = 0u;
       frustIdx <
       Renderer::Vulkan::DefaultRenderProcess::_activeFrustums.size();
       ++frustIdx)
  {
    for (uint32_t matPassIdx = 0u;
         matPassIdx < Renderer::Vulkan::MaterialPass::kCount; ++matPassIdx)
    {
      MeshCollectionParallelTaskSet& meshTaskSet =
          meshCollectionTaskSets[currentJobIdx];
      meshTaskSet._frustumIdx = frustIdx;
      meshTaskSet._materialPassIdx = matPassIdx;
      meshTaskSet.m_SetSize = 1u;

      Application::_scheduler.AddTaskSetToPipe(&meshTaskSet);

      DrawCallCollectionParallelTaskSet& drawCallTaskSet =
          drawCallCollectionTaskSets[currentJobIdx];
      drawCallTaskSet._frustumIdx = frustIdx;
      drawCallTaskSet._materialPassIdx = matPassIdx;
      drawCallTaskSet.m_SetSize = 1u;

      Application::_scheduler.AddTaskSetToPipe(&drawCallTaskSet);

      ++currentJobIdx;
    }
  }

  // Wait for all
  for (uint32_t i = 0u; i < currentJobIdx; ++i)
  {
    Application::_scheduler.WaitforTaskSet(&drawCallCollectionTaskSets[i]);
    Application::_scheduler.WaitforTaskSet(&meshCollectionTaskSets[i]);
  }
}
}
}
}
