

FIND_PATH(LuaJIT_ROOT_DIR src/lua.h
  PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luajit-2.0
)
  message(STATUS "@@@ ${LuaJIT_ROOT_DIR}")


IF(NOT LuaJIT_ROOT_DIR)
  MESSAGE(FATAL_ERROR "cannot find luajit package; forgot submodule update --init?")
ELSE()
  include(ExternalProject)
  ExternalProject_Add(
    external_luajit
    CONFIGURE_COMMAND ""
    SOURCE_DIR ${LuaJIT_ROOT_DIR}
    BUILD_IN_SOURCE 1
    BUILD_COMMAND $(MAKE) clean
      COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) PREFIX=${CMAKE_INSTALL_PREFIX} install
  )

  add_library(luajit SHARED IMPORTED)
  set_property(TARGET luajit PROPERTY IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libluajit-5.1.so)
  add_dependencies(luajit
    external_luajit
  )

  SET(LuaJIT_INCLUDE_DIR ${LuaJIT_ROOT_DIR}/src)

  LIST(APPEND LuaJIT_LIBRARY
    luajit
  )

  IF(LuaJIT_LIBRARY)
    # include the math library for Unix
    IF(UNIX AND NOT APPLE)
      FIND_LIBRARY(LuaJIT_MATH_LIBRARY m)
      SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY};${LuaJIT_MATH_LIBRARY}" CACHE STRING "Lua Libraries")
    # For Windows and Mac, don't need to explicitly include the math library
    ELSE()
      SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY}" CACHE STRING "Lua Libraries")
    ENDIF(UNIX AND NOT APPLE)
  ENDIF(LuaJIT_LIBRARY)

  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LuaJIT  DEFAULT_MSG  LuaJIT_LIBRARIES LuaJIT_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    LuaJIT_INCLUDE_DIR
    LuaJIT_LIBRARIES
    LuaJIT_LIBRARY
    LuaJIT_MATH_LIBRARY
  )

ENDIF()


#FIND_PATH(LuaJIT_INCLUDE_DIR lua.h
  #HINTS
  #$ENV{LuaJIT_DIR}
  #PATH_SUFFIXES include/luajit-2.0 include/luajit2.0 include/luajit include
  #PATHS
  #${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lua
#)

#FIND_LIBRARY(LuaJIT_LIBRARY 
  #NAMES luajit-51 luajit-5.1 luajit51 luajit lua-51 lua-5.1 lua51 lua
  #HINTS
  #$ENV{LuaJIT_DIR}
  #PATH_SUFFIXES lib64 lib
  #PATHS
  #${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lua
#)

#IF(LuaJIT_LIBRARY)
  ## include the math library for Unix
  #IF(UNIX AND NOT APPLE)
    #FIND_LIBRARY(LuaJIT_MATH_LIBRARY m)
    #SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY};${LuaJIT_MATH_LIBRARY}" CACHE STRING "Lua Libraries")
  ## For Windows and Mac, don't need to explicitly include the math library
  #ELSE(UNIX AND NOT APPLE)
    #SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY}" CACHE STRING "Lua Libraries")
  #ENDIF(UNIX AND NOT APPLE)
#ENDIF(LuaJIT_LIBRARY)

#INCLUDE(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(LuaJIT  DEFAULT_MSG  LuaJIT_LIBRARIES LuaJIT_INCLUDE_DIR)

#MARK_AS_ADVANCED(LuaJIT_INCLUDE_DIR LuaJIT_LIBRARIES LuaJIT_LIBRARY LuaJIT_MATH_LIBRARY)
