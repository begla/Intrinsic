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
using namespace Resources;

namespace
{
bool _swapChainUpdatePending = false;
const float _timeBetweenSwapChainUpdates = 1.0f;
float _timePassedSinceLastSwapChainUpdate = _timeBetweenSwapChainUpdates;
}

// Public static members
VkInstance RenderSystem::_vkInstance = nullptr;

VkPhysicalDevice RenderSystem::_vkPhysicalDevice = nullptr;
VkPhysicalDeviceMemoryProperties
    RenderSystem::_vkPhysicalDeviceMemoryProperties;

VkDevice RenderSystem::_vkDevice = nullptr;
VkPipelineCache RenderSystem::_vkPipelineCache = VK_NULL_HANDLE;

VkSurfaceKHR RenderSystem::_vkSurface = VK_NULL_HANDLE;
VkSwapchainKHR RenderSystem::_vkSwapchain = VK_NULL_HANDLE;
_INTR_ARRAY(VkImage) RenderSystem::_vkSwapchainImages;
_INTR_ARRAY(VkImageView) RenderSystem::_vkSwapchainImageViews;
glm::uvec2 RenderSystem::_backbufferDimensions = glm::uvec2(0u, 0u);

VkQueue RenderSystem::_vkQueue = nullptr;

uint32_t RenderSystem::_vkGraphicsAndComputeQueueFamilyIndex = (uint32_t)-1;

// <-

uint32_t RenderSystem::_backbufferIndex = 0u;
uint32_t RenderSystem::_activeBackbufferMask = 0u;

// <-

LockFreeStack<Dod::Ref, _INTR_MAX_DRAW_CALL_COUNT> RenderSystem::
    _visibleDrawCallsPerMaterialPass[_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT]
                                    [MaterialPass::kCount];
LockFreeStack<Dod::Ref, _INTR_MAX_MESH_COMPONENT_COUNT>
    RenderSystem::_visibleMeshComponents[_INTR_MAX_FRUSTUMS_PER_FRAME_COUNT];

// Private static members
VkCommandPool RenderSystem::_vkPrimaryCommandPool;
_INTR_ARRAY(VkCommandPool) RenderSystem::_vkSecondaryCommandPools;

_INTR_ARRAY(VkCommandBuffer) RenderSystem::_vkCommandBuffers;
_INTR_ARRAY(VkCommandBuffer) RenderSystem::_vkSecondaryCommandBuffers;

VkCommandBuffer RenderSystem::_vkTempCommandBuffer = nullptr;
VkFence RenderSystem::_vkTempCommandBufferFence = VK_NULL_HANDLE;

VkSemaphore RenderSystem::_vkImageAcquiredSemaphore;
_INTR_ARRAY(VkFence) RenderSystem::_vkDrawFences;

uint32_t RenderSystem::_allocatedSecondaryCmdBufferCount = 0u;
_INTR_ARRAY(ResourceReleaseEntry) RenderSystem::_resourcesToFree;

// <-

void RenderSystem::init(void* p_PlatformHandle, void* p_PlatformWindow)
{
  _INTR_LOG_INFO("Inititializing Vulkan Renderer...");

  // Init render states
  {
    RenderStates::init();
  }

  // Init. Vulkan
  {
    initVkInstance();
    initVkDevice();
    Debugging::init(_vkInstance);
    initVkSurface(p_PlatformHandle, p_PlatformWindow);
    initVkPipelineCache();
    initVkCommandPools();
    initVkTempCommandBuffer();
  }

  {
    GpuMemoryManager::init();
    Samplers::init();
    initManagers();
  }

  {
    GpuProgramManager::compileAllShaders();
    GpuProgramManager::createAllResources();
    RenderPassManager::createAllResources();
    ImageManager::createAllResources();
    VertexLayoutManager::createAllResources();
    PipelineLayoutManager::createAllResources();
    PipelineManager::createAllResources();
  }

  // Init. swap chain and cmd buffers
  {
    initOrUpdateVkSwapChain();
    initVkSynchronization();
    initVkCommandBuffers();
  }

  // Init. separate manager
  {
    UniformManager::init();
    MaterialBuffer::init();
  }

  // Init. render passes
  {
    RenderPass::GBuffer::init();
    RenderPass::GBufferTransparents::init();
    RenderPass::Sky::init();
    RenderPass::Foliage::init();
    RenderPass::Shadow::init();

    RenderPass::Lighting::init();
    RenderPass::VolumetricLighting::init();

    RenderPass::PreCombine::init();
    RenderPass::Bloom::init();
    RenderPass::PostCombine::init();
    RenderPass::LensFlare::init();

    RenderPass::Debug::init();
    RenderPass::PerPixelPicking::init();
  }

  {
    MaterialManager::initMaterialPasses();
    MaterialManager::createAllResources();
  }

  updateResolutionDependentResources();
}

// <-

void RenderSystem::onViewportChanged() { _swapChainUpdatePending = true; }

// <-

