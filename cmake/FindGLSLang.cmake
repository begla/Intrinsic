# Locate the GLSLang library

FIND_PATH(GLSLang_ROOT_DIR glslang/Include/Common.h
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang
)

FIND_LIBRARY(GLSLang_LIBRARY_DEBUG glslangd glslang
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install
)

FIND_LIBRARY(GLSLang_LIBRARY_RELEASE glslang
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install
)

FIND_LIBRARY(GLSLang_LIBRARY_HLSL_DEBUG HLSLd HLSL
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build_debug/install
)

FIND_LIBRARY(GLSLang_LIBRARY_HLSL_RELEASE HLSL
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install
)

FIND_LIBRARY(GLSLang_LIBRARY_OGL_DEBUG OGLCompilerd OGLCompiler
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build_debug/install
)

FIND_LIBRARY(GLSLang_LIBRARY_OGL_RELEASE OGLCompiler
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install
)

FIND_LIBRARY(GLSLang_LIBRARY_OSDEP_DEBUG OSDependentd OSDependent
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build_debug/glslang/OSDependent/Unix
)

FIND_LIBRARY(GLSLang_LIBRARY_OSDEP_RELEASE OSDependent
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Unix
)

FIND_LIBRARY(GLSLang_LIBRARY_SPIRV_DEBUG SPIRVd SPIRV
  PATH_SUFFIXES lib64 lib Debug
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build_debug/install
)

FIND_LIBRARY(GLSLang_LIBRARY_SPIRV_RELEASE SPIRV
  PATH_SUFFIXES lib64 lib Release
  PATHS
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV
  ${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install
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
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLSLang  DEFAULT_MSG  GLSLang_ROOT_DIR GLSLang_LIBRARY_DEBUG GLSLang_LIBRARY_RELEASE
  GLSLang_LIBRARY_HLSL_DEBUG GLSLang_LIBRARY_OGL_DEBUG GLSLang_LIBRARY_OSDEP_DEBUG GLSLang_LIBRARY_SPIRV_DEBUG
  GLSLang_LIBRARY_HLSL_RELEASE GLSLang_LIBRARY_OGL_RELEASE GLSLang_LIBRARY_OSDEP_RELEASE GLSLang_LIBRARY_SPIRV_RELEASE)

MARK_AS_ADVANCED(GLSLang_ROOT_DIR GLSLang_LIBRARIES
  GLSLang_LIBRARY_DEBUG GLSLang_LIBRARY_RELEASE
  GLSLang_LIBRARY_HLSL_DEBUG GLSLang_LIBRARY_OGL_DEBUG GLSLang_LIBRARY_OSDEP_DEBUG GLSLang_LIBRARY_SPIRV_DEBUG
  GLSLang_LIBRARY_HLSL_RELEASE GLSLang_LIBRARY_OGL_RELEASE GLSLang_LIBRARY_OSDEP_RELEASE GLSLang_LIBRARY_SPIRV_RELEASE
)
