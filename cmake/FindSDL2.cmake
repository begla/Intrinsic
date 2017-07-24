# Locate SDL2

FIND_PATH(SDL2_ROOT_DIR CMakeLists.txt
  PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2
)

IF(NOT SDL2_ROOT_DIR)
  MESSAGE(FATAL_ERROR "cannot find SDL2 package; forgot submodule update --init?") 
ELSE()
#setup everything else
  ADD_SUBDIRECTORY(${SDL2_ROOT_DIR})

# Binary directory needs to come first
#  because SDL fallsback to a different SDL_config.h,
#  which will not work with the CMAKE system correctly
  SET(SDL2_INCLUDE_DIR
    ${SDL2_BINARY_DIR}/include #for SDL_config.h
    ${SDL2_ROOT_DIR}/include
  )

  SET(SDL2_LIBRARY          SDL2)
  SET(SDL2_LIBRARY_SDL2MAIN SDL2main)
  SET(SDL2_LIBRARIES
    ${SDL2_LIBRARY}
    ${SDL2_LIBRARY_SDL2MAIN}
  )

  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 DEFAULT_MSG 
    SDL2_INCLUDE_DIR
    SDL2_LIBRARY
    SDL2_LIBRARY_SDL2MAIN
  )

  MARK_AS_ADVANCED(
    SDL2_INCLUDE_DIR
    SDL2_LIBRARY
    SDL2_LIBRARY_SDL2MAIN
  )
ENDIF()
