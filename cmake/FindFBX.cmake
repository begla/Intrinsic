# Locate the FBX SDK

if(WIN32)
  SET(FBX_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx)
else()
  SET(FBX_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx_linux)
endif()

FIND_PATH(FBX_INCLUDE_DIR fbxsdk.h
  PATH_SUFFIXES include
  PATHS
  ${FBX_LIBRARY_PATH}
)

FIND_LIBRARY(FBX_LIBRARY_DEBUG 
  NAMES libfbxsdkd fbxsdkd
  PATH_SUFFIXES lib64 lib
  PATHS
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx_linux
)

FIND_LIBRARY(FBX_LIBRARY_RELEASE 
  NAMES libfbxsdk fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx_linux
)

SET(FBX_LIBRARIES
	debug ${FBX_LIBRARY_DEBUG}
	optimized ${FBX_LIBRARY_RELEASE}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBX  DEFAULT_MSG  FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)

MARK_AS_ADVANCED(FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)
