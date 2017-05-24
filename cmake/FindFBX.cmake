# Locate the FBX SDK
# This module defines
#  FBX_FOUND, if false, do not try to link to FBX
#  FBX_LIBRARIES
#  FBX_INCLUDE_DIR

FIND_PATH(FBX_INCLUDE_DIR fbxsdk.h
  PATH_SUFFIXES include
  PATHS
  ${CMAKE_SOURCE_DIR}/Intrinsic_Dependencies/dependencies/fbx
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(LIBFOLDERSUFFIX "x64")
ELSE()
  SET(LIBFOLDERSUFFIX "x86")
ENDIF()

IF (NOT FBX_LIBRARY_DIR)
  IF (MSVC)
    IF (MSVC_VERSION EQUAL 1800)
      SET(LIBFOLDER vs2013/${LIBFOLDERSUFFIX})
    ELSEIF (MSVC_VERSION EQUAL 1900)
      SET(LIBFOLDER vs2015/${LIBFOLDERSUFFIX})
    ENDIF()
  ENDIF()

  SET(FBX_LIBRARY_DIR ${FBX_INCLUDE_DIR}/../lib/${LIBFOLDER})
ENDIF()

#Debug FBX LIBRARY DIR
MESSAGE(STATUS  "SOURCE DIR PATH"  ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS 	"FBX LIBRARY DIR: " ${FBX_LIBRARY_DIR}/debug)
MESSAGE(STATUS  "FBX INCLUDE DIR: " ${FBX_INCLUDE_DIR})


FIND_LIBRARY(FBX_LIBRARY_DEBUG 
  NAMES libfbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ${FBX_LIBRARY_DIR}
  ${FBX_LIBRARY_DIR}/debug
  ${CMAKE_SOURCE_DIR}/Intrinsic_Dependencies/dependencies/fbx
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
  NAMES libfbxsdk fbxsdk
  PATH_SUFFIXES lib64 lib
  PATHS
  ${FBX_LIBRARY_DIR}
  ${FBX_LIBRARY_DIR}/release
  ${CMAKE_SOURCE_DIR}/Intrinsic_Dependencies/dependencies/fbx
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

MESSAGE(STATUS "FIND LIB DEBUG: " ${FBX_LIBRARY_DEBUG})

SET(FBX_LIBRARIES
	debug ${FBX_LIBRARY_DEBUG}
	optimized ${FBX_LIBRARY_RELEASE}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBX  DEFAULT_MSG  FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)

MARK_AS_ADVANCED(FBX_INCLUDE_DIR FBX_LIBRARY_DEBUG FBX_LIBRARY_RELEASE)