void RenderSystem::releaseQueuedResources()
{
  _INTR_PROFILE_CPU("Render System", "Releae Queued Resources");

  for (auto it = _resourcesToFree.begin(); it != _resourcesToFree.end();)
  {
    ResourceReleaseEntry& entry = *it;

    if (entry.age >= (uint32_t)RenderSystem::_vkSwapchainImages.size())
    {
      VkResult result = VK_SUCCESS;
      if (entry.typeName == _N(VkDescriptorSet))
      {
        result = vkFreeDescriptorSets(RenderSystem::_vkDevice,
                                      (VkDescriptorPool)entry.userData1, 1u,
                                      (VkDescriptorSet*)&entry.userData0);
      }
      else if (entry.typeName == _N(VkImage))
      {
        vkDestroyImage(RenderSystem::_vkDevice, (VkImage)entry.userData0,
                       nullptr);
      }
      else if (entry.typeName == _N(VkImageView))
      {
        vkDestroyImageView(RenderSystem::_vkDevice,
                           (VkImageView)entry.userData0, nullptr);
      }
      else if (entry.typeName == _N(VkDeviceMemory))
      {
        vkFreeMemory(RenderSystem::_vkDevice, (VkDeviceMemory)entry.userData0,
                     nullptr);
      }
      else if (entry.typeName == _N(VkBuffer))
      {
        vkDestroyBuffer(RenderSystem::_vkDevice, (VkBuffer)entry.userData0,
                        nullptr);
      }
      else if (entry.typeName == _N(VkPipeline))
      {
        vkDestroyPipeline(RenderSystem::_vkDevice, (VkPipeline)entry.userData0,
                          nullptr);
      }
      else
      {
        _INTR_ASSERT(false);
      }
      _INTR_VK_CHECK_RESULT(result);

      it = _resourcesToFree.erase(it);
    }
    else
    {
      ++entry.age;
      ++it;
    }
  }
}

void RenderSystem::updateResolutionDependentResources()
{
  // Recreate all pipelines (to update the view port size)
  PipelineManager::createAllResources();

  // Reset allocators
  GpuMemoryManager::resetAllocator(MemoryPoolType::kResolutionDependentBuffers);
  GpuMemoryManager::resetAllocator(MemoryPoolType::kResolutionDependentImages);

  RenderPass::GBufferTransparents::updateResolutionDependentResources();
  RenderPass::GBuffer::updateResolutionDependentResources();
  RenderPass::Debug::updateResolutionDependentResources();
  RenderPass::PerPixelPicking::updateResolutionDependentResources();
  RenderPass::Foliage::updateResolutionDependentResources();
  RenderPass::Sky::updateResolutionDependentResources();
  RenderPass::Shadow::updateResolutionDependentResources();
  RenderPass::Lighting::updateResolutionDependentResources();
  RenderPass::VolumetricLighting::updateResolutionDependentResources();
  RenderPass::PreCombine::updateResolutionDependentResources();
  RenderPass::Bloom::updateResolutionDependentResources();
  RenderPass::LensFlare::updateResolutionDependentResources();
  RenderPass::PostCombine::updateResolutionDependentResources();

  // Recreate all mesh draw calls (might use resources of the render passes)
  Components::MeshManager::destroyResources(
      Components::MeshManager::_activeRefs);
  Components::MeshManager::createResources(
      Components::MeshManager::_activeRefs);
}

// <-

void RenderSystem::dispatchComputeCall(Dod::Ref p_ComputeCall,
                                       VkCommandBuffer p_CommandBuffer)
{
  PipelineRef pipelineRef = ComputeCallManager::_descPipeline(p_ComputeCall);
  PipelineLayoutRef pipelineLayoutRef =
      PipelineManager::_descPipelineLayout(pipelineRef);
  const glm::uvec3& dimensions =
      ComputeCallManager::_descDimensions(p_ComputeCall);

  VkPipeline newPipeline = PipelineManager::_vkPipeline(pipelineRef);
  vkCmdBindPipeline(p_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    newPipeline);

  if (ComputeCallManager::_vkDescriptorSet(p_ComputeCall))
  {
    vkCmdBindDescriptorSets(
        p_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        PipelineLayoutManager::_vkPipelineLayout(pipelineLayoutRef), 0u, 1u,
        &ComputeCallManager::_vkDescriptorSet(p_ComputeCall),
        (uint32_t)ComputeCallManager::_dynamicOffsets(p_ComputeCall).size(),
        ComputeCallManager::_dynamicOffsets(p_ComputeCall).data());
  }

  vkCmdDispatch(p_CommandBuffer, (uint32_t)dimensions.x, (uint32_t)dimensions.y,
                (uint32_t)dimensions.z);
}

// <-

void RenderSystem::dispatchDrawCall(Dod::Ref p_DrawCall,
                                    VkCommandBuffer p_CommandBuffer)
{
  PipelineRef pipelineRef = DrawCallManager::_descPipeline(p_DrawCall);
  PipelineLayoutRef pipelineLayoutRef =
      PipelineManager::_descPipelineLayout(pipelineRef);

  VkPipeline newPipeline = PipelineManager::_vkPipeline(pipelineRef);
  vkCmdBindPipeline(p_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    newPipeline);

  if (DrawCallManager::_vkDescriptorSet(p_DrawCall))
  {
    vkCmdBindDescriptorSets(
        p_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        PipelineLayoutManager::_vkPipelineLayout(pipelineLayoutRef), 0u, 1u,
        &DrawCallManager::_vkDescriptorSet(p_DrawCall),
        (uint32_t)DrawCallManager::_dynamicOffsets(p_DrawCall).size(),
        DrawCallManager::_dynamicOffsets(p_DrawCall).data());
  }

  // Bind vertex buffers
  {
    _INTR_ARRAY(VkBuffer)& vtxBuffers =
        DrawCallManager::_vertexBuffers(p_DrawCall);

    if (!vtxBuffers.empty())
    {
      vkCmdBindVertexBuffers(
          p_CommandBuffer, 0u, (uint32_t)vtxBuffers.size(), vtxBuffers.data(),
          DrawCallManager::_vertexBufferOffsets(p_DrawCall).data());
    }
  }

  // Draw
  {
    BufferRef indexBufferRef = DrawCallManager::_descIndexBuffer(p_DrawCall);
    if (indexBufferRef.isValid())
    {
      const VkIndexType indexType =
          BufferManager::_descBufferType(indexBufferRef) == BufferType::kIndex16
              ? VK_INDEX_TYPE_UINT16
              : VK_INDEX_TYPE_UINT32;
      vkCmdBindIndexBuffer(
          p_CommandBuffer, BufferManager::_vkBuffer(indexBufferRef),
          DrawCallManager::_indexBufferOffset(p_DrawCall), indexType);
      vkCmdDrawIndexed(
          p_CommandBuffer, DrawCallManager::_descIndexCount(p_DrawCall),
          DrawCallManager::_descInstanceCount(p_DrawCall), 0u, 0u, 0u);
    }
    else
    {
      vkCmdDraw(p_CommandBuffer, DrawCallManager::_descVertexCount(p_DrawCall),
                DrawCallManager::_descInstanceCount(p_DrawCall), 0u, 0u);
    }
  }
}

// <-

void RenderSystem::beginRenderPass(Core::Dod::Ref p_RenderPass,
                                   Core::Dod::Ref p_Framebuffer,
                                   VkSubpassContents p_SubpassContents,
                                   uint32_t p_ClearValueCount,
                                   VkClearValue* p_ClearValues)
{
  VkRenderPassBeginInfo renderPassBegin = {};
  {
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = nullptr;
    renderPassBegin.renderPass = RenderPassManager::_vkRenderPass(p_RenderPass);
    renderPassBegin.framebuffer =
        FramebufferManager::_vkFrameBuffer(p_Framebuffer);

    const glm::uvec2& framebufferDim =
        FramebufferManager::_descDimensions(p_Framebuffer);
    renderPassBegin.renderArea.offset.x = 0u;
    renderPassBegin.renderArea.offset.y = 0u;
    renderPassBegin.renderArea.extent.width = framebufferDim.x;
    renderPassBegin.renderArea.extent.height = framebufferDim.y;

    renderPassBegin.clearValueCount = p_ClearValueCount;
    renderPassBegin.pClearValues = p_ClearValues;
  }

  vkCmdBeginRenderPass(getPrimaryCommandBuffer(), &renderPassBegin,
                       p_SubpassContents);
}

// <-

void RenderSystem::initManagers()
{
  _INTR_LOG_INFO("Inititializing Renderer Managers...");

  // Init managers
  {
    BufferManager::init();
    GpuProgramManager::init();
    RenderPassManager::init();
    VertexLayoutManager::init();
    PipelineLayoutManager::init();
    PipelineManager::init();
    DrawCallManager::init();
    ComputeCallManager::init();
    ImageManager::init();
    FramebufferManager::init();
    MaterialManager::init();
  }

  // Load managers
  {
    GpuProgramManager::loadFromSingleFile("managers/GpuProgram.manager.json");
    ImageManager::loadFromSingleFile("managers/Image.manager.json");
    MaterialManager::loadFromSingleFile("managers/Material.manager.json");
  }

  // Setup default vertex layouts
  {
    // Mesh
    {
      VertexLayoutRef meshVertexLayout =
          VertexLayoutManager::createVertexLayout(_N(Mesh));
      VertexLayoutManager::resetToDefault(meshVertexLayout);

      Helper::createDefaultMeshVertexLayout(meshVertexLayout);

      VertexLayoutRef debugLineVertexLayout =
          VertexLayoutManager::createVertexLayout(_N(DebugLineVertex));
      VertexLayoutManager::resetToDefault(debugLineVertexLayout);

      Helper::createDebugLineVertexLayout(debugLineVertexLayout);
    }
  }
}

// <-

void RenderSystem::initVkInstance()
{
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Intrinsic";
  appInfo.pEngineName = "Intrinsic";
  appInfo.apiVersion = VK_API_VERSION_1_0;

  _INTR_ARRAY(const char*) enabledExtensions;
  {
    enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    enabledExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif // VK_USE_PLATFORM_WIN32_KHR
  }

  _INTR_ARRAY(const char*) enabledLayers;
  {
    if ((Settings::Manager::_rendererFlags &
         Settings::RendererFlags::kValidationEnabled) > 0u)
    {
      enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
      enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
  }

  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = nullptr;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  if (enabledExtensions.size() > 0u)
  {
    instanceCreateInfo.enabledExtensionCount =
        (uint32_t)enabledExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
  }

  if (enabledLayers.size() > 0u)
  {
    instanceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
  }

  VkResult result =
      vkCreateInstance(&instanceCreateInfo, nullptr, &_vkInstance);
  _INTR_VK_CHECK_RESULT(result);
}

// <-

void RenderSystem::initVkDevice()
{
  _INTR_LOG_INFO("Creating Vulkan device...");
  _INTR_LOG_PUSH();

  // Retrieve the physical device and its features
  VkPhysicalDeviceProperties physDeviceProps;
  VkPhysicalDeviceFeatures physDevFeatures;
  {
    uint32_t deviceCount;
    VkResult result =
        vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);
    _INTR_VK_CHECK_RESULT(result);
    _INTR_ASSERT(deviceCount >= 1u && "No Vulkan capable Device found");

    _INTR_LOG_INFO("Found %u available physical devices...", deviceCount);

    _INTR_ARRAY(VkPhysicalDevice) physDevs;
    physDevs.resize(deviceCount);

    result =
        vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, physDevs.data());
    _INTR_VK_CHECK_RESULT(result);

    // Always use the first device found
    _vkPhysicalDevice = physDevs[0];

    vkGetPhysicalDeviceProperties(_vkPhysicalDevice, &physDeviceProps);
    vkGetPhysicalDeviceFeatures(_vkPhysicalDevice, &physDevFeatures);

    // Queue memory properties
    vkGetPhysicalDeviceMemoryProperties(_vkPhysicalDevice,
                                        &_vkPhysicalDeviceMemoryProperties);

    _INTR_LOG_INFO("Using physical device %s (Driver %u)...",
                   physDeviceProps.deviceName, physDeviceProps.driverVersion);
  }

  // Find graphics _AND_ compute queue
  {
    _INTR_LOG_INFO("Retrieving compute and graphics queues...");

    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueCount,
                                             nullptr);
    _INTR_ASSERT(queueCount >= 1u);

    _INTR_LOG_INFO("Found %u available queues...", queueCount);

    _INTR_ARRAY(VkQueueFamilyProperties) queueProps;
    queueProps.resize(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueCount,
                                             queueProps.data());

    uint32_t graphicsComputeQueue;
    for (graphicsComputeQueue = 0; graphicsComputeQueue < queueCount;
         graphicsComputeQueue++)
    {
      if ((queueProps[graphicsComputeQueue].queueFlags &
           VK_QUEUE_GRAPHICS_BIT) > 0u &&
          (queueProps[graphicsComputeQueue].queueFlags & VK_QUEUE_COMPUTE_BIT) >
              0u)
      {
        _INTR_LOG_INFO("Using queue #%u for graphics and compute...",
                       graphicsComputeQueue);
        break;
      }
    }
    _INTR_ASSERT(graphicsComputeQueue < queueCount &&
                 "Unable to locate a matching queue");

    _vkGraphicsAndComputeQueueFamilyIndex = graphicsComputeQueue;
  }

  // Setup device queue
  const float queuePriorities[1] = {0.0f};
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  {
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = _vkGraphicsAndComputeQueueFamilyIndex;
    queueCreateInfo.queueCount = 1u;
    queueCreateInfo.pQueuePriorities = queuePriorities;
  }

  // Check if debug marker extension is supported
  bool debugMarkerExtPresent = false;
  {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr,
                                         &extensionCount, nullptr);

    _INTR_ARRAY(VkExtensionProperties) extensions;
    extensions.resize(extensionCount);

    vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr,
                                         &extensionCount, extensions.data());

    for (auto& ext : extensions)
    {
      if (!strcmp(ext.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
        debugMarkerExtPresent = true;
      }
    }
  }

  _INTR_ARRAY(const char*) enabledExtensions;
  {
    enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (debugMarkerExtPresent)
      enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  }

  _INTR_ARRAY(const char*) enabledLayers;
  {
    // None yet
  }

  // Create device
  VkDeviceCreateInfo deviceCreateInfo = {};
  {
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.queueCreateInfoCount = 1u;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &physDevFeatures;

    if (enabledExtensions.size() > 0)
    {
      deviceCreateInfo.enabledExtensionCount =
          (uint32_t)enabledExtensions.size();
      deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }

    if (enabledLayers.size() > 0)
    {
      deviceCreateInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
      deviceCreateInfo.ppEnabledLayerNames = enabledLayers.data();
    }

    VkResult result = vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo,
                                     nullptr, &_vkDevice);
    _INTR_VK_CHECK_RESULT(result);
  }

  // Retrieve device queue
  vkGetDeviceQueue(_vkDevice, _vkGraphicsAndComputeQueueFamilyIndex, 0u,
                   &_vkQueue);

  _INTR_LOG_POP();
}

