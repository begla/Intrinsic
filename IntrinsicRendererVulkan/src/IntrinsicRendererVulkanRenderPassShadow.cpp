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
#include "stdafx_vulkan.h"
#include "stdafx.h"

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
Resources::ImageRef _shadowBufferImageRef;
_INTR_ARRAY(Resources::FramebufferRef) _framebufferRefs;
Resources::RenderPassRef _renderPassRef;

// <-

_INTR_INLINE void
calculateFrustumForSplit(uint32_t p_SplitIdx,
                         Core::Resources::FrustumRef p_FrustumRef,
                         Components::CameraRef p_CameraRef)
{
  _INTR_PROFILE_CPU("Render Pass", "Calc. Shadow Map Matrices");

  // Make this configurable
  const bool lastSplit = p_SplitIdx == _INTR_PSSM_SPLIT_COUNT - 1u;
  const float maxShadowDistance = 3000.0f;
  const float splitDistance = 5.0f;

  // TODO: Replace this with an actual scene AABB
  Math::AABB worldBounds = {glm::vec3(-5000.0f, -5000.0f, -5000.0f),
                            glm::vec3(5000.0f, 5000.0f, 5000.0f)};

  const glm::vec3 worldBoundsHalfExtent = Math::calcAABBHalfExtent(worldBounds);
  const float worldBoundsHalfExtentLength = glm::length(worldBoundsHalfExtent);
  const glm::vec3 worldBoundsCenter = Math::calcAABBCenter(worldBounds);

  const glm::vec3 eye =
      worldBoundsHalfExtentLength *
      (Core::Resources::PostEffectManager::calcActualMainLightOrientation(
           Core::Resources::PostEffectManager::_blendTargetRef) *
       glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);

  glm::mat4& shadowViewMatrix =
      Core::Resources::FrustumManager::_descViewMatrix(p_FrustumRef);
  shadowViewMatrix = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));

  const float nearPlane =
      glm::max(glm::pow(splitDistance * p_SplitIdx, 2.0f), 0.1f);
  const float farPlane = !lastSplit
                             ? glm::pow(splitDistance * (p_SplitIdx + 1u), 2.0f)
                             : maxShadowDistance;

  const glm::mat4 inverseProj =
      glm::inverse(Components::CameraManager::computeCustomProjMatrix(
          p_CameraRef, nearPlane, farPlane));

  Math::FrustumCorners viewSpaceCorners;
  Math::extractFrustumsCorners(inverseProj, viewSpaceCorners);

  glm::vec3 fpMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
  glm::vec3 fpMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

  for (uint32_t cornerIdx = 0; cornerIdx < 8u; ++cornerIdx)
  {
    const glm::vec3& fp = viewSpaceCorners.c[cornerIdx];

    fpMin = Math::calcVecMin(fpMin, fp);
    fpMax = Math::calcVecMax(fpMax, fp);
  }

  const glm::vec3 boundingSphereCenter = (fpMin + fpMax) * 0.5f;
  const glm::mat4 viewToShadowView =
      shadowViewMatrix *
      Components::CameraManager::_inverseViewMatrix(p_CameraRef);
  const glm::vec3 boundingSphereCenterShadowSpace =
      glm::vec3(viewToShadowView * glm::vec4(boundingSphereCenter, 1.0f));
  const float boundingSphereRadius = glm::length(fpMax - fpMin) * 0.5f;

  fpMin = boundingSphereCenterShadowSpace - boundingSphereRadius;
  fpMax = boundingSphereCenterShadowSpace + boundingSphereRadius;

  // Snap to texel increments
  {
    const glm::vec2 worldUnitsPerTexel =
        glm::vec2(fpMax - fpMin) / glm::vec2(Shadow::_shadowMapSize);

    fpMin.x /= worldUnitsPerTexel.x;
    fpMin.y /= worldUnitsPerTexel.y;
    fpMin.x = floor(fpMin.x);
    fpMin.y = floor(fpMin.y);
    fpMin.x *= worldUnitsPerTexel.x;
    fpMin.y *= worldUnitsPerTexel.y;

    fpMax.x /= worldUnitsPerTexel.x;
    fpMax.y /= worldUnitsPerTexel.y;
    fpMax.x = floor(fpMax.x);
    fpMax.y = floor(fpMax.y);
    fpMax.x *= worldUnitsPerTexel.x;
    fpMax.y *= worldUnitsPerTexel.y;
  }

  const float orthoLeft = fpMin.x;
  const float orthoRight = fpMax.x;
  const float orthoBottom = fpMax.y;
  const float orthoTop = fpMin.y;
  float orthoNear = FLT_MAX;
  float orthoFar = -FLT_MAX;

  // Calc. near/fear
  {
    glm::vec3 aabbCorners[8];
    Math::calcAABBCorners(worldBounds, aabbCorners);

    for (uint32_t i = 0u; i < 8; ++i)
    {
      aabbCorners[i] =
          glm::vec3(shadowViewMatrix * glm::vec4(aabbCorners[i], 1.0));

      orthoNear = glm::min(orthoNear, -aabbCorners[i].z);
      orthoFar = glm::max(orthoFar, -aabbCorners[i].z);
    }
  }

  Core::Resources::FrustumManager::_descProjectionType(p_FrustumRef) =
      Core::Resources::ProjectionType::kOrthographic;
  Core::Resources::FrustumManager::_descNearFarPlaneDistances(p_FrustumRef) =
      glm::vec2(orthoNear, orthoFar);
  Core::Resources::FrustumManager::_descProjectionMatrix(p_FrustumRef) =
      glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear,
                 orthoFar);
}
}

