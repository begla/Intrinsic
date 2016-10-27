# Locate the Vulkan SDK
# This module defines
#  Vulkan_FOUND, if false, do not try to link to Lua 
#  Vulkan_INCLUDE_DIR
#  Vulkan_LIBRARY
#
# Specifying either Qt5_ROOT_DIR or Qt5_QMAKE_EXECUTABLE is enough
# to find the entire Qt5 install.

FIND_LIBRARY(Vulkan_LIBRARY
  NAMES vulkan-1 vulkan
  PATH_SUFFIXES lib64 lib Bin
  PATHS
    ${VK_SDK_PATH}
    $ENV{VK_SDK_PATH}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
)
FIND_PATH(Vulkan_INCLUDE_DIR vulkan/vulkan.h
  PATH_SUFFIXES include Include
  PATHS
    ${VK_SDK_PATH}
    $ENV{VK_SDK_PATH}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
)

SET(Vulkan_LIBRARIES
  ${Vulkan_LIBRARY}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Vulkan  DEFAULT_MSG  Vulkan_INCLUDE_DIR Vulkan_LIBRARY)

MARK_AS_ADVANCED(Vulkan_INCLUDE_DIR Vulkan_LIBRARY)
