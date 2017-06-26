# Locate Lua library
# This module defines
#  LuaJIT_FOUND, if false, do not try to link to Lua 
#  LuaJIT_LIBRARIES
#  LuaJIT_INCLUDE_DIR, where to find lua.h 
#
# Note that the expected include convention is
#  #include "lua.h"
# and not
#  #include <lua/lua.h>
# This is because, the lua location is not standardized and may exist
# in locations other than lua/

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)
#
# ################
# 2010 - modified for cronkite to find luajit instead of lua, as it was before.
# 2015 - modified to help Intrinsic find LuaJIT
#

FIND_PATH(LuaJIT_INCLUDE_DIR lua.h
  HINTS
  $ENV{LuaJIT_DIR}
  PATH_SUFFIXES include/luajit-2.0 include/luajit2.0 include/luajit include
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/lua
)

FIND_LIBRARY(LuaJIT_LIBRARY 
  NAMES luajit-51 luajit-5.1 luajit51 luajit lua-51 lua-5.1 lua51 lua
  HINTS
  $ENV{LuaJIT_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/lua
)

IF(LuaJIT_LIBRARY)
  # include the math library for Unix
  IF(UNIX AND NOT APPLE)
    FIND_LIBRARY(LuaJIT_MATH_LIBRARY m)
    SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY};${LuaJIT_MATH_LIBRARY}" CACHE STRING "Lua Libraries")
  # For Windows and Mac, don't need to explicitly include the math library
  ELSE(UNIX AND NOT APPLE)
    SET( LuaJIT_LIBRARIES "${LuaJIT_LIBRARY}" CACHE STRING "Lua Libraries")
  ENDIF(UNIX AND NOT APPLE)
ENDIF(LuaJIT_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LuaJIT  DEFAULT_MSG  LuaJIT_LIBRARIES LuaJIT_INCLUDE_DIR)

MARK_AS_ADVANCED(LuaJIT_INCLUDE_DIR LuaJIT_LIBRARIES LuaJIT_LIBRARY LuaJIT_MATH_LIBRARY)
