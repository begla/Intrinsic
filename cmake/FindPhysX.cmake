# Locate the PhysX SDK
#
# This module defines
#  PhysX_FOUND, if false, do not try to link to PhysX
#  PhysX_LIBRARIES
#  PhysX_INCLUDE_DIR
#
# To control the finding you can specify the following entries;
#  PhysX_LIBRARY_DIR
#  PhysX_PROFILE

FIND_PATH(PhysX_INCLUDE_DIR PxPhysicsAPI.h
  PATHS
  $ENV{PHYSX_HOME}/include
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/include
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
  SET(LIBFOLDERSUFFIX "64")
  IF(MSVC)
    SET(PHYSXPREFIX "_x64")
  ELSE()
    SET(PHYSXPREFIX "_64")
  ENDIF()
ELSE()
  SET(LIBFOLDERSUFFIX "32")
ENDIF()

IF (PhysX_PROFILE)
  SET(PHYSXRELEASE "PROFILE")
ENDIF()

IF (NOT PhysX_LIBRARY_DIR)
  IF (MSVC)
    IF (MSVC_VERSION EQUAL 1700)
      SET(LIBFOLDER vc11win${LIBFOLDERSUFFIX})
    ELSEIF (MSVC_VERSION EQUAL 1800)
      SET(LIBFOLDER vc12win${LIBFOLDERSUFFIX})
    ELSEIF (MSVC_VERSION EQUAL 1900)
      SET(LIBFOLDER vc14win${LIBFOLDERSUFFIX})
    ENDIF()
  ELSEIF (CMAKE_HOST_APPLE)
    SET(LIBFOLDER osx${LIBFOLDERSUFFIX})
  ELSE()
    SET(LIBFOLDER linux${LIBFOLDERSUFFIX})
  ENDIF()

  SET(PhysX_LIBRARY_DIR ${PhysX_INCLUDE_DIR}/../Lib/${LIBFOLDER})
ENDIF()

FIND_LIBRARY(PhysX_LIBRARY_RELEASE PhysX3${PHYSXRELEASE}${PHYSXPREFIX}
  PATH_SUFFIXES lib lib64
  PATHS
  ${PhysX_LIBRARY_DIR}
  $ENV{PHYSX_HOME}/Lib/${LIBFOLDERSUFFIX}
  $ENV{PHYSX_HOME}/
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/Lib/${LIBFOLDERSUFFIX}
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)
FIND_LIBRARY(PhysX_LIBRARY_DEBUG PhysX3DEBUG${PHYSXPREFIX}
  PATH_SUFFIXES lib lib64
  PATHS
  ${PhysX_LIBRARY_DIR}
  $ENV{PHYSX_HOME}/Lib/${LIBFOLDERSUFFIX}
  $ENV{PHYSX_HOME}/
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/Lib/${LIBFOLDERSUFFIX}
  ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

SET(PhysX_LIBRARIES
  debug ${PhysX_LIBRARY_DEBUG}
  optimized ${PhysX_LIBRARY_RELEASE}
)

FOREACH(component ${PhysX_FIND_COMPONENTS})
  FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_DEBUG PhysX3${component}DEBUG${PHYSXPREFIX} PhysX3${component}DEBUG PhysX${component}DEBUG ${component}DEBUG
    PATH_SUFFIXES lib lib64
    PATHS
    ${PhysX_LIBRARY_DIR}
    $ENV{PHYSX_HOME}/Lib/${LIBFOLDERSUFFIX}
    $ENV{PHYSX_HOME}/
    ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/Lib/${LIBFOLDERSUFFIX}
    ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
  )
  IF (PhysX_LIBRARY_COMPONENT_${component}_DEBUG)
    SET(PhysX_LIBRARIES
      ${PhysX_LIBRARIES}
      debug "${PhysX_LIBRARY_COMPONENT_${component}_DEBUG}"
    )
  ENDIF()

  FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_RELEASE PhysX3${component}${PHYSXRELEASE}${PHYSXPREFIX} PhysX3${component}${PHYSXRELEASE} PhysX${component}${PHYSXRELEASE} ${component}${PHYSXRELEASE}
    PATH_SUFFIXES lib lib64
    PATHS
    ${PhysX_LIBRARY_DIR}
    $ENV{PHYSX_HOME}/Lib/${LIBFOLDERSUFFIX}
    $ENV{PHYSX_HOME}/
    ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/Lib/${LIBFOLDERSUFFIX}
    ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/physx/
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
  )
  IF (PhysX_LIBRARY_COMPONENT_${component}_RELEASE)
    SET(PhysX_LIBRARIES
      ${PhysX_LIBRARIES}
      optimized "${PhysX_LIBRARY_COMPONENT_${component}_RELEASE}"
    )
  ENDIF()
ENDFOREACH()

MESSAGE(STATUS "LIBS: ${PhysX_LIBRARIES}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PhysX  DEFAULT_MSG  PhysX_LIBRARIES PhysX_INCLUDE_DIR PhysX_LIBRARY_DIR)

MARK_AS_ADVANCED(PhysX_INCLUDE_DIR PhysX_LIBRARIES PhysX_LIBRARY_DIR)