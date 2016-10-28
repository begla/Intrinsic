# Locate the FBX SDK
# This module defines
#  FBX_FOUND, if false, do not try to link to FBX
#  FBX_LIBRARIES
#  FBX_INCLUDE_DIR

FIND_PATH(FBX_INCLUDE_DIR fbxsdk.h
  PATH_SUFFIXES include
  PATHS
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

FIND_LIBRARY(FBX_LIBRARY_DEBUG 
  NAMES fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(FBX_LIBRARY_RELEASE 
  NAMES fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

SET(FBX_LIBRARIES
	debug ${FBX_LIBRARY_DEBUG}
	optimized ${FBX_LIBRARY_RELEASE}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBX  DEFAULT_MSG  FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)

MARK_AS_ADVANCED(FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)