# Locate the PhysX SDK

FIND_PATH(PHYSX_STANDARD PxPhysicsAPI.h
  PATH_SUFFIXES include Include
  PATHS
  ${PHYSX_HOME}/*
  $ENV{PHYSX_HOME}/*
)

FIND_PATH(PHYSX_SHARED foundation/Px.h
  PATH_SUFFIXES include Include
  PATHS
  ${PHYSX_HOME}/*
  $ENV{PHYSX_HOME}/*
)

LIST(APPEND
  PhysX_INCLUDE_DIR
  ${PHYSX_STANDARD}
  ${PHYSX_SHARED}
)

IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(LIBFOLDERSUFFIX "64")
  SET(PHYSXPREFIX "_x64")
ELSE()
  SET(LIBFOLDERSUFFIX "32")
ENDIF()

IF (NOT PhysX_LIBRARY_DIR)
  IF (MSVC)
    IF (MSVC_VERSION EQUAL 1700)
      SET(LIBFOLDER vc11win${LIBFOLDERSUFFIX})
    ELSEIF (MSVC_VERSION EQUAL 1800)
      SET(LIBFOLDER vc12win${LIBFOLDERSUFFIX})
    ELSEIF (MSVC_VERSION EQUAL 1900 OR MSVC_VERSION EQUAL 1910)
      SET(LIBFOLDER vc14win${LIBFOLDERSUFFIX})
    ENDIF()
  ELSEIF (CMAKE_HOST_APPLE)
    SET(LIBFOLDER osx${LIBFOLDERSUFFIX})
  ELSE()
    SET(LIBFOLDER linux${LIBFOLDERSUFFIX})
  ENDIF()

  SET(PhysX_LIBRARY_DIR ${PhysX_INCLUDE_DIR}/../Lib/${LIBFOLDER})
ENDIF()

FIND_LIBRARY(PhysX_LIBRARY_RELEASE PhysX3${PHYSXPREFIX}
  PATH_SUFFIXES Bin/${LIBFOLDER} Lib/${LIBFOLDER}
  PATHS
  ${PHYSX_HOME}/*
)

FIND_LIBRARY(PhysX_LIBRARY_PROFILE PhysX3PROFILE${PHYSXPREFIX}
  PATH_SUFFIXES Bin/${LIBFOLDER} Lib/${LIBFOLDER}
  PATHS
  ${PHYSX_HOME}/*
)

FIND_LIBRARY(PhysX_LIBRARY_DEBUG PhysX3DEBUG${PHYSXPREFIX}
  PATH_SUFFIXES Bin/${LIBFOLDER} Lib/${LIBFOLDER}
  PATHS
  ${PHYSX_HOME}/*
)

SET(PhysX_LIBRARIES
  debug ${PhysX_LIBRARY_DEBUG}
)

IF (PhysX_PROFILE)
  SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} optimized ${PhysX_LIBRARY_PROFILE})
ELSE()
  SET(PhysX_LIBRARIES ${PhysX_LIBRARIES} optimized ${PhysX_LIBRARY_RELEASE})
ENDIF()

SET(NECESSARY_COMPONENTS "")
FOREACH(component ${PhysX_FIND_COMPONENTS})
  FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_DEBUG PhysX3${component}DEBUG${PHYSXPREFIX} PhysX3${component}DEBUG PhysX${component}DEBUG ${component}DEBUG ${component}DEBUG${PHYSXPREFIX} 
    PATH_SUFFIXES bin/${LIBFOLDER} lib/${LIBFOLDER} Bin/${LIBFOLDER} Lib/${LIBFOLDER}
    PATHS
    ${PHYSX_HOME}/*
  )
  IF (PhysX_LIBRARY_COMPONENT_${component}_DEBUG)
    SET(PhysX_LIBRARIES
      ${PhysX_LIBRARIES}
      debug "${PhysX_LIBRARY_COMPONENT_${component}_DEBUG}"
    )
  ENDIF()

  FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_PROFILE PhysX3${component}PROFILE${PHYSXPREFIX} PhysX3${component}PROFILE PhysX${component}PROFILE ${component}PROFILE ${component}PROFILE${PHYSXPREFIX} 
    PATH_SUFFIXES bin/${LIBFOLDER} lib/${LIBFOLDER} Bin/${LIBFOLDER} Lib/${LIBFOLDER}
    PATHS
    ${PHYSX_HOME}/*
  )

  FIND_LIBRARY(PhysX_LIBRARY_COMPONENT_${component}_RELEASE PhysX3${component}${PHYSXPREFIX} PhysX3${component} PhysX${component} ${component} ${component}${PHYSXPREFIX}
    PATH_SUFFIXES bin/${LIBFOLDER} lib/${LIBFOLDER} Bin/${LIBFOLDER} Lib/${LIBFOLDER}
    PATHS
    ${PHYSX_HOME}/*
  )

  MARK_AS_ADVANCED(PhysX_LIBRARY_COMPONENT_${component}_DEBUG PhysX_LIBRARY_COMPONENT_${component}_PROFILE PhysX_LIBRARY_COMPONENT_${component}_RELEASE)
  
  IF (PhysX_PROFILE)
    SET(TARGET "PhysX_LIBRARY_COMPONENT_${component}_PROFILE")
  ELSE()
    SET(TARGET "PhysX_LIBRARY_COMPONENT_${component}_RELEASE")
  ENDIF()

  IF (${TARGET})
    SET(PhysX_LIBRARIES
      ${PhysX_LIBRARIES}
      optimized "${${TARGET}}"
    )
  ENDIF()

  SET(NECESSARY_COMPONENTS
    ${NECESSARY_COMPONENTS}
    PhysX_LIBRARY_COMPONENT_${component}_DEBUG ${TARGET}
  )
ENDFOREACH()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PhysX  DEFAULT_MSG  PhysX_INCLUDE_DIR PhysX_LIBRARY_DEBUG PhysX_LIBRARY_RELEASE ${NECESSARY_COMPONENTS})

MARK_AS_ADVANCED(PhysX_INCLUDE_DIR PhysX_LIBRARY_DIR PhysX_LIBRARY_DEBUG PhysX_LIBRARY_RELEASE PhysX_LIBRARY_PROFILE)
