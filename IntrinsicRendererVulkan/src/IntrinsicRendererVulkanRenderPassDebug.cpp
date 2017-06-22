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
#include "stdafx_vulkan.h"
#include "stdafx.h"

#define MAX_LINE_COUNT 256000

using namespace RVResources;

namespace Intrinsic
{
namespace Renderer
{
namespace Vulkan
{
namespace RenderPass
{
namespace
{
struct DebugLineVertex
{
  glm::vec3 pos;
  uint32_t color;
};

struct PerInstanceDataDebugLineVertex
{
  glm::mat4 worldViewProjMatrix;
  glm::mat4 normalMatrix;
};

ImageRef _albedoImageRef;
ImageRef _normalImageRef;
ImageRef _parameter0ImageRef;
ImageRef _depthImageRef;
FramebufferRef _framebufferRef;

PipelineLayoutRef _debugLinePipelineLayout;
PipelineRef _debugLinePipelineRef;
DrawCallRef _debugLineDrawCallRef;
BufferRef _debugLineVertexBufferRef;

RenderPassRef _renderPassRef;

DebugLineVertex* _mappedLineMemory = nullptr;
uint32_t _currentLineVertexCount = 0u;

rapidjson::Document _benchmarkDesc;
_INTR_ARRAY(GameStates::Benchmark::Path) _benchmarkPaths;
bool _parseBenchmark = true;

void displayWorldBoundingSpheres()
{
  for (uint32_t i = 0u; i < Components::NodeManager::_activeRefs.size(); ++i)
  {
    Components::NodeRef nodeRef = Components::NodeManager::_activeRefs[i];

    if ((Components::NodeManager::_visibilityMask(nodeRef) & 1u) > 0u)
    {
      Math::Sphere& worldBoundingSphere =
          Components::NodeManager::_worldBoundingSphere(nodeRef);
      Debug::renderSphere(worldBoundingSphere.p, worldBoundingSphere.r,
                          glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }
}

// <-

void displayBenchmarkPaths()
{
  if (_parseBenchmark)
  {
    GameStates::Benchmark::parseBenchmark(_benchmarkDesc);
    _parseBenchmark = false;
  }

  _benchmarkPaths.clear();
  GameStates::Benchmark::assembleBenchmarkPaths(_benchmarkDesc,
                                                _benchmarkPaths);

  for (uint32_t i = 0u; i < _benchmarkPaths.size(); ++i)
  {
    static const float stepSize = 0.01f;

    _INTR_ARRAY(glm::vec3) curvePositions;
    curvePositions.reserve((uint32_t)(1.0f / 0.01f));

    for (float perc = 0.0f; perc < 1.0f; perc += stepSize)
    {
      curvePositions.push_back(
          Math::bezierQuadratic(_benchmarkPaths[i].nodePositions, perc));
    }

    if (curvePositions.size() > 1u)
    {
      for (uint32_t j = 0u; j < curvePositions.size() - 1u; ++j)
      {
        Debug::renderLine(curvePositions[j], curvePositions[j + 1],
                          glm::vec3(0.0f, 1.0f, 1.0f),
                          glm::vec3(0.0f, 1.0f, 1.0f));
      }
    }
  }
}

// <-

void displayDebugLineGeometryForSelectedObject()
{
  if (GameStates::Editing::_currentlySelectedEntity.isValid())
  {
    Components::NodeRef nodeRef =
        Components::NodeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (!nodeRef.isValid())
    {
      return;
    }

    Components::PostEffectVolumeRef postVolumeRef =
        Components::PostEffectVolumeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (postVolumeRef.isValid())
    {
      Debug::renderSphere(
          Components::NodeManager::_worldPosition(nodeRef),
          Components::PostEffectVolumeManager::_descRadius(postVolumeRef),
          glm::vec3(0.0f, 0.0f, 1.0f));
      Debug::renderSphere(
          Components::NodeManager::_worldPosition(nodeRef),
          Components::PostEffectVolumeManager::_descRadius(postVolumeRef) -
              Components::PostEffectVolumeManager::_descBlendRange(
                  postVolumeRef),
          glm::vec3(0.0f, 1.0f, 1.0f));
    }

    Components::LightRef lightRef =
        Components::LightManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (lightRef.isValid())
    {
      Debug::renderSphere(
          Components::NodeManager::_worldPosition(nodeRef),
          Components::LightManager::_descRadius(lightRef),
          glm::vec3(Components::LightManager::_descColor(lightRef)));
    }

    Components::IrradianceProbeRef irradProbeRef =
        Components::IrradianceProbeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (irradProbeRef.isValid())
    {
      Debug::renderSphere(
          Components::NodeManager::_worldPosition(nodeRef),
          Components::IrradianceProbeManager::_descRadius(irradProbeRef),
          glm::vec3(1.0f, 0.0f, 0.0f));
    }

    Components::SpecularProbeRef specProbeRef =
        Components::SpecularProbeManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (specProbeRef.isValid())
    {
      Debug::renderSphere(
          Components::NodeManager::_worldPosition(nodeRef),
          Components::SpecularProbeManager::_descRadius(specProbeRef),
          glm::vec3(1.0f, 0.0f, 0.0f));
    }

    Components::DecalRef decalRef =
        Components::DecalManager::getComponentForEntity(
            GameStates::Editing::_currentlySelectedEntity);

    if (decalRef.isValid())
    {
      Debug::renderDecal(decalRef);
    }
  }
}
}

// Static members
uint32_t Debug::_activeDebugStageFlags = DebugStageFlags::kSelectedObject;

void Debug::init()
{
  RenderPassRefArray renderpassesToCreate;
  PipelineLayoutRefArray pipelineLayoutsToCreate;
  BufferRefArray buffersToCreate;

  // Pipeline layouts
  {
    _debugLinePipelineLayout =
        PipelineLayoutManager::createPipelineLayout(_N(DebugLineVertex));
    PipelineLayoutManager::resetToDefault(_debugLinePipelineLayout);

    GpuProgramManager::reflectPipelineLayout(
        8u,
        {GpuProgramManager::getResourceByName("debug_line.vert"),
         GpuProgramManager::getResourceByName("debug_line.frag")},
        _debugLinePipelineLayout);

    pipelineLayoutsToCreate.push_back(_debugLinePipelineLayout);
  }

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(Debug));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription albedoAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription normalAttachment = {Format::kR16G16B16A16Float, 0u};
    AttachmentDescription parameter0Attachment = {Format::kR16G16B16A16Float,
                                                  0u};
    AttachmentDescription depthStencilAttachment = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse, 0u};

    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(albedoAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(normalAttachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(parameter0Attachment);
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(depthStencilAttachment);
  }
  renderpassesToCreate.push_back(_renderPassRef);

  {
    BufferRef& lineVertexBuffer = _debugLineVertexBufferRef;
    lineVertexBuffer = BufferManager::createBuffer(_N(LineDebug));

    BufferManager::resetToDefault(lineVertexBuffer);

    BufferManager::addResourceFlags(
        lineVertexBuffer, Dod::Resources::ResourceFlags::kResourceVolatile);
    BufferManager::_descBufferType(lineVertexBuffer) = BufferType::kVertex;
    BufferManager::_descSizeInBytes(lineVertexBuffer) =
        MAX_LINE_COUNT * sizeof(DebugLineVertex) * 2u;
    BufferManager::_descMemoryPoolType(lineVertexBuffer) =
        MemoryPoolType::kStaticStagingBuffers;

    buffersToCreate.push_back(lineVertexBuffer);
  }

  PipelineLayoutManager::createResources(pipelineLayoutsToCreate);
  RenderPassManager::createResources(renderpassesToCreate);
  BufferManager::createResources(buffersToCreate);

  // Map line buffer memory
  _mappedLineMemory =
      (DebugLineVertex*)BufferManager::getGpuMemory(_debugLineVertexBufferRef);
}

// <-

void Debug::onReinitRendering()
{
  FramebufferRefArray framebuffersToDestroy;
  FramebufferRefArray framebuffersToCreate;
  PipelineRefArray pipelinesToDestroy;
  PipelineRefArray pipelinesToCreate;
  DrawCallRefArray drawCallsToDestroy;
  DrawCallRefArray drawCallsToCreate;

  // Cleanup old resources
  {
    if (_framebufferRef.isValid())
      framebuffersToDestroy.push_back(_framebufferRef);
    if (_debugLinePipelineRef.isValid())
      pipelinesToDestroy.push_back(_debugLinePipelineRef);
    if (_debugLineDrawCallRef.isValid())
      drawCallsToDestroy.push_back(_debugLineDrawCallRef);

    DrawCallManager::destroyDrawCallsAndResources(drawCallsToDestroy);
    FramebufferManager::destroyFramebuffersAndResources(framebuffersToDestroy);
    PipelineManager::destroyPipelinesAndResources(pipelinesToDestroy);
  }

  // Line debugging
  {
    PipelineRef& pipeline = _debugLinePipelineRef;
    pipeline = PipelineManager::createPipeline(_N(DebugLineVertex));
    PipelineManager::resetToDefault(pipeline);

    PipelineManager::_descFragmentProgram(pipeline) =
        GpuProgramManager::getResourceByName("debug_line.frag");
    PipelineManager::_descVertexProgram(pipeline) =
        GpuProgramManager::getResourceByName("debug_line.vert");
    PipelineManager::_descRenderPass(pipeline) = _renderPassRef;
    PipelineManager::_descPipelineLayout(pipeline) = _debugLinePipelineLayout;
    PipelineManager::_descVertexLayout(pipeline) =
        VertexLayoutManager::getResourceByName(_N(DebugLineVertex));
    PipelineManager::_descInputAssemblyState(pipeline) =
        InputAssemblyStates::kLineList;

    PipelineManager::_descBlendStates(pipeline).clear();
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);
    PipelineManager::_descBlendStates(pipeline).push_back(
        BlendStates::kDefault);

    pipelinesToCreate.push_back(pipeline);
  }

  {
    DrawCallRef& drawCallLine = _debugLineDrawCallRef;
    drawCallLine = DrawCallManager::createDrawCall(_N(DebugLineVertex));
    DrawCallManager::resetToDefault(drawCallLine);
    DrawCallManager::addResourceFlags(
        drawCallLine, Dod::Resources::ResourceFlags::kResourceVolatile);

    DrawCallManager::_descPipeline(drawCallLine) = _debugLinePipelineRef;
    DrawCallManager::_descVertexBuffers(drawCallLine)
        .push_back(_debugLineVertexBufferRef);
    DrawCallManager::_descIndexBuffer(drawCallLine) = Dod::Ref();
    DrawCallManager::_descVertexCount(drawCallLine) = 0u;
    DrawCallManager::_descIndexCount(drawCallLine) = 0u;

    DrawCallManager::bindBuffer(
        drawCallLine, _N(PerInstance), GpuProgramType::kVertex,
        UniformManager::_perInstanceUniformBuffer, UboType::kPerInstanceVertex,
        sizeof(PerInstanceDataDebugLineVertex));

    drawCallsToCreate.push_back(drawCallLine);
  }

  PipelineManager::createResources(pipelinesToCreate);
  DrawCallManager::createResources(drawCallsToCreate);

  _albedoImageRef = ImageManager::getResourceByName(_N(GBufferAlbedo));
  _normalImageRef = ImageManager::getResourceByName(_N(GBufferNormal));
  _parameter0ImageRef = ImageManager::getResourceByName(_N(GBufferParameter0));
  _depthImageRef = ImageManager::getResourceByName(_N(GBufferDepth));