// <-

// Static members
glm::uvec2 Shadow::_shadowMapSize = glm::uvec2(1536u, 1536u);

// <-

void Shadow::init()
{
  using namespace Resources;

  RenderPassRefArray renderPassesToCreate;
  ImageRefArray imagesToCreate;

  // Render passes
  {
    _renderPassRef = RenderPassManager::createRenderPass(_N(Shadow));
    RenderPassManager::resetToDefault(_renderPassRef);

    AttachmentDescription shadowBufferAttachment = {
        (uint8_t)RenderSystem::_depthStencilFormatToUse,
        AttachmentFlags::kClearOnLoad | AttachmentFlags::kClearStencilOnLoad};
    RenderPassManager::_descAttachments(_renderPassRef)
        .push_back(shadowBufferAttachment);
  }
  renderPassesToCreate.push_back(_renderPassRef);

  RenderPassManager::createResources(renderPassesToCreate);

  glm::uvec3 dim = glm::uvec3(_shadowMapSize, 1u);

  // Create images
  _shadowBufferImageRef = ImageManager::createImage(_N(ShadowBuffer));
  {
    ImageManager::resetToDefault(_shadowBufferImageRef);
    ImageManager::addResourceFlags(
        _shadowBufferImageRef,
        Dod::Resources::ResourceFlags::kResourceVolatile);

    ImageManager::_descDimensions(_shadowBufferImageRef) = dim;
    ImageManager::_descImageFormat(_shadowBufferImageRef) =
        RenderSystem::_depthStencilFormatToUse;
    ImageManager::_descImageType(_shadowBufferImageRef) = ImageType::kTexture;
    ImageManager::_descArrayLayerCount(_shadowBufferImageRef) =
        _INTR_MAX_SHADOW_MAP_COUNT;
  }
  imagesToCreate.push_back(_shadowBufferImageRef);

  // Create framebuffers
  for (uint32_t shadowMapIdx = 0u; shadowMapIdx < _INTR_MAX_SHADOW_MAP_COUNT;
       ++shadowMapIdx)
  {
    FramebufferRef frameBufferRef =
        FramebufferManager::createFramebuffer(_N(RenderPassShadow));
    {
      FramebufferManager::resetToDefault(frameBufferRef);
      FramebufferManager::addResourceFlags(
          frameBufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

      FramebufferManager::_descAttachedImages(frameBufferRef)
          .push_back(AttachmentInfo(_shadowBufferImageRef, shadowMapIdx));
      FramebufferManager::_descDimensions(frameBufferRef) = glm::uvec2(dim);
      FramebufferManager::_descRenderPass(frameBufferRef) = _renderPassRef;
    }
    _framebufferRefs.push_back(frameBufferRef);
  }

  ImageManager::createResources(imagesToCreate);
  FramebufferManager::createResources(_framebufferRefs);
}

// <-

void Shadow::onReinitRendering() {}

// <-

void Shadow::destroy() {}

// <-

void Shadow::prepareFrustums(Components::CameraRef p_CameraRef,
                             _INTR_ARRAY(Core::Resources::FrustumRef) &
                                 p_ShadowFrustums)
{
  _INTR_PROFILE_CPU("Render Pass", "Prepare Shadow Frustums");

  for (uint32_t i = 0u; i < p_ShadowFrustums.size(); ++i)
  {
    Core::Resources::FrustumManager::destroyFrustum(p_ShadowFrustums[i]);
  }
  p_ShadowFrustums.clear();

  for (uint32_t shadowMapIdx = 0u; shadowMapIdx < _INTR_PSSM_SPLIT_COUNT;
       ++shadowMapIdx)
  {
    Core::Resources::FrustumRef frustumRef =
        Core::Resources::FrustumManager::createFrustum(_N(ShadowFrustum));

    calculateFrustumForSplit(shadowMapIdx, frustumRef, p_CameraRef);

    p_ShadowFrustums.push_back(frustumRef);
  }
}

// <-

void Shadow::render(float p_DeltaT, Components::CameraRef p_CameraRef)
{
  using namespace Resources;

  _INTR_PROFILE_CPU("Render Pass", "Render Shadows");
  _INTR_PROFILE_GPU("Render Shadows");

  _INTR_PROFILE_COUNTER_SET("Dispatched Draw Calls (Shadows)",
                            DrawCallDispatcher::_dispatchedDrawCallCount);

  const _INTR_ARRAY(Core::Resources::FrustumRef)& shadowFrustums =
      RenderProcess::Default::_shadowFrustums[p_CameraRef];
  for (uint32_t shadowMapIdx = 0u; shadowMapIdx < shadowFrustums.size();
       ++shadowMapIdx)
  {
    _INTR_PROFILE_CPU("Render Pass", "Render Shadow Map");
    _INTR_PROFILE_GPU("Render Shadow Map");

    Core::Resources::FrustumRef frustumRef = shadowFrustums[shadowMapIdx];

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferImageRef, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0u, shadowMapIdx);

    const uint32_t frustumIdx = shadowMapIdx + 1u;

    static DrawCallRefArray visibleDrawCalls;
    visibleDrawCalls.clear();

    RenderProcess::Default::getVisibleDrawCalls(
        p_CameraRef, frustumIdx, MaterialManager::getMaterialPassId(_N(Shadow)))
        .copy(visibleDrawCalls);
    RenderProcess::Default::getVisibleDrawCalls(
        p_CameraRef, frustumIdx,
        MaterialManager::getMaterialPassId(_N(ShadowFoliage)))
        .copy(visibleDrawCalls);
    RenderProcess::Default::getVisibleDrawCalls(
        p_CameraRef, frustumIdx,
        MaterialManager::getMaterialPassId(_N(ShadowGrass)))
        .copy(visibleDrawCalls);

    DrawCallManager::sortDrawCallsFrontToBack(visibleDrawCalls);

    // Update per mesh uniform data
    {
      Components::MeshManager::updatePerInstanceData(p_CameraRef, frustumIdx);
      Core::Components::MeshManager::updateUniformData(visibleDrawCalls);
    }

    VkClearValue clearValues[1] = {};
    {
      clearValues[0].depthStencil.depth = 1.0f;
      clearValues[0].depthStencil.stencil = 0u;
    }

    RenderSystem::beginRenderPass(
        _renderPassRef, _framebufferRefs[shadowMapIdx],
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, 1u, clearValues);
    {
      DrawCallDispatcher::queueDrawCalls(visibleDrawCalls, _renderPassRef,
                                         _framebufferRefs[shadowMapIdx]);
      _INTR_PROFILE_COUNTER_ADD("Dispatched Draw Calls (Shadows)",
                                DrawCallDispatcher::_dispatchedDrawCallCount);
    }
    RenderSystem::endRenderPass(_renderPassRef);

    ImageManager::insertImageMemoryBarrierSubResource(
        _shadowBufferImageRef, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0u, shadowMapIdx);
  }
}
}
}
}
}
