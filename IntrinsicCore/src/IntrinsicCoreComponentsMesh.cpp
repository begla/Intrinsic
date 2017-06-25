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

// Precompiled header file
#include "stdafx.h"

using namespace RResources;

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
        R::RenderProcess::Default::_activeFrustums[_frustumIdx];
    glm::mat4& viewMatrix =
        Resources::FrustumManager::_descViewMatrix(frustumRef);
    glm::mat4& viewProjectionMatrix =
        Resources::FrustumManager::_viewProjectionMatrix(frustumRef);

    for (uint32_t meshIdx = p_Range.start; meshIdx < p_Range.end; ++meshIdx)
    {
      MeshRef meshCompRef =
          R::RenderProcess::Default::_visibleMeshComponents[_frustumIdx]
                                                           [meshIdx];
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
        perInstanceDataVertex.viewProjMatrix = viewProjectionMatrix;
        perInstanceDataVertex.worldViewProjMatrix =
            viewProjectionMatrix * perInstanceDataVertex.worldMatrix;
        perInstanceDataVertex.worldViewMatrix =
            viewMatrix * perInstanceDataVertex.worldMatrix;
        perInstanceDataVertex.viewMatrix = viewMatrix;
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

        perInstanceDataFragment.data0.x = World::_currentDayNightFactor;
        perInstanceDataFragment.data0.y = distToCamera;
        perInstanceDataFragment.data0.z = (float)nodeRef._id;
        perInstanceDataFragment.data0.w = TaskManager::_totalTimePassed;
        perInstanceDataFragment.colorTint =
            Components::MeshManager::_descColorTint(meshCompRef);
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

    DrawCallRefArray& drawCalls = *_drawCalls;

    for (uint32_t dcIdx = p_Range.start; dcIdx < p_Range.end; ++dcIdx)
    {
      DrawCallRef dcRef = drawCalls[dcIdx];
      MeshRef meshCompRef = DrawCallManager::_descMeshComponent(dcRef);
      _INTR_ASSERT(meshCompRef.isValid());

      MeshPerInstanceDataVertex& vertData =
          Components::MeshManager::_perInstanceDataVertex(meshCompRef);
      MeshPerInstanceDataFragment& fragData =
          Components::MeshManager::_perInstanceDataFragment(meshCompRef);

      DrawCallManager::allocateUniformMemory(dcRef);
      DrawCallManager::updateUniformMemory(
          dcRef, &vertData, sizeof(MeshPerInstanceDataVertex), &fragData,
          sizeof(MeshPerInstanceDataFragment));
    }
  }

  DrawCallRefArray* _drawCalls;
};

// <-

struct DrawCallCollectionParallelTaskSet : enki::ITaskSet
{
  virtual ~DrawCallCollectionParallelTaskSet() {}