  glm::uvec2 dim = glm::uvec2(RenderSystem::_backbufferDimensions.x,
                              RenderSystem::_backbufferDimensions.y);

  // Create framebuffer
  _framebufferRef = FramebufferManager::createFramebuffer(_N(Debug));
  {
    FramebufferManager::resetToDefault(_framebufferRef);
    FramebufferManager::addResourceFlags(
        _framebufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_albedoImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_normalImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_parameter0ImageRef);
    FramebufferManager::_descAttachedImages(_framebufferRef)
        .push_back(_depthImageRef);
    FramebufferManager::_descDimensions(_framebufferRef) = dim;
    FramebufferManager::_descRenderPass(_framebufferRef) = _renderPassRef;
  }
  framebuffersToCreate.push_back(_framebufferRef);

  FramebufferManager::createResources(framebuffersToCreate);
}

// <-

void Debug::destroy() {}

// <-

void Debug::renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                       const glm::vec3& p_Color0, const glm::vec3& p_Color1)
{
  if (_currentLineVertexCount >= MAX_LINE_COUNT * 2u ||
      (GameStates::Manager::getActiveGameState() !=
       GameStates::GameState::kEditing))
  {
    return;
  }

  DebugLineVertex& lineVertex0 = _mappedLineMemory[_currentLineVertexCount++];
  lineVertex0.pos = p_Pos0;
  lineVertex0.color = Math::convertColorToBGRA(glm::vec4(p_Color0, 1.0f));

  DebugLineVertex& lineVertex1 = _mappedLineMemory[_currentLineVertexCount++];
  lineVertex1.pos = p_Pos1;
  lineVertex1.color = Math::convertColorToBGRA(glm::vec4(p_Color1, 1.0f));
}

// <-

void Debug::renderLine(const glm::vec3& p_Pos0, const glm::vec3& p_Pos1,
                       uint32_t p_Color0, uint32_t p_Color1)
{
  if (_currentLineVertexCount >= MAX_LINE_COUNT * 2u ||
      (GameStates::Manager::getActiveGameState() !=
       GameStates::GameState::kEditing))
  {
    return;
  }

  DebugLineVertex& lineVertex0 = _mappedLineMemory[_currentLineVertexCount++];
  lineVertex0.pos = p_Pos0;
  lineVertex0.color = p_Color0;

  DebugLineVertex& lineVertex1 = _mappedLineMemory[_currentLineVertexCount++];
  lineVertex1.pos = p_Pos1;
  lineVertex1.color = p_Color1;
}

// <-

void Debug::renderSphere(const glm::vec3& p_Center, float p_Radius,
                         const glm::vec3& p_Color)
{
  if (GameStates::Manager::getActiveGameState() !=
      GameStates::GameState::kEditing)
  {
    return;
  }

  static const uint32_t segments = 16u;
  static const float stepSize = glm::two_pi<float>() / segments;

  for (uint32_t i = 0u; i < segments; ++i)
  {
    const float x0 = sin(i * stepSize);
    const float x1 = sin((i + 1u) * stepSize);

    const float y0 = cos(i * stepSize);
    const float y1 = cos((i + 1u) * stepSize);

    renderLine(p_Center + p_Radius * glm::vec3(x0, y0, 0.0f),
               p_Center + p_Radius * glm::vec3(x1, y1, 0.0f), p_Color, p_Color);
    renderLine(p_Center + p_Radius * glm::vec3(0.0f, x0, y0),
               p_Center + p_Radius * glm::vec3(0.0f, x1, y1), p_Color, p_Color);
    renderLine(p_Center + p_Radius * glm::vec3(x0, 0.0f, y0),
               p_Center + p_Radius * glm::vec3(x1, 0.0f, y1), p_Color, p_Color);
  }
}

// <-

void Debug::renderDecal(Dod::Ref p_Decal)
{

  const Components::NodeRef nodeRef =
      Components::NodeManager::getComponentForEntity(
          Components::DecalManager::_entity(p_Decal));
  const glm::vec3 halfExtent =
      Components::DecalManager::_descHalfExtent(p_Decal) *
      Components::NodeManager::_worldSize(nodeRef);

  const glm::quat orient = Components::NodeManager::_worldOrientation(nodeRef);
  const glm::vec3 right = orient * glm::vec3(halfExtent.x, 0.0f, 0.0f);
  const glm::vec3 forward =
      orient * glm::vec3(0.0f, 0.0f, -halfExtent.z * 2.0f);
  const glm::vec3 up = orient * glm::vec3(0.0f, halfExtent.y, 0.0f);

  glm::vec3 verts[8] = {
      forward + right + up,   forward + right - up, forward - (right + up),
      forward - (right - up), right + up,           right - up,
      -(right + up),          -(right - up)};

  // Move to world space
  for (uint32_t i = 0u; i < 8u; ++i)
    verts[i] += Components::NodeManager::_worldPosition(nodeRef);

  static const glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
  for (uint32_t i = 0u; i < 4u; ++i)
  {
    // Front
    renderLine(verts[i], verts[(i + 1u) % 4u], color, color);
    // Back
    renderLine(verts[4u + i], verts[4u + (i + 1u) % 4u], color, color);
    // Front-Back connections
    renderLine(verts[i], verts[i + 4u], color, color);
  }
}

// <-

void Debug::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Render Debug Geometry");
  _INTR_PROFILE_GPU("Render Debug Geometry");

