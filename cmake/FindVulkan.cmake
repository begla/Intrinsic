# Locate the Vulkan SDK

FIND_LIBRARY(Vulkan_LIBRARY
  NAMES vulkan-1 vulkan
  PATH_SUFFIXES lib64 lib Bin
  PATHS
  ${VK_SDK_PATH}
  $ENV{VK_SDK_PATH}
)
FIND_PATH(Vulkan_INCLUDE_DIR vulkan/vulkan.h
  PATH_SUFFIXES include Include
  PATHS
    ${VK_SDK_PATH}
    $ENV{VK_SDK_PATH}
)

SET(Vulkan_LIBRARIES
  ${Vulkan_LIBRARY}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Vulkan  DEFAULT_MSG  Vulkan_INCLUDE_DIR Vulkan_LIBRARY)

MARK_AS_ADVANCED(Vulkan_INCLUDE_DIR Vulkan_LIBRARY)
