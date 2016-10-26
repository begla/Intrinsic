# Locate the FBX SDK
# This module defines
#  FBX_FOUND, if false, do not try to link to FBX
#  FBX_LIBRARIES
#  FBX_INCLUDE_DIR

FIND_PATH(FBX_INCLUDE_DIR fbxsdk.h
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
)

FIND_LIBRARY(FBX_LIBRARY_DEBUG 
  NAMES fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
)

FIND_LIBRARY(FBX_LIBRARY_RELEASE 
  NAMES fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/fbx
)

SET(FBX_LIBRARIES
	debug ${FBX_LIBRARY_DEBUG}
	optimized ${FBX_LIBRARY_RELEASE}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBX  DEFAULT_MSG  FBX_LIBRARIES FBX_INCLUDE_DIR)

MARK_AS_ADVANCED(FBX_INCLUDE_DIR FBX_LIBRARIES)