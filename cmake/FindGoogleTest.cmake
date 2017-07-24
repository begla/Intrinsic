# Locate the GLSLang library

FIND_PATH(GOOGLETEST_ROOT_DIR googletest/include/gtest/gtest.h
  PATHS
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/googletest
)

IF(NOT GOOGLETEST_ROOT_DIR)
  MESSAGE(FATAL_ERROR "cannot find googletest package; forgot submodule update --init?") 
ELSE()
  ADD_SUBDIRECTORY(${GOOGLETEST_ROOT_DIR})
  INCLUDE(FindPackageHandleStandardArgs)
ENDIF()