  void ExecuteRange(enki::TaskSetPartition p_Range,
                    uint32_t p_ThreadNum) override
  {
    _INTR_PROFILE_CPU("General", "Collect Visible Mesh Draw Calls Job");

    auto& drawCallsPerMaterialPass =
        DrawCallManager::_drawCallsPerMaterialPass[_materialPassIdx];
    const uint32_t activeFrustumCount =
        (uint32_t)R::RenderProcess::Default::_activeFrustums.size();

    for (uint32_t frustIdx = 0u; frustIdx < activeFrustumCount; ++frustIdx)
    {
      auto& visibleDrawCallsPerMaterialPass = R::RenderProcess::Default::
          _visibleDrawCallsPerMaterialPass[frustIdx][_materialPassIdx];

      for (uint32_t drawCallIdx = p_Range.start; drawCallIdx < p_Range.end;
           ++drawCallIdx)
      {
        DrawCallRef drawCallRef = drawCallsPerMaterialPass[drawCallIdx];
        Components::MeshRef meshComponentRef =
            DrawCallManager::_descMeshComponent(drawCallRef);

        if (meshComponentRef.isValid())
        {
          Components::NodeRef nodeComponentRef =
              Components::MeshManager::_node(meshComponentRef);

          if ((Components::NodeManager::_visibilityMask(nodeComponentRef) &
               (1u << frustIdx)) > 0u)
          {
            DrawCallManager::updateSortingHash(
                drawCallRef, Components::MeshManager::_perInstanceDataVertex(
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
        (uint32_t)R::RenderProcess::Default::_activeFrustums.size();

    for (uint32_t meshCompId = p_Range.start; meshCompId < p_Range.end;
         ++meshCompId)
    {
      Components::MeshRef meshComponentRef =
          Components::MeshManager::getActiveResourceAtIndex(meshCompId);
      Components::NodeRef nodeComponentRef =
          Components::MeshManager::_node(meshComponentRef);

      for (uint32_t frustIdx = 0u; frustIdx < activeFrustumsCount; ++frustIdx)
      {

        if ((Components::NodeManager::_visibilityMask(nodeComponentRef) &
             (1u << frustIdx)) > 0u)
        {
          auto& visibleMeshComponents =
              R::RenderProcess::Default::_visibleMeshComponents[frustIdx];
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
    drawCalls[i].resize(MaterialManager::_materialPasses.size());
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
  DrawCallRefArray drawCallsToCreate;

  for (uint32_t meshIdx = 0u; meshIdx < p_Meshes.size(); ++meshIdx)
  {
    MeshRef meshCompRef = p_Meshes[meshIdx];
    NodeRef nodeRef = NodeManager::getComponentForEntity(_entity(meshCompRef));
    Name& meshName = _descMeshName(meshCompRef);
    DrawCallArray& drawCalls = _drawCalls(meshCompRef);

    Resources::MeshRef meshRef =
        Resources::MeshManager::getResourceByName(meshName);
    const uint32_t subMeshCount =
        (uint32_t)Resources::MeshManager::_descIndicesPerSubMesh(meshRef)
            .size();

    for (uint32_t subMeshIdx = 0u; subMeshIdx < subMeshCount; ++subMeshIdx)
    {
      MaterialRef matToUse = MaterialManager::getResourceByName(
          Resources::MeshManager::_descMaterialNamesPerSubMesh(
              meshRef)[subMeshIdx]);

      const uint32_t matPassMask = MaterialManager::_materialPassMask(matToUse);

      for (uint32_t matPassIdx = 0u;
           matPassIdx < MaterialManager::_materialPasses.size(); ++matPassIdx)
      {
        if ((matPassMask & (1u << matPassIdx)) == 0u)
        {
          continue;
        }

        DrawCallRef drawCallMesh = DrawCallManager::createDrawCallForMesh(
            _N(_MeshComponent), meshRef, matToUse, matPassIdx,
            sizeof(MeshPerInstanceDataVertex),
            sizeof(MeshPerInstanceDataFragment), subMeshIdx);

        DrawCallManager::_descMeshComponent(drawCallMesh) = meshCompRef;

        drawCallsToCreate.push_back(drawCallMesh);

        if (drawCalls.size() < matPassIdx + 1u)
        {
          drawCalls.resize(matPassIdx + 1u);
        }
        drawCalls[matPassIdx].push_back(drawCallMesh);
      }
    }

    // Update transform since the AABB most probably changed
    NodeManager::updateTransforms(nodeRef);

    // Create references
    {
      _node(meshCompRef) = nodeRef;
    }

    // Update dependent resources/components
    if ((World::_flags & WorldFlags::kLoadingUnloading) == 0u)
    {
      RigidBodyRef rigidBodyComp =
          RigidBodyManager::getComponentForEntity(_entity(meshCompRef));

      if (rigidBodyComp.isValid() &&
          RigidBodyManager::_pxRigidActor(rigidBodyComp) !=
              nullptr) // TODO: Better to handle checking of available resources
                       // in a general manner
      {
        RigidBodyManager::destroyResources(rigidBodyComp);
        RigidBodyManager::createResources(rigidBodyComp);
      }
    }
  }

  DrawCallManager::createResources(drawCallsToCreate);
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
        DrawCallRef dcRef = drawCallsPerMaterialPass[matPassIdx][dcIdx];
        dcsToDestroy.push_back(dcRef);
      }

      drawCallsPerMaterialPass[matPassIdx].clear();
    }
  }

  DrawCallManager::destroyDrawCallsAndResources(dcsToDestroy);
}

// <-

void MeshManager::updatePerInstanceData(Dod::Ref p_CameraRef,
                                        uint32_t p_FrustumIdx)
{
  _INTR_PROFILE_CPU("General", "Update Per Instance Data");

  const uint32_t frustumId =
      R::RenderProcess::Default::_cameraToIdMapping[p_CameraRef] + p_FrustumIdx;
  _perInstanceDataUpdateTaskSet.m_SetSize =
      (uint32_t)R::RenderProcess::Default::_visibleMeshComponents[frustumId]
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
      Renderer::Resources::MaterialManager::_materialPasses.size());

  for (uint32_t frustIdx = 0u;
       frustIdx < RenderProcess::Default::_activeFrustums.size(); ++frustIdx)
  {
    for (uint32_t materialPassIdx = 0u;
         materialPassIdx <
         Renderer::Resources::MaterialManager::_materialPasses.size();
         ++materialPassIdx)
    {
      RenderProcess::Default::_visibleDrawCallsPerMaterialPass[frustIdx]
                                                              [materialPassIdx]
                                                                  .clear();
    }

    RenderProcess::Default::_visibleMeshComponents[frustIdx].clear();
  }

  meshCollectionTaskSet.m_SetSize =
      Components::MeshManager::getActiveResourceCount();
  Application::_scheduler.AddTaskSetToPipe(&meshCollectionTaskSet);

  uint32_t currentJobIdx = 0u;
  for (uint32_t matPassIdx = 0u;
       matPassIdx <
       Renderer::Resources::MaterialManager::_materialPasses.size();
       ++matPassIdx)
  {
    if (!Renderer::Resources::DrawCallManager::_drawCallsPerMaterialPass
             [matPassIdx]
                 .empty())
    {
      DrawCallCollectionParallelTaskSet& drawCallTaskSet =
          drawCallCollectionTaskSets[currentJobIdx];
      drawCallTaskSet._materialPassIdx = matPassIdx;
      drawCallTaskSet.m_SetSize =
          (uint32_t)Renderer::Resources::DrawCallManager::
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
