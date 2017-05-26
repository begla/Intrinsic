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
    _INTR_PROFILE_CPU("General", "Mesh Inst. Data Updt. Job");

    Dod::Ref frustumRef =
        Renderer::Vulkan::RenderProcess::Default::_activeFrustums[_frustumIdx];
    glm::mat4& viewMatrix =
        Resources::FrustumManager::_descViewMatrix(frustumRef);
    glm::mat4& viewProjectionMatrix =
        Resources::FrustumManager::_viewProjectionMatrix(frustumRef);

    for (uint32_t meshIdx = p_Range.start; meshIdx < p_Range.end; ++meshIdx)
    {
      MeshRef meshCompRef = Renderer::Vulkan::RenderProcess::Default::
          _visibleMeshComponents[_frustumIdx][meshIdx];
      Entity::EntityRef entityRef = MeshManager::_entity(meshCompRef);
      Components::NodeRef nodeRef =
          Components::NodeManager::getComponentForEntity(entityRef);

      const float distToCamera = glm::distance(
          Components::NodeManager::_worldPosition(nodeRef),
          Resources::FrustumManager::_frustumWorldPosition(frustumRef));

      // Fill per instance data
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
        perInstanceDataFragment.camParams.x =
            CameraManager::_descNearPlane(_camRef);
        perInstanceDataFragment.camParams.y =
            CameraManager::_descFarPlane(_camRef);
        perInstanceDataFragment.camParams.z =
            1.0f / perInstanceDataFragment.camParams.x;
        perInstanceDataFragment.camParams.w =
            1.0f / perInstanceDataFragment.camParams.y;

        perInstanceDataFragment.data0.x =
            Core::Resources::PostEffectManager::_descDayNightFactor(
                Core::Resources::PostEffectManager::_blendTargetRef);
        perInstanceDataFragment.data0.y = distToCamera;
        perInstanceDataFragment.data0.z = (float)nodeRef._id;
        perInstanceDataFragment.data0.w = TaskManager::_totalTimePassed;
        perInstanceDataFragment.colorTint =
            Components::MeshManager::_descColorTint(meshCompRef);

        // Modulate tint when entity is selected
        if (GameStates::Manager::getActiveGameState() ==
                GameStates::GameState::kEditing &&
            GameStates::Editing::_currentlySelectedEntity == entityRef)
        {
          perInstanceDataFragment.colorTint = glm::mix(
              perInstanceDataFragment.colorTint,
              glm::vec4(1.0f, 0.6f, 0.0f, 1.0f),
              glm::clamp(abs(sin(TaskManager::_totalTimePassed * 4.0f)) * 0.5f +
                             0.5f,
                         0.0f, 1.0f));
        }
      }
    }
  };

  uint32_t _frustumIdx;
  CameraRef _camRef;
} _perInstanceDataUpdateTaskSet;

// <-

struct UniformUpdateParallelTaskSet : enki::ITaskSet
{
  virtual ~UniformUpdateParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("General", "Mesh Uniform Data Updt. Job");

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
    _INTR_PROFILE_CPU("General", "Collect Visible Mesh Draw Calls Job");

    auto& drawCallsPerMaterialPass = Renderer::Vulkan::Resources::
        DrawCallManager::_drawCallsPerMaterialPass[_materialPassIdx];
    const uint32_t activeFrustumCount =
        (uint32_t)
            Renderer::Vulkan::RenderProcess::Default::_activeFrustums.size();

    for (uint32_t drawCallIdx = p_Range.start; drawCallIdx < p_Range.end;
         ++drawCallIdx)
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

