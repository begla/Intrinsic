cmake_minimum_required(VERSION 3.1)

project(Intrinsic)

set(BUILD_STANDALONE ON)
# TODO: Microprofile and IntrinsicEd are currently disabled for Linux builds
if(WIN32)
  set(BUILD_INTRINSICED ON)
  set(USE_MICROPROFILE ON)
endif()

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
set(COMPILE_ONLY OFF CACHE BOOL "Compile without linking")

set(INTR_GENERAL_LINK_FLAGS " ")
set(INTR_GENERAL_COMPILE_FLAGS " ")

add_definitions("-DMICROPROFILE_GPU_TIMERS_VULKAN")

if(WIN32)
  set(INTR_GENERAL_COMPILE_FLAGS "/Zm256 /EHa /W3")
  add_definitions("-D_SCL_SECURE_NO_WARNINGS")
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(INTR_GENERAL_COMPILE_FLAGS "${INTR_GENERAL_COMPILE_FLAGS} /Zi")
  set(INTR_GENERAL_LINK_FLAGS "${INTR_GENERAL_LINK_FLAGS} /DEBUG")

  if(INTR_FINAL_BUILD)
    add_definitions("-D_HAS_EXCEPTIONS=0")
    set(INTR_GENERAL_COMPILE_FLAGS "${INTR_GENERAL_COMPILE_FLAGS} /GL")
    set(INTR_GENERAL_LINK_FLAGS "${INTR_GENERAL_LINK_FLAGS} /LTCG")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

if(INTR_FINAL_BUILD)
  add_definitions("-D_INTR_FINAL_BUILD")
else()
  if(USE_MICROPROFILE)
    add_definitions("-D_INTR_PROFILING_ENABLED")
  endif()
  add_definitions("-D_INTR_LOGGING_ENABLED -D_INTR_ASSERTS_ENABLED")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (INTR_FINAL_BUILD)
else()
  set(PhysX_PROFILE ON)
endif()

find_package(GLSLang REQUIRED)
find_package(LuaJIT REQUIRED)
find_package(PhysX REQUIRED Common Cooking CharacterKinematic Extensions VisualDebuggerSDK)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
if (BUILD_INTRINSICED)
  find_package(Qt5 REQUIRED)
  find_package(FBX REQUIRED)
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

if(USE_MICROPROFILE)
  set(INTDEP_SOURCE_FILES
    ${INTDEP_SOURCE_FILES} 
    IntrinsicCore/src/microprofile.cpp
  )
endif()

# Qt5
if(BUILD_INTRINSICED)
  set(CMAKE_PREFIX_PATH ${Qt5_ROOT_DIR})
  find_package(Qt5Widgets REQUIRED)
  include_directories(${Qt5Widgets_INCLUDES})
  add_definitions(${Qt5Widgets_DEFINITIONS})

  foreach (srcFile ${MOC_SOURCES})
    set_source_files_properties (${srcFile} 
    PROPERTIES COMPILE_FLAGS "/Yustdafx_editor.h /FIstdafx_editor.h"
  )
  endforeach()
endif()

# Executables
if (BUILD_INTRINSICED AND NOT COMPILE_ONLY)
add_executable(IntrinsicEd ${INTR_ED_SOURCE_FILES} ${MOC_SOURCES} ${INTR_ED_HEADER_FILES} IntrinsicEd/IntrinsicEd.qrc IntrinsicEd/IntrinsicEd.rc)
set_target_properties(IntrinsicEd PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_NAME_RELEASE "IntrinsicEd"
  RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicEdDebug"
)
endif()

if (BUILD_STANDALONE AND NOT COMPILE_ONLY)
add_executable(Intrinsic ${INTR_SOURCE_FILES} ${INTR_HEADER_FILES} Intrinsic/Intrinsic.rc)
set_target_properties(Intrinsic PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
  RUNTIME_OUTPUT_NAME_RELEASE "Intrinsic"
  RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicDebug"
)
endif()

