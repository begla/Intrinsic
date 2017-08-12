

FIND_PATH(LuaJIT_ROOT_DIR src/lua.h
  PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luajit
)

IF(NOT LuaJIT_ROOT_DIR)
  MESSAGE(FATAL_ERROR "cannot find luajit package; forgot submodule update --init?")
ELSE()
  ADD_SUBDIRECTORY(${LuaJIT_ROOT_DIR})

  SET(LuaJIT_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/dependencies/luajit ##for luaconf.h
    ${LuaJIT_ROOT_DIR}/src
  )

  LIST(APPEND LuaJIT_LIBRARIES
    liblua
  )

  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LuaJIT  DEFAULT_MSG  LuaJIT_LIBRARIES LuaJIT_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    LuaJIT_INCLUDE_DIR
  )

ENDIF()