        for (uint32_t frustIdx = 0u; frustIdx < activeFrustumCount; ++frustIdx)
        {
          auto& visibleDrawCallsPerMaterialPass =
              Renderer::Vulkan::RenderProcess::Default::
                  _visibleDrawCallsPerMaterialPass[frustIdx][_materialPassIdx];
          if ((Core::Components::NodeManager::_visibilityMask(
                   nodeComponentRef) &
               (1u << frustIdx)) > 0u)
          {
            Renderer::Vulkan::Resources::DrawCallManager::updateSortingHash(
                drawCallRef,
                Core::Components::MeshManager::_perInstanceDataVertex(
                    meshComponentRef)
                    .data0.y);

            visibleDrawCallsPerMaterialPass.push_back(drawCallRef);
          }
        }
      }
    }
  }

  uint8_t _materialPassIdx;
};

// <-

struct MeshCollectionParallelTaskSet : enki::ITaskSet
{
  virtual ~MeshCollectionParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("General", "Collect Visible Mesh Components Job");
    uint32_t activeFrustumsCount =
        (uint32_t)
            Renderer::Vulkan::RenderProcess::Default::_activeFrustums.size();

    for (uint32_t meshCompId = p_Range.start; meshCompId < p_Range.end;
         ++meshCompId)
    {
      Core::Components::MeshRef meshComponentRef =
          Core::Components::MeshManager::getActiveResourceAtIndex(meshCompId);
      Core::Components::NodeRef nodeComponentRef =
          Core::Components::MeshManager::_node(meshComponentRef);

      for (uint32_t frustIdx = 0u; frustIdx < activeFrustumsCount; ++frustIdx)
      {

        if ((Core::Components::NodeManager::_visibilityMask(nodeComponentRef) &
             (1u << frustIdx)) > 0u)
        {
          auto& visibleMeshComponents = Renderer::Vulkan::RenderProcess::
              Default::_visibleMeshComponents[frustIdx];
          visibleMeshComponents.push_back(meshComponentRef);
        }
      }
    }
  }
};
}

// <-

MeshData::MeshData()
    : Dod::Components::ComponentDataBase(_INTR_MAX_MESH_COMPONENT_COUNT)
{
  descMeshName.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  descColorTint.resize(_INTR_MAX_MESH_COMPONENT_COUNT);

  perInstanceDataVertex.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  perInstanceDataFragment.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  drawCalls.resize(_INTR_MAX_MESH_COMPONENT_COUNT);
  node.resize(_INTR_MAX_MESH_COMPONENT_COUNT);

  for (uint32_t i = 0u; i < _INTR_MAX_MESH_COMPONENT_COUNT; ++i)
  {
    drawCalls[i].resize(
        Renderer::Vulkan::Resources::MaterialManager::_materialPasses.size());
  }
}

// <-

void MeshManager::resetToDefault(MeshRef p_Mesh)
{
  _descMeshName(p_Mesh) = "";
  _descColorTint(p_Mesh) = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

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
          Renderer::Vulkan::Resources::MaterialManager::_materialPassMask(
              matToUse);

      for (uint32_t matPassIdx = 0u;
           matPassIdx <
           Renderer::Vulkan::Resources::MaterialManager::_materialPasses.size();
           ++matPassIdx)
      {
        if ((matPassMask & (1u << matPassIdx)) == 0u)
        {
          continue;
        }

        Renderer::Vulkan::Resources::DrawCallRef drawCallMesh =
            Renderer::Vulkan::Resources::DrawCallManager::createDrawCallForMesh(
                _N(_MeshComponent), meshRef, matToUse, matPassIdx,
                sizeof(MeshPerInstanceDataVertex),
                sizeof(MeshPerInstanceDataFragment), subMeshIdx);

        Renderer::Vulkan::Resources::DrawCallManager::_descMeshComponent(
            drawCallMesh) = meshCompRef;

        drawCallsToCreate.push_back(drawCallMesh);

        if (drawCalls.size() < matPassIdx + 1u)
        {
          drawCalls.resize(matPassIdx + 1u);
        }
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

void MeshManager::updatePerInstanceData(Dod::Ref p_CameraRef,
                                        uint32_t p_FrustumIdx)
{
  _INTR_PROFILE_CPU("General", "Update Per Instance Data");

  const uint32_t frustumId = Renderer::Vulkan::RenderProcess::Default::
                                 _cameraToIdMapping[p_CameraRef] +
                             p_FrustumIdx;
  _perInstanceDataUpdateTaskSet.m_SetSize =
      (uint32_t)Renderer::Vulkan::RenderProcess::Default::_visibleMeshComponents
          [frustumId]
              .size();
  _perInstanceDataUpdateTaskSet._frustumIdx = frustumId;
  _perInstanceDataUpdateTaskSet._camRef = p_CameraRef;

  Application::_scheduler.AddTaskSetToPipe(&_perInstanceDataUpdateTaskSet);
  Application::_scheduler.WaitforTaskSet(&_perInstanceDataUpdateTaskSet);
}

// <-

void MeshManager::updateUniformData(Dod::RefArray& p_DrawCalls)
{
  static UniformUpdateParallelTaskSet uniformUpdateTaskSet;

  _INTR_PROFILE_CPU("General", "Mesh Uniform Data Updt.");

  uniformUpdateTaskSet._drawCalls = &p_DrawCalls;
  uniformUpdateTaskSet.m_SetSize = (uint32_t)p_DrawCalls.size();

  Application::_scheduler.AddTaskSetToPipe(&uniformUpdateTaskSet);
  Application::_scheduler.WaitforTaskSet(&uniformUpdateTaskSet);
}

// <-

void MeshManager::collectDrawCallsAndMeshComponents()
{
  static MeshCollectionParallelTaskSet meshCollectionTaskSet;
  static _INTR_ARRAY(DrawCallCollectionParallelTaskSet)
      drawCallCollectionTaskSets;

  _INTR_PROFILE_CPU("General",
                    "Collect Visible Mesh Components And Draw Calls");

  using namespace Renderer;

  drawCallCollectionTaskSets.resize(
      Vulkan::Resources::MaterialManager::_materialPasses.size());

  for (uint32_t frustIdx = 0u;
       frustIdx < Vulkan::RenderProcess::Default::_activeFrustums.size();
       ++frustIdx)
  {
    for (uint32_t materialPassIdx = 0u;
         materialPassIdx <
         Vulkan::Resources::MaterialManager::_materialPasses.size();
         ++materialPassIdx)
    {
      Vulkan::RenderProcess::Default::_visibleDrawCallsPerMaterialPass
          [frustIdx][materialPassIdx]
              .clear();
    }

    Vulkan::RenderProcess::Default::_visibleMeshComponents[frustIdx].clear();
  }

  meshCollectionTaskSet.m_SetSize =
      Components::MeshManager::getActiveResourceCount();
  Application::_scheduler.AddTaskSetToPipe(&meshCollectionTaskSet);

  uint32_t currentJobIdx = 0u;
  for (uint32_t matPassIdx = 0u;
       matPassIdx < Vulkan::Resources::MaterialManager::_materialPasses.size();
       ++matPassIdx)
  {
    if (!Vulkan::Resources::DrawCallManager::_drawCallsPerMaterialPass
             [matPassIdx]
                 .empty())
    {
      DrawCallCollectionParallelTaskSet& drawCallTaskSet =
          drawCallCollectionTaskSets[currentJobIdx];
      drawCallTaskSet._materialPassIdx = matPassIdx;
      drawCallTaskSet.m_SetSize = (uint32_t)Vulkan::Resources::DrawCallManager::
                                      _drawCallsPerMaterialPass[matPassIdx]
                                          .size();

      Application::_scheduler.AddTaskSetToPipe(&drawCallTaskSet);
    }

    ++currentJobIdx;
  }

  // Wait for all
  for (uint32_t i = 0u; i < currentJobIdx; ++i)
  {
    Application::_scheduler.WaitforTaskSet(&drawCallCollectionTaskSets[i]);
  }

  Application::_scheduler.WaitforTaskSet(&meshCollectionTaskSet);
}
}
}
}