// <-

void RenderSystem::initVkSurface(void* p_PlatformHandle, void* p_PlatformWindow)
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
  surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  surfaceCreateInfo.dpy = (Display*)p_PlatformHandle;
  surfaceCreateInfo.window = (Window)p_PlatformWindow;

  VkResult result = vkCreateXlibSurfaceKHR(_vkInstance, &surfaceCreateInfo,
                                           nullptr, &_vkSurface);
  _INTR_VK_CHECK_RESULT(result);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
  surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfaceCreateInfo.hinstance = (HINSTANCE)p_PlatformHandle;
  surfaceCreateInfo.hwnd = (HWND)p_PlatformWindow;

  VkResult result = vkCreateWin32SurfaceKHR(_vkInstance, &surfaceCreateInfo,
                                            nullptr, &_vkSurface);
  _INTR_VK_CHECK_RESULT(result);
#endif // VK_USE_PLATFORM_WIN32_KHR
}

// <-

void RenderSystem::initOrUpdateVkSwapChain()
{
  _INTR_LOG_INFO("Creating Vulkan swapchain...");
  _INTR_LOG_PUSH();

  // Check surface support
  VkBool32 supported = false;
  VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
      _vkPhysicalDevice, 0u, _vkSurface, &supported);
  _INTR_VK_CHECK_RESULT(result);
  _INTR_ASSERT(supported);

  // Get surfaces "features"
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      _vkPhysicalDevice, _vkSurface, &surfaceCapabilities);
  _INTR_VK_CHECK_RESULT(result);

  static VkFormat surfaceFormatToUse = VK_FORMAT_B8G8R8A8_SRGB;
  static VkColorSpaceKHR surfaceColorSpaceToUse =
      VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  static VkPresentModeKHR presentModeToUse = Settings::mapPresentMode(Settings::Manager::_presentMode);

  // Check if present mode is supported
  {
    uint32_t presentModeCount = 0u;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        _vkPhysicalDevice, _vkSurface, &presentModeCount, nullptr);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_ASSERT(presentModeCount > 0u);
    _INTR_ARRAY(VkPresentModeKHR) presentModes;
    presentModes.resize(presentModeCount);

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        _vkPhysicalDevice, _vkSurface, &presentModeCount, presentModes.data());
    _INTR_VK_CHECK_RESULT(result);

    bool found = false;
    for (uint32_t i = 0u; i < presentModeCount; ++i)
    {
      if (presentModes[i] == presentModeToUse)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      _INTR_LOG_WARNING("Selected present mode is not supported, falling back to immediate mode...");
      presentModeToUse = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }

  // Check if surface format is supported
  {
    uint32_t formatCount = 0u;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(_vkPhysicalDevice, _vkSurface,
                                                  &formatCount, nullptr);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_ASSERT(formatCount > 0u);
    _INTR_ARRAY(VkSurfaceFormatKHR) surfaceFormats;
    surfaceFormats.resize(formatCount);

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        _vkPhysicalDevice, _vkSurface, &formatCount, surfaceFormats.data());
    _INTR_VK_CHECK_RESULT(result);

    bool found = false;
    for (uint32_t i = 0u; i < formatCount; ++i)
    {
      if (surfaceFormats[i].colorSpace == surfaceColorSpaceToUse &&
          surfaceFormats[i].format == surfaceFormatToUse)
      {
        found = true;
        break;
      }
    }
    _INTR_ASSERT(found &&
                 "Surface format and/or surface color space not supported");
  }

  // Create swapchain
  VkSwapchainCreateInfoKHR swapchainCreationInfo = {};
  {
    swapchainCreationInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreationInfo.pNext = nullptr;
    swapchainCreationInfo.surface = _vkSurface;
    swapchainCreationInfo.minImageCount =
        std::min(2u, surfaceCapabilities.maxImageCount);
    swapchainCreationInfo.imageFormat = surfaceFormatToUse;
    swapchainCreationInfo.imageColorSpace = surfaceColorSpaceToUse;
    swapchainCreationInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainCreationInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreationInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreationInfo.imageArrayLayers = 1u;
    swapchainCreationInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreationInfo.queueFamilyIndexCount = 0u;
    swapchainCreationInfo.pQueueFamilyIndices = nullptr;
    swapchainCreationInfo.presentMode = presentModeToUse;
    swapchainCreationInfo.oldSwapchain = _vkSwapchain;
    swapchainCreationInfo.clipped = true;
    swapchainCreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    result = vkCreateSwapchainKHR(_vkDevice, &swapchainCreationInfo, nullptr,
                                  &_vkSwapchain);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_LOG_INFO("Created new swapchain (%u px x %u px)...",
                   swapchainCreationInfo.imageExtent.width,
                   swapchainCreationInfo.imageExtent.height);
  }

  // Update backbuffer dimensions
  _backbufferDimensions = glm::uvec2(swapchainCreationInfo.imageExtent.width,
                                     swapchainCreationInfo.imageExtent.height);

  // Destroy previous swapchain
  if (swapchainCreationInfo.oldSwapchain != VK_NULL_HANDLE)
  {
    _INTR_LOG_INFO("Destroying previous swapchain...");

    // Destroy the previous swapchain and image views
    for (uint32_t i = 0u; i < _vkSwapchainImageViews.size(); ++i)
    {
      vkDestroyImageView(_vkDevice, _vkSwapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(_vkDevice, swapchainCreationInfo.oldSwapchain,
                          nullptr);
  }

  // Retrieve swapchain images
  uint32_t swapchainImageCount = 0u;
  {
    result = vkGetSwapchainImagesKHR(_vkDevice, _vkSwapchain,
                                     &swapchainImageCount, nullptr);
    _INTR_VK_CHECK_RESULT(result);

    _INTR_LOG_INFO("Retrieving %u swapchain images...", swapchainImageCount);
    _vkSwapchainImages.resize(swapchainImageCount);
    _vkSwapchainImageViews.resize(swapchainImageCount);

    result =
        vkGetSwapchainImagesKHR(_vkDevice, _vkSwapchain, &swapchainImageCount,
                                _vkSwapchainImages.data());
    _INTR_VK_CHECK_RESULT(result);
  }

  // Create image views
  {
    for (uint32_t i = 0u; i < swapchainImageCount; ++i)
    {
      VkImageViewCreateInfo colorAttachmentView = {};
      {
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = nullptr;
        colorAttachmentView.format = VK_FORMAT_B8G8R8A8_SRGB;
        colorAttachmentView.components.r = VK_COMPONENT_SWIZZLE_R;
        colorAttachmentView.components.g = VK_COMPONENT_SWIZZLE_G;
        colorAttachmentView.components.b = VK_COMPONENT_SWIZZLE_B;
        colorAttachmentView.components.a = VK_COMPONENT_SWIZZLE_A;
        colorAttachmentView.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.baseMipLevel = 0u;
        colorAttachmentView.subresourceRange.levelCount = 1u;
        colorAttachmentView.subresourceRange.baseArrayLayer = 0u;
        colorAttachmentView.subresourceRange.layerCount = 1u;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0u;
        colorAttachmentView.image = _vkSwapchainImages[i];
      }

      result = vkCreateImageView(_vkDevice, &colorAttachmentView, nullptr,
                                 &_vkSwapchainImageViews[i]);
      _INTR_VK_CHECK_RESULT(result);
    }
  }

  _INTR_ARRAY(ImageRef) imagesToCreate;
  _INTR_ARRAY(ImageRef) imagesToDestroy;
  _INTR_ARRAY(FramebufferRef) fbsToCreate;
  _INTR_ARRAY(FramebufferRef) fbsToDestroy;

  // Destroy existing backbuffer GPU resources
  {
    for (uint32_t i = 0u; i < swapchainImageCount; ++i)
    {
      Name bufferName =
          _INTR_STRING("Backbuffer") + StringUtil::toString<uint32_t>(i);

      ImageRef imageRef = ImageManager::_getResourceByName(bufferName);
      if (imageRef.isValid())
      {
        imagesToDestroy.push_back(imageRef);
      }
      else
      {
        imageRef = ImageManager::createImage(bufferName);
      }
    }

    FramebufferManager::destroyResources(fbsToDestroy);
    ImageManager::destroyResources(imagesToDestroy);
  }

  // Create backbuffer GPU resources
  {
    for (uint32_t i = 0u; i < swapchainImageCount; ++i)
    {
      Name bufferName =
          _INTR_STRING("Backbuffer") + StringUtil::toString<uint32_t>(i);

      ImageRef backbufferRef = ImageManager::getResourceByName(bufferName);
      {
        ImageManager::resetToDefault(backbufferRef);
        ImageManager::addResourceFlags(
            backbufferRef, Dod::Resources::ResourceFlags::kResourceVolatile);

        ImageManager::_vkImage(backbufferRef) = _vkSwapchainImages[i];
        ImageManager::_vkImageView(backbufferRef) = _vkSwapchainImageViews[i];
        ImageManager::_vkSubResourceImageViews(backbufferRef).resize(1u);
        ImageManager::_vkSubResourceImageViews(backbufferRef)[0u].resize(1u);
        ImageManager::_vkSubResourceImageViews(backbufferRef)[0u][0u] =
            _vkSwapchainImageViews[i];
        ImageManager::_descDimensions(backbufferRef) =
            glm::uvec3(_backbufferDimensions.x, _backbufferDimensions.y, 1u);
        ImageManager::addImageFlags(backbufferRef,
                                    ImageFlags::kExternalImage |
                                        ImageFlags::kExternalView |
                                        ImageFlags::kExternalDeviceMemory);

        imagesToCreate.push_back(backbufferRef);
      }
    }

    ImageManager::createResources(imagesToCreate);
  }

  _INTR_LOG_POP();
}

// <-

void RenderSystem::initVkPipelineCache()
{
  _INTR_LOG_INFO("Creating Vulkan cache...");

  VkPipelineCacheCreateInfo pipelineCache = {};
  {
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCache.pNext = nullptr;
    pipelineCache.initialDataSize = 0;
    pipelineCache.pInitialData = nullptr;
    pipelineCache.flags = 0u;
  }

  VkResult result = vkCreatePipelineCache(_vkDevice, &pipelineCache, nullptr,
                                          &_vkPipelineCache);
  _INTR_VK_CHECK_RESULT(result);
}

// <-

void RenderSystem::initVkCommandPools()
{
  _INTR_LOG_INFO("Creating Vulkan command pools...");

  // Prim. command pool
  {
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    {
      commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      commandPoolCreateInfo.pNext = nullptr;
      commandPoolCreateInfo.queueFamilyIndex =
          _vkGraphicsAndComputeQueueFamilyIndex;
      commandPoolCreateInfo.flags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    VkResult result = vkCreateCommandPool(_vkDevice, &commandPoolCreateInfo,
                                          nullptr, &_vkPrimaryCommandPool);
    _INTR_VK_CHECK_RESULT(result);
  }

  // Second. command pool
  {
    const uint32_t actualSecondCmdPoolCount =
        _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT;
    _vkSecondaryCommandPools.resize(actualSecondCmdPoolCount);

    for (uint32_t i = 0u; i < actualSecondCmdPoolCount; ++i)
    {
      VkCommandPoolCreateInfo commandPoolCreateInfo = {};
      {
        commandPoolCreateInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.pNext = nullptr;
        commandPoolCreateInfo.queueFamilyIndex =
            _vkGraphicsAndComputeQueueFamilyIndex;
        commandPoolCreateInfo.flags =
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      }

      VkResult result =
          vkCreateCommandPool(_vkDevice, &commandPoolCreateInfo, nullptr,
                              &_vkSecondaryCommandPools[i]);
      _INTR_VK_CHECK_RESULT(result);
    }
  }
}

// <-

void RenderSystem::initVkTempCommandBuffer()
{
  _INTR_LOG_INFO("Creating Vulkan temporary command buffer...");

  {
    VkCommandBufferAllocateInfo cmd = {};
    {
      cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      cmd.pNext = nullptr;
      cmd.commandPool = _vkPrimaryCommandPool;
      cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      cmd.commandBufferCount = 1u;
    }

    VkResult result =
        vkAllocateCommandBuffers(_vkDevice, &cmd, &_vkTempCommandBuffer);
    _INTR_VK_CHECK_RESULT(result);
  }

  {
    VkFenceCreateInfo fenceInfo = {};
    {
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.pNext = nullptr;
    }

    VkResult result = vkCreateFence(RenderSystem::_vkDevice, &fenceInfo,
                                    nullptr, &_vkTempCommandBufferFence);
    _INTR_VK_CHECK_RESULT(result);
  }
}

void RenderSystem::initVkCommandBuffers()
{
  _INTR_LOG_INFO("Creating Vulkan command buffers...");

  // Primary cmd buffer
  {
    const uint32_t actualPrimCmdBufferCount =
        (uint32_t)_vkSwapchainImages.size();
    _vkCommandBuffers.resize(actualPrimCmdBufferCount);

    VkCommandBufferAllocateInfo cmd = {};
    {
      cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      cmd.pNext = nullptr;
      cmd.commandPool = _vkPrimaryCommandPool;
      cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      cmd.commandBufferCount = actualPrimCmdBufferCount;
    }

    VkResult result =
        vkAllocateCommandBuffers(_vkDevice, &cmd, _vkCommandBuffers.data());
    _INTR_VK_CHECK_RESULT(result);
  }

  // Secondary cmd buffers
  {
    const uint32_t actualSecondCmdBufferCount =
        (uint32_t)_vkSwapchainImages.size() *
        _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT;
    _vkSecondaryCommandBuffers.resize(actualSecondCmdBufferCount);

    for (uint32_t i = 0u; i < actualSecondCmdBufferCount; ++i)
    {
      VkCommandBufferAllocateInfo cmd = {};
      {
        cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd.pNext = nullptr;
        // Reuse command pools from frame to frame
        cmd.commandPool =
            _vkSecondaryCommandPools[i %
                                     _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT];
        cmd.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmd.commandBufferCount = 1u;
      }

      VkResult result = vkAllocateCommandBuffers(
          _vkDevice, &cmd, &_vkSecondaryCommandBuffers[i]);
      _INTR_VK_CHECK_RESULT(result);
    }
  }
}

void RenderSystem::destroyVkCommandBuffers()
{
  _INTR_LOG_INFO("Destroying Vulkan command buffers...");

  vkFreeCommandBuffers(_vkDevice, _vkPrimaryCommandPool,
                       (uint32_t)_vkCommandBuffers.size(),
                       _vkCommandBuffers.data());
  _vkCommandBuffers.clear();

  const uint32_t actualSecondCmdBufferCount =
      (uint32_t)_vkSwapchainImages.size() *
      _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT;
  for (uint32_t i = 0u; i < actualSecondCmdBufferCount; ++i)
  {
    vkFreeCommandBuffers(
        _vkDevice,
        _vkSecondaryCommandPools[i % _INTR_VK_SECONDARY_COMMAND_BUFFER_COUNT],
        1u, &_vkSecondaryCommandBuffers[i]);
  }
  _vkSecondaryCommandBuffers.clear();
}

// <-

void RenderSystem::initVkSynchronization()
{
  VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  imageAcquiredSemaphoreCreateInfo.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  imageAcquiredSemaphoreCreateInfo.pNext = nullptr;
  imageAcquiredSemaphoreCreateInfo.flags = 0;

  VkResult result =
      vkCreateSemaphore(_vkDevice, &imageAcquiredSemaphoreCreateInfo, nullptr,
                        &_vkImageAcquiredSemaphore);
  _INTR_VK_CHECK_RESULT(result);

  _vkDrawFences.resize(_vkSwapchainImages.size());
  for (uint32_t i = 0u; i < _vkSwapchainImages.size(); ++i)
  {
    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkResult result =
        vkCreateFence(_vkDevice, &fenceInfo, nullptr, &_vkDrawFences[i]);
    _INTR_VK_CHECK_RESULT(result);
  }

  result = vkResetFences(_vkDevice, (uint32_t)_vkDrawFences.size(),
                         _vkDrawFences.data());
  _INTR_VK_CHECK_RESULT(result);
}

void RenderSystem::resizeSwapChain()
{
  if (_swapChainUpdatePending &&
      _timePassedSinceLastSwapChainUpdate >= _timeBetweenSwapChainUpdates)
  {
    vkDeviceWaitIdle(_vkDevice);

    initOrUpdateVkSwapChain();
    updateResolutionDependentResources();

    _swapChainUpdatePending = false;
    _timePassedSinceLastSwapChainUpdate = 0.0f;
  }
  else
  {
    _timePassedSinceLastSwapChainUpdate += TaskManager::_lastDeltaT;
  }
}

// <-

void RenderSystem::beginFrame()
{
  _INTR_PROFILE_CPU("Render System", "Begin Frame");

  {
    releaseQueuedResources();
  }

  {
    _INTR_PROFILE_CPU("Render System", "Acquire Next Image");

    VkResult result = vkAcquireNextImageKHR(_vkDevice, _vkSwapchain, UINT64_MAX,
                                            _vkImageAcquiredSemaphore,
                                            VK_NULL_HANDLE, &_backbufferIndex);
    _INTR_VK_CHECK_RESULT(result);
  }

  {
    _INTR_PROFILE_CPU("Render System", "Wait For Previous Frame");
    waitForFrame(_backbufferIndex);
  }

  {
    _allocatedSecondaryCmdBufferCount = 0u;
    beginPrimaryCommandBuffer();

#if defined(_INTR_PROFILING_ENABLED)
    MicroProfileFlip(getPrimaryCommandBuffer());
#endif // _INTR_PROFILING_ENABLED

    insertPostPresentBarrier();
  }

  UniformManager::resetPerInstanceAllocators();
  DrawCallDispatcher::reset();
}

// <-

void RenderSystem::endFrame()
{
  _INTR_PROFILE_CPU("Render System", "End Frame");

  {
    // Wait for any remaining tasks
    Application::_scheduler.WaitforAll();

    // Insert pre-present barrier and end primary command buffer
    insertPrePresentBarrier();
    endPrimaryCommandBuffer();
  }

  {
    _INTR_PROFILE_CPU("Render System", "Queue Submit");

    VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {};
    {
      submitInfo.pNext = nullptr;
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.waitSemaphoreCount = 1u;
      submitInfo.pWaitSemaphores = &_vkImageAcquiredSemaphore;
      submitInfo.pWaitDstStageMask = &pipeStageFlags;
      submitInfo.commandBufferCount = 1u;
      submitInfo.pCommandBuffers = &_vkCommandBuffers[_backbufferIndex];
      submitInfo.signalSemaphoreCount = 0u;
      submitInfo.pSignalSemaphores = nullptr;
    }

    VkResult result = vkQueueSubmit(_vkQueue, 1u, &submitInfo,
                                    _vkDrawFences[_backbufferIndex]);
    _INTR_VK_CHECK_RESULT(result);

    _activeBackbufferMask |= 1u << _backbufferIndex;
  }

  {
    _INTR_PROFILE_CPU("Renderer", "Queue Present");

    VkPresentInfoKHR present = {};
    {
      present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      present.pNext = nullptr;
      present.swapchainCount = 1u;
      present.pSwapchains = &_vkSwapchain;
      present.pImageIndices = &_backbufferIndex;
      present.pWaitSemaphores = nullptr;
      present.waitSemaphoreCount = 0u;
      present.pResults = nullptr;
    }

    VkResult result = vkQueuePresentKHR(_vkQueue, &present);
    _INTR_VK_CHECK_RESULT(result);
  }

  GpuMemoryManager::updateMemoryStats();
}
}
}
}