  static DrawCallRefArray visibleDrawCalls;
  static DrawCallRefArray visibleMeshDrawCalls;
  visibleMeshDrawCalls.clear();
  visibleDrawCalls.clear();

  if (GameStates::Manager::getActiveGameState() !=
      GameStates::GameState::kEditing)
  {
    return;
  }

  GameStates::Editing::findVisibleEditingDrawCalls(visibleDrawCalls);
  GameStates::Editing::updatePerInstanceData();

  // Debug rendering stages
  {
    if ((_activeDebugStageFlags & DebugStageFlags::kWorldBoundingSpheres) > 0u)
    {
      displayWorldBoundingSpheres();
    }
    if ((_activeDebugStageFlags & DebugStageFlags::kBenchmarkPaths) > 0u)
    {
      displayBenchmarkPaths();
    }
    else
    {
      _parseBenchmark = true;
    }

    if ((_activeDebugStageFlags & DebugStageFlags::kSelectedObject) > 0u)
      displayDebugLineGeometryForSelectedObject();
  }

  // Line debug rendering
  if (_currentLineVertexCount > 0u)
  {
    // Update line per instance data
    {
      PerInstanceDataDebugLineVertex perInstanceDataVertex;
      {
        perInstanceDataVertex.worldViewProjMatrix =
            Components::CameraManager::_viewProjectionMatrix(p_CameraRef);
        perInstanceDataVertex.normalMatrix =
            Components::CameraManager::_viewMatrix(p_CameraRef);
      }

      // Write to GPU memory
      DrawCallManager::allocateAndUpdateUniformMemory(
          {_debugLineDrawCallRef}, &perInstanceDataVertex,
          sizeof(PerInstanceDataDebugLineVertex), nullptr, 0u);
    }

    // Update line count
    {
      DrawCallManager::_descVertexCount(_debugLineDrawCallRef) =
          _currentLineVertexCount;
    }

    visibleDrawCalls.push_back(_debugLineDrawCallRef);
  }

