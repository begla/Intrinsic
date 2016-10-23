cmake_minimum_required(VERSION 3.5)

project(Intrinsic)

if(!CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(GLOBAL_PROPERTY USE_FOLDERS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS "-fstdafx_editor.h")

set(INTR_FINAL_BUILD OFF CACHE BOOL "Final build setting")

set(INTR_GENERAL_LINK_FLAGS " ")
set(INTR_GENERAL_COMPILE_FLAGS " ")

add_definitions("-DMICROPROFILE_GPU_TIMERS_VULKAN")

if(WIN32)
  set(CMAKE_PREFIX_PATH "$ENV{INTR_QTDIR}")

  set(INTR_GENERAL_COMPILE_FLAGS "/Zm256 /EHa /W3")

  add_definitions("-D_SCL_SECURE_NO_WARNINGS")
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(VULKAN_INCLUDE_DIR "$ENV{VK_SDK_PATH}/Include")

  if(INTR_FINAL_BUILD)
    add_definitions("-D_INTR_FINAL_BUILD -D_HAS_EXCEPTIONS=0")
    set(INTR_GENERAL_COMPILE_FLAGS "${INTR_GENERAL_COMPILE_FLAGS} /GL")
    set(INTR_GENERAL_LINK_FLAGS "${INTR_GENERAL_LINK_FLAGS} /LTCG")
  endif()
else()
  set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/5.7/gcc_64")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

  set(VULKAN_INCLUDE_DIR "$ENV{HOME}/VulkanSDK/1.0.30.0/x86_64/include")
endif()

file(GLOB INTR_RENDERER_VULKAN_SOURCE_FILES IntrinsicRendererVulkan/src/IntrinsicRendererVulkan*.cpp)
file(GLOB INTR_RENDERER_VULKAN_HEADER_FILES IntrinsicRendererVulkan/src/IntrinsicRendererVulkan*.h)

set(INTR_RENDERER_VULKAN_SOURCE_FILES IntrinsicRendererVulkan/src/stdafx.cpp IntrinsicRendererVulkan/src/stdafx_vulkan.h ${INTR_RENDERER_VULKAN_SOURCE_FILES})

set(INTR_RENDERER_VULKAN_DEP_SOURCE_FILES 
  dependencies/spirvcross/spirv_glsl.cpp
  dependencies/spirvcross/spirv_cross.cpp
)

file(GLOB INTR_CORE_SOURCE_FILES IntrinsicCore/src/IntrinsicCore*.cpp)
file(GLOB INTR_CORE_HEADER_FILES IntrinsicCore/src/IntrinsicCore*.h)

set(INTR_CORE_SOURCE_FILES IntrinsicCore/src/stdafx.cpp IntrinsicCore/src/stdafx.h ${INTR_CORE_SOURCE_FILES})

set(INTR_CORE_C_SOURCE_FILES
  IntrinsicCore/src/tlsf.c
)

file(GLOB INTR_HEADER_FILES Intrinsic/src/Intrinsic*.h)
file(GLOB INTR_SOURCE_FILES Intrinsic/src/Intrinsic*.cpp)

set(INTR_SOURCE_FILES Intrinsic/src/main.cpp ${INTR_SOURCE_FILES})

file(GLOB INTR_ED_SOURCE_FILES IntrinsicEd/src/IntrinsicEd*.cpp)
file(GLOB INTR_ED_HEADER_FILES IntrinsicEd/src/IntrinsicEd*.h)

set(INTR_ED_SOURCE_FILES IntrinsicEd/src/stdafx.cpp IntrinsicEd/src/main.cpp IntrinsicEd/src/stdafx_editor.h ${INTR_ED_SOURCE_FILES})

file(GLOB INTASSET_SOURCE_FILES IntrinsicAssetManagement/src/IntrinsicAssetManagement*.cpp)
file(GLOB INTASSET_HEADER_FILES IntrinsicAssetManagement/src/IntrinsicAssetManagement*.h)

set(INTASSET_SOURCE_FILES IntrinsicAssetManagement/src/stdafx.cpp IntrinsicAssetManagement/src/stdafx_assets.h ${INTASSET_SOURCE_FILES})

set(INTDEP_SOURCE_FILES
  "dependencies/enkits/TaskScheduler.cpp")

if(WIN32)
set(INTDEP_SOURCE_FILES
  ${INTDEP_SOURCE_FILES} 
  IntrinsicCore/src/microprofile.cpp
)
endif()

# Qt5
find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

foreach (srcFile ${MOC_SOURCES})
  set_source_files_properties (${srcFile} 
  PROPERTIES COMPILE_FLAGS "/Yustdafx_editor.h /FIstdafx_editor.h"
)
endforeach()

# Executables
if(WIN32)
add_executable(IntrinsicEd ${INTR_ED_SOURCE_FILES} ${MOC_SOURCES} ${INTR_ED_HEADER_FILES} IntrinsicEd/IntrinsicEd.qrc IntrinsicEd/IntrinsicEd.rc)
set_target_properties(IntrinsicEd PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_NAME_RELEASE "IntrinsicEd"
  RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicEdDebug"
)
endif()

add_executable(Intrinsic ${INTR_SOURCE_FILES} ${INTR_HEADER_FILES} Intrinsic/Intrinsic.rc)
set_target_properties(Intrinsic PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_NAME_RELEASE "Intrinsic"
  RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicDebug"
)

# Libs
add_library(IntrinsicCore ${INTR_CORE_SOURCE_FILES} ${INTR_CORE_C_SOURCE_FILES} ${INTDEP_SOURCE_FILES} ${INTR_CORE_HEADER_FILES})
add_library(IntrinsicRendererVulkan ${INTR_RENDERER_VULKAN_SOURCE_FILES} ${INTR_RENDERER_VULKAN_HEADER_FILES} ${INTR_RENDERER_VULKAN_DEP_SOURCE_FILES})
if(WIN32)
add_library(IntrinsicAssetManagement ${INTASSET_SOURCE_FILES} ${INTASSET_HEADER_FILES})
endif(WIN32)

if (WIN32)
set_target_properties(IntrinsicEd PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicEd PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
set_target_properties(IntrinsicRendererVulkan PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
set_target_properties(IntrinsicAssetManagement PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicAssetManagement PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
endif()
set_target_properties(Intrinsic PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(Intrinsic PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
set_target_properties(IntrinsicCore PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicCore PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
set_target_properties(IntrinsicRendererVulkan PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})

# Precompiled header files

# Editor
if(WIN32)
  set_source_files_properties(IntrinsicEd ${INTR_ED_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yustdafx_editor.h")
  set_source_files_properties(IntrinsicEd IntrinsicEd/src/stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx_editor.h")
# Standalone
# Core
  set_source_files_properties(${INTR_CORE_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
  set_source_files_properties(IntrinsicCore/src/stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
# Rend. Vulk.
  set_source_files_properties(${INTR_RENDERER_VULKAN_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yustdafx_vulkan.h")
  set_source_files_properties(IntrinsicRendererVulkan/src/stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx_vulkan.h")
# Asset Management
  set_source_files_properties(${INTASSET_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yustdafx_assets.h")
  set_source_files_properties(IntrinsicAssetManagement/src/stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx_assets.h")
endif()

# Library includes
set(INTR_DEPENDENCIES
  "dependencies/gli"
  "../Intrinsic_Dependencies/dependencies/fbx/include"
  "dependencies/glm"
  "dependencies/glslang"
  "dependencies/rapidjson"
  "dependencies/"
  "dependencies/enkits"
  "dependencies/lua/include"
  "dependencies/sol"
  "../Intrinsic_Dependencies/dependencies/physx/include"
  "dependencies/spirvcross/"
  "${VULKAN_INCLUDE_DIR}"
)

if(WIN32)
  set(INTR_DEPENDENCIES ${INTR_DEPENDENCIES}
    "dependencies/sdl/include"
  )
  add_definitions("-DVK_USE_PLATFORM_WIN32_KHR")
elseif(UNIX)
  INCLUDE(FindPkgConfig)
  PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
  set(INTR_DEPENDENCIES ${INTR_DEPENDENCIES}
    "${SDL2_INCLUDE_DIRS}"
  )
  add_definitions("-DVK_USE_PLATFORM_XLIB_KHR")
endif()

include_directories(${INTR_DEPENDENCIES})

# Intrinsic includes
set(INTR_INCLUDES
  "IntrinsicEd/src"
  "IntrinsicCore/src"
  "IntrinsicRendererVulkan/src"
  "IntrinsicAssetManagement/src"
)
include_directories(${INTR_INCLUDES})

set(PHYSX_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/physx/lib)
set(LUAJIT_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/dependencies/lua)

if(WIN32)
  set(VULKAN_LIBRARY_DIRS $ENV{VK_SDK_PATH}/Bin)
  set(SDL2_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/dependencies/sdl/lib/x64")
else()
  set(VULKAN_LIBRARY_DIRS "$ENV{HOME}/VulkanSDK/1.0.30.0/x86_64/lib")
endif()

set(FBX_LIBRARY_DEBUG_DIRS ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx/lib/debug)
set(FBX_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/../Intrinsic_Dependencies/dependencies/fbx/lib/release)

set(GLSLANG_LIBRARY_DIRS
  # Windows
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/Release"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl/Release"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL/Release"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV/Release"

  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/Debug"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/hlsl/Debug"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/OGLCompilersDLL/Debug"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/SPIRV/Debug"

  # Unix
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/install/lib"
)

if(WIN32)
set(GLSLANG_LIBRARY_DIRS ${GLSLANG_LIBRARY_DIRS}
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows/Release"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Windows/Debug"
)
else()
set(GLSLANG_LIBRARY_DIRS ${GLSLANG_LIBRARY_DIRS}
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Unix/Release"
  "${CMAKE_SOURCE_DIR}/dependencies/glslang/build/glslang/OSDependent/Unix/Debug"
)
endif()

# GLSLang
find_library(LIB_GLSLANG NAMES glslang HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_HLSL NAMES HLSL HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_OGL NAMES OGLCompiler HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_OSDEP NAMES OSDependent HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_SPIRV NAMES SPIRV HINTS ${GLSLANG_LIBRARY_DIRS})

find_library(LIB_GLSLANG_DEBUG NAMES glslangd HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_HLSL_DEBUG NAMES HLSLd HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_OGL_DEBUG NAMES OGLCompilerd HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_OSDEP_DEBUG NAMES OSDependentd HINTS ${GLSLANG_LIBRARY_DIRS})
find_library(LIB_GLSLANG_SPIRV_DEBUG NAMES SPIRVd HINTS ${GLSLANG_LIBRARY_DIRS})

# FBX
find_library(LIB_FBX_RELEASE NAMES libfbxsdk HINTS ${FBX_LIBRARY_DEBUG_DIRS})
find_library(LIB_FBX_DEBUG NAMES libfbxsdk HINTS ${FBX_LIBRARY_DIRS})

# SDL
find_library(LIB_SDL2 NAMES SDL2 HINTS ${SDL2_LIBRARY_DIRS})
find_library(LIB_SDL2_MAIN NAMES SDL2main HINTS ${SDL2_LIBRARY_DIRS})

# Vulkan
find_library(LIB_VULKAN NAMES vulkan-1 vulkan HINTS ${VULKAN_LIBRARY_DIRS})

# Lua
find_library(LIB_LUAJIT NAMES lua51 luajit HINTS ${LUAJIT_LIBRARY_DIRS})

# PhysX Release
find_library(LIB_PHYSX3_RELEASE NAMES PhysX3_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COOKING_RELEASE NAMES PhysX3Cooking_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_CHAR_KINEMATIC_RELEASE NAMES PhysX3CharacterKinematic_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_EXTENSIONS_RELEASE NAMES PhysX3Extensions HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COMMON_RELEASE NAMES PhysX3Common_x64 HINTS ${PHYSX_LIBRARY_DIRS})

# PhysX Profile
find_library(LIB_PHYSX3_PROFILE NAMES PhysX3PROFILE_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COOKING_PROFILE NAMES PhysX3CookingPROFILE_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_CHAR_KINEMATIC_PROFILE NAMES PhysX3CharacterKinematicPROFILE_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_EXTENSIONS_PROFILE NAMES PhysX3ExtensionsPROFILE HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COMMON_PROFILE NAMES PhysX3CommonPROFILE_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_VIS_DEBUGGER_PROFILE NAMES PhysXVisualDebuggerSDKPROFILE HINTS ${PHYSX_LIBRARY_DIRS})

# PhysX Debug
find_library(LIB_PHYSX3_DEBUG NAMES PhysX3DEBUG_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COOKING_DEBUG NAMES PhysX3CookingDEBUG_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_CHAR_KINEMATIC_DEBUG NAMES PhysX3CharacterKinematicDEBUG_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_EXTENSIONS_DEBUG NAMES PhysX3ExtensionsDEBUG HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_COMMON_DEBUG NAMES PhysX3CommonDEBUG_x64 HINTS ${PHYSX_LIBRARY_DIRS})
find_library(LIB_PHYSX3_VIS_DEBUGGER_DEBUG NAMES PhysXVisualDebuggerSDKDEBUG HINTS ${PHYSX_LIBRARY_DIRS})

# PThread
find_library(LIB_PTHREAD NAMES pthread)

# Libraries
set(INTR_ED_EXTERNAL_LIBS
  Qt5::Widgets
  optimized "${LIB_FBX_RELEASE}"
  debug "${LIB_FBX_DEBUG}"
)

if(INTR_FINAL_BUILD)
  set(PHYSX_RELEASE_LIBS
    optimized "${LIB_PHYSX3_RELEASE}"
    optimized "${LIB_PHYSX3_COOKING_RELEASE}"
    optimized "${LIB_PHYSX3_CHAR_KINEMATIC_RELEASE}"
    optimized "${LIB_PHYSX3_EXTENSIONS_RELEASE}"
    optimized "${LIB_PHYSX3_COMMON_RELEASE}"
  )
else()
  set(PHYSX_RELEASE_LIBS
    optimized "${LIB_PHYSX3_PROFILE}"
    optimized "${LIB_PHYSX3_COOKING_PROFILE}"
    optimized "${LIB_PHYSX3_CHAR_KINEMATIC_PROFILE}"
    optimized "${LIB_PHYSX3_EXTENSIONS_PROFILE}"
    optimized "${LIB_PHYSX3_COMMON_PROFILE}"
    optimized "${LIB_PHYSX3_VIS_DEBUGGER_PROFILE}"
  )
endif()

set(INTR_EXTERNAL_LIBS
  # Vulkan
  "${LIB_VULKAN}"

  # Lua
  "${LIB_LUAJIT}"

  # SDL2
  "${LIB_SDL2}"
  "${LIB_SDL2_MAIN}"

  # PhysX
  ${PHYSX_RELEASE_LIBS}

  debug "${LIB_PHYSX3_DEBUG}"
  debug "${LIB_PHYSX3_COOKING_DEBUG}"
  debug "${LIB_PHYSX3_CHAR_KINEMATIC_DEBUG}"
  debug "${LIB_PHYSX3_EXTENSIONS_DEBUG}"
  debug "${LIB_PHYSX3_COMMON_DEBUG}"
  debug "${LIB_PHYSX3_VIS_DEBUGGER_DEBUG}"
)

if(UNIX)
  set(INTR_EXTERNAL_LIBS "${INTR_EXTERNAL_LIBS}" 
    "${LIB_PTHREAD}"

    # GLSLang
    "${LIB_GLSLANG}"
    "${LIB_GLSLANG_HLSL}"
    "${LIB_GLSLANG_OGL}"
    "${LIB_GLSLANG_OSDEP}"
    "${LIB_GLSLANG_SPIRV}"
  )
else()
  set(INTR_EXTERNAL_LIBS "${INTR_EXTERNAL_LIBS}" 
    "${LIB_WINSOCKS}"

    # GLSLang
    optimized "${LIB_GLSLANG}"
    optimized "${LIB_GLSLANG_HLSL}"
    optimized "${LIB_GLSLANG_OGL}"
    optimized "${LIB_GLSLANG_OSDEP}"
    optimized "${LIB_GLSLANG_SPIRV}"

    debug "${LIB_GLSLANG_DEBUG}"
    debug "${LIB_GLSLANG_HLSL_DEBUG}"
    debug "${LIB_GLSLANG_OGL_DEBUG}"
    debug "${LIB_GLSLANG_OSDEP_DEBUG}"
    debug "${LIB_GLSLANG_SPIRV_DEBUG}"
  )
endif()

# Link libraries
# Core
if (WIN32)
  target_link_libraries(IntrinsicCore wsock32 ws2_32)
endif()
target_link_libraries(IntrinsicCore IntrinsicRendererVulkan)
target_link_libraries(IntrinsicCore ${INTR_EXTERNAL_LIBS})

# Intrinsic standalone
target_link_libraries(Intrinsic IntrinsicCore)

# IntrinsicEd
if(WIN32)
target_link_libraries(IntrinsicEd IntrinsicCore)
target_link_libraries(IntrinsicEd IntrinsicAssetManagement)
target_link_libraries(IntrinsicEd ${INTR_ED_EXTERNAL_LIBS})
endif()