# Libs
add_library(IntrinsicCore ${INTR_CORE_SOURCE_FILES} ${INTR_CORE_C_SOURCE_FILES} ${INTDEP_SOURCE_FILES} ${INTR_CORE_HEADER_FILES})
add_library(IntrinsicRendererVulkan ${INTR_RENDERER_VULKAN_SOURCE_FILES} ${INTR_RENDERER_VULKAN_HEADER_FILES} ${INTR_RENDERER_VULKAN_DEP_SOURCE_FILES})
if (BUILD_INTRINSICED)
  add_library(IntrinsicAssetManagement ${INTASSET_SOURCE_FILES} ${INTASSET_HEADER_FILES})
endif()

set_target_properties(IntrinsicRendererVulkan PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicRendererVulkan PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
set_target_properties(IntrinsicCore PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicCore PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
if (BUILD_INTRINSICED)
  if (NOT COMPILE_ONLY)
    set_target_properties(IntrinsicEd PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
    set_target_properties(IntrinsicEd PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
  endif()
  set_target_properties(IntrinsicAssetManagement PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
  set_target_properties(IntrinsicAssetManagement PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
endif()
if (BUILD_STANDALONE AND NOT COMPILE_ONLY)
  set_target_properties(Intrinsic PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
  set_target_properties(Intrinsic PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
endif()

# Precompiled header files

# Editor
if(WIN32)
  set_source_files_properties(IntrinsicEd ${INTR_ED_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yustdafx_editor.h")
  set_source_files_properties(IntrinsicEd IntrinsicEd/src/stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx_editor.h")
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
  "${FBX_INCLUDE_DIR}"
  "dependencies/glm"
  "${GLSLang_INCLUDE_DIR}"
  "dependencies/rapidjson"
  "dependencies/"
  "dependencies/enkits"
  "${LuaJIT_INCLUDE_DIR}"
  "dependencies/sol"
  "${PhysX_INCLUDE_DIR}"
  "${SDL2_INCLUDE_DIR}"
  "dependencies/spirvcross/"
  "${Vulkan_INCLUDE_DIR}"
)

if(WIN32)
  add_definitions("-DVK_USE_PLATFORM_WIN32_KHR")
elseif(UNIX)
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

# Libraries
if(BUILD_INTRINSICED)
  set(INTR_ED_EXTERNAL_LIBS
    Qt5::Widgets
    ${FBX_LIBRARIES}
  )
endif()

set(INTR_EXTERNAL_LIBS
  # Vulkan
  ${Vulkan_LIBRARIES}

  # Lua
  ${LuaJIT_LIBRARIES}

  # SDL2
  ${SDL2_LIBRARIES}

  # PhysX
  ${PhysX_LIBRARIES}
)

if(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  set(INTR_EXTERNAL_LIBS "${INTR_EXTERNAL_LIBS}" 
    Threads::Threads

    # GLSLang
    ${GLSLang_LIBRARIES}
  )
else()
  set(INTR_EXTERNAL_LIBS "${INTR_EXTERNAL_LIBS}" 
    # GLSLang
    ${GLSLang_LIBRARIES}
  )
endif()

# Link libraries
if (NOT COMPILE_ONLY)
  # Core
  if (WIN32)
    target_link_libraries(IntrinsicCore wsock32 ws2_32)
    target_link_libraries(IntrinsicCore Dbghelp)
  endif()
  target_link_libraries(IntrinsicCore IntrinsicRendererVulkan)
  target_link_libraries(IntrinsicCore ${INTR_EXTERNAL_LIBS})

  # Intrinsic standalone
  if (BUILD_STANDALONE)
    target_link_libraries(Intrinsic IntrinsicCore)
  endif()

  # IntrinsicEd
  if (BUILD_INTRINSICED)
    target_link_libraries(IntrinsicEd IntrinsicCore)
    target_link_libraries(IntrinsicEd IntrinsicAssetManagement)
    target_link_libraries(IntrinsicEd ${INTR_ED_EXTERNAL_LIBS})
  endif()
endif()
