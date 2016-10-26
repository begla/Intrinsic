# Locate Lua library
# This module defines
#  GLSLang_FOUND, if false, do not try to link to Lua 
#  GLSLang_LIBRARIES
#  GLSLang_INCLUDE_DIR, where to find lua.h 
#

FIND_PATH(GLSLang_INCLUDE_DIR glslang/Include/Common.h
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang
)

FIND_LIBRARY(GLSLang_LIBRARY_DEBUG glslangd
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_RELEASE glslang
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_HLSL_DEBUG HLSLd
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_HLSL_RELEASE HLSL
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_OGL_DEBUG OGLCompilerd
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_OGL_RELEASE OGLCompiler
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_OSDEP_DEBUG OSDependentd
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Unix
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_OSDEP_RELEASE OSDependent
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Unix
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_SPIRV_DEBUG SPIRVd
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

FIND_LIBRARY(GLSLang_LIBRARY_SPIRV_RELEASE SPIRV
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

SET(GLSLang_LIBRARIES
  debug ${GLSLang_LIBRARY_DEBUG}
  debug ${GLSLang_LIBRARY_HLSL_DEBUG}
  debug ${GLSLang_LIBRARY_OGL_DEBUG}
  debug ${GLSLang_LIBRARY_OSDEP_DEBUG}
  debug ${GLSLang_LIBRARY_SPIRV_DEBUG}
  optimized ${GLSLang_LIBRARY_RELEASE}
  optimized ${GLSLang_LIBRARY_HLSL_RELEASE}
  optimized ${GLSLang_LIBRARY_OGL_RELEASE}
  optimized ${GLSLang_LIBRARY_OSDEP_RELEASE}
  optimized ${GLSLang_LIBRARY_SPIRV_RELEASE}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLSLang  DEFAULT_MSG  GLSLang_INCLUDE_DIR GLSLang_LIBRARY_DEBUG GLSLang_LIBRARY_RELEASE
  GLSLang_LIBRARY_HLSL_DEBUG GLSLang_LIBRARY_OGL_DEBUG GLSLang_LIBRARY_OSDEP_DEBUG GLSLang_LIBRARY_SPIRV_DEBUG
  GLSLang_LIBRARY_HLSL_RELEASE GLSLang_LIBRARY_OGL_RELEASE GLSLang_LIBRARY_OSDEP_RELEASE GLSLang_LIBRARY_SPIRV_RELEASE)

MARK_AS_ADVANCED(GLSLang_INCLUDE_DIR GLSLang_LIBRARIES
  GLSLang_LIBRARY_DEBUG GLSLang_LIBRARY_RELEASE
  GLSLang_LIBRARY_HLSL_DEBUG GLSLang_LIBRARY_OGL_DEBUG GLSLang_LIBRARY_OSDEP_DEBUG GLSLang_LIBRARY_SPIRV_DEBUG
  GLSLang_LIBRARY_HLSL_RELEASE GLSLang_LIBRARY_OGL_RELEASE GLSLang_LIBRARY_OSDEP_RELEASE GLSLang_LIBRARY_SPIRV_RELEASE
)