  if ((_activeDebugStageFlags & DebugStageFlags::kWireframeRendering) > 0u ||
      GameStates::Editing::_currentlySelectedEntity.isValid())
  {
    RenderProcess::Default::getVisibleDrawCalls(
        p_CameraRef, 0u,
        MaterialManager::getMaterialPassId(_N(GBufferWireframe)))
        .copy(visibleMeshDrawCalls);
    // Update per mesh uniform data
    CComponents::MeshManager::updateUniformData(visibleMeshDrawCalls);

    if ((_activeDebugStageFlags & DebugStageFlags::kWireframeRendering) > 0u)
    {
      visibleDrawCalls.insert(visibleDrawCalls.end(),
                              visibleMeshDrawCalls.begin(),
                              visibleMeshDrawCalls.end());
    }
    else
    {
      for (uint32_t i = 0u; i < visibleMeshDrawCalls.size(); ++i)
      {
        DrawCallRef dcRef = visibleMeshDrawCalls[i];
        Entity::EntityRef dcEntity = Components::MeshManager::_entity(
            DrawCallManager::_descMeshComponent(dcRef));

        if (dcEntity == GameStates::Editing::_currentlySelectedEntity)
        {
          visibleDrawCalls.push_back(dcRef);
        }
      }
    }
  }

  RenderSystem::beginRenderPass(_renderPassRef, _framebufferRef,
                                VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
  {
    DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                       _framebufferRef);
    _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (Debug)",
                              DrawCallDispatcher::_dispatchedDrawCallCount);
  }
  RenderSystem::endRenderPass(_renderPassRef);

  // Reset the current line count to zero
  _currentLineVertexCount = 0u;
}
}
}
}
}
