cmake_minimum_required(VERSION 3.8)

# Setup modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(cotire)

project(Intrinsic)

set(INTR_BUILD_STANDALONE_APP ON CACHE BOOL "Sets whether the standalone app should be build - or not")
set(INTR_BUILD_INTRINSICED ON CACHE BOOL "Sets whether the editor app should be build - or not")
set(INTR_USE_MICROPROFILE ON CACHE BOOL "Sets whether Microprofile support is enabled - or not")

if(WIN32)
  message("Setting up build process for WINDOWS...")
  set(INTR_PLATFORM_WINDOWS ON)
elseif(UNIX AND NOT APPLE)
  message("Setting up build process for LINUX...")
  set(INTR_PLATFORM_LINUX ON)
else()
  message(FATAL_ERROR "Platform not supported")
endif()

# TODO: Microprofile and IntrinsicEd are currently only support on Windows
if(NOT INTR_PLATFORM_WINDOWS)
  set(INTR_BUILD_INTRINSICED OFF)
  set(INTR_USE_MICROPROFILE OFF)
endif()

if(INTR_BUILD_INTRINSICED)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTOMOC_MOC_OPTIONS "-fstdafx_editor.h")
endif()

set(INTR_FINAL_BUILD OFF CACHE BOOL "Final build setting")

add_definitions("-DMICROPROFILE_GPU_TIMERS_VULKAN")

if(MSVC)
  set(INTR_GENERAL_COMPILE_FLAGS "/EHa /W3")
  add_definitions("-D_SCL_SECURE_NO_WARNINGS")
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(INTR_GENERAL_COMPILE_FLAGS "${INTR_GENERAL_COMPILE_FLAGS} /Zi /MP")
  set(INTR_GENERAL_LINK_FLAGS "${INTR_GENERAL_LINK_FLAGS} /DEBUG")

  if(INTR_FINAL_BUILD)
    add_definitions("-D_HAS_EXCEPTIONS=0")
    set(INTR_GENERAL_COMPILE_FLAGS "${INTR_GENERAL_COMPILE_FLAGS} /GL")
    set(INTR_GENERAL_LINK_FLAGS "${INTR_GENERAL_LINK_FLAGS} /LTCG")
  endif()
else("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" 
  OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
  
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

if(INTR_FINAL_BUILD)
  add_definitions("-D_INTR_FINAL_BUILD")
else()
  if(INTR_USE_MICROPROFILE)
    add_definitions("-D_INTR_PROFILING_ENABLED")
  endif()
  add_definitions("-D_INTR_LOGGING_ENABLED")
  add_definitions("-D_INTR_ASSERTS_ENABLED")
endif()

if (NOT INTR_FINAL_BUILD)
  set(PhysX_PROFILE ON)
endif()

find_package(GLSLang REQUIRED)
find_package(LuaJIT REQUIRED)
find_package(PhysX REQUIRED Common Cooking CharacterKinematic Extensions PxFoundation)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

if(INTR_BUILD_INTRINSICED)
  include(Qt5)
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  find_package(FBX REQUIRED)
endif()

if(INTR_PLATFORM_WINDOWS)
  find_package(BugSplat REQUIRED)
endif()

set(INTR_CORE_DEP_SOURCE_FILES 
  dependencies/spirvcross/spirv_glsl.cpp
  dependencies/spirvcross/spirv_cross.cpp
  dependencies/spirvcross/spirv_cfg.cpp
)

file(GLOB INTR_CORE_SOURCE_FILES IntrinsicCore/src/IntrinsicCore*.cpp IntrinsicRenderer/src/IntrinsicRenderer*.cpp )
file(GLOB INTR_CORE_HEADER_FILES IntrinsicCore/src/IntrinsicCore*.h IntrinsicRenderer/src/IntrinsicRenderer*.h )

set(INTR_CORE_C_SOURCE_FILES
  dependencies/tlsf/tlsf.c
)

file(GLOB INTR_HEADER_FILES Intrinsic/src/Intrinsic*.h)
file(GLOB INTR_SOURCE_FILES Intrinsic/src/Intrinsic*.cpp)

set(INTR_SOURCE_FILES Intrinsic/src/main.cpp ${INTR_SOURCE_FILES})

file(GLOB INTR_ED_SOURCE_FILES IntrinsicEd/src/IntrinsicEd*.cpp)
file(GLOB INTR_ED_HEADER_FILES IntrinsicEd/src/IntrinsicEd*.h)

set(INTR_ED_SOURCE_FILES IntrinsicEd/src/main.cpp ${INTR_ED_SOURCE_FILES})

file(GLOB INTASSET_SOURCE_FILES IntrinsicAssetManagement/src/IntrinsicAssetManagement*.cpp)
file(GLOB INTASSET_HEADER_FILES IntrinsicAssetManagement/src/IntrinsicAssetManagement*.h)

set(INTR_CORE_SOURCE_FILES IntrinsicCore/src/stdafx.cpp ${INTR_CORE_SOURCE_FILES})
set(INTR_ED_SOURCE_FILES IntrinsicEd/src/stdafx.cpp ${INTR_ED_SOURCE_FILES})
set(INTASSET_SOURCE_FILES IntrinsicAssetManagement/src/stdafx.cpp ${INTASSET_SOURCE_FILES})

set(INTDEP_SOURCE_FILES
  "dependencies/enkits/src/TaskScheduler.cpp")

if(INTR_USE_MICROPROFILE)
  set(INTDEP_SOURCE_FILES
    ${INTDEP_SOURCE_FILES} 
    dependencies/microprofile/microprofile.cpp
  )
endif()

if(INTR_BUILD_INTRINSICED)
  foreach (srcFile ${MOC_SOURCES})
    set_source_files_properties (${srcFile} 
    PROPERTIES COMPILE_FLAGS "/Yustdafx_editor.h /FIstdafx_editor.h"
  )
  endforeach()
endif()

# Executables
if (INTR_BUILD_INTRINSICED)
  add_executable(IntrinsicEd ${INTR_ED_SOURCE_FILES} ${MOC_SOURCES} ${INTR_ED_HEADER_FILES} IntrinsicEd/IntrinsicEd.qrc IntrinsicEd/IntrinsicEd.rc)
  set_target_properties(IntrinsicEd PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
    RUNTIME_OUTPUT_NAME_RELEASE "IntrinsicEd"
    RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicEdDebug"
  )
endif()

if (INTR_BUILD_STANDALONE_APP)
  add_executable(Intrinsic ${INTR_SOURCE_FILES} ${INTR_HEADER_FILES} Intrinsic/Intrinsic.rc)
  set_target_properties(Intrinsic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/app
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/app
    RUNTIME_OUTPUT_NAME_RELEASE "Intrinsic"
    RUNTIME_OUTPUT_NAME_DEBUG "IntrinsicDebug"
  )
endif()

# Libs
add_library(IntrinsicCore ${INTR_CORE_SOURCE_FILES} ${INTR_CORE_C_SOURCE_FILES} 
  ${INTDEP_SOURCE_FILES} ${INTR_CORE_HEADER_FILES} ${INTR_CORE_DEP_SOURCE_FILES})

if (INTR_BUILD_INTRINSICED)
  add_library(IntrinsicAssetManagement ${INTASSET_SOURCE_FILES} ${INTASSET_HEADER_FILES})
endif()

set_target_properties(IntrinsicCore PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
set_target_properties(IntrinsicCore PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})

if (INTR_BUILD_INTRINSICED)
  set_target_properties(IntrinsicEd PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
  set_target_properties(IntrinsicEd PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
  set_target_properties(IntrinsicAssetManagement PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
  set_target_properties(IntrinsicAssetManagement PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
endif()
if (INTR_BUILD_STANDALONE_APP)
  set_target_properties(Intrinsic PROPERTIES COMPILE_FLAGS ${INTR_GENERAL_COMPILE_FLAGS})
  set_target_properties(Intrinsic PROPERTIES LINK_FLAGS ${INTR_GENERAL_LINK_FLAGS})
endif()

# Library includes
set(INTR_DEPENDENCIES
  "dependencies/gli"
  "${FBX_INCLUDE_DIR}"
  "dependencies/glm"
  "${GLSLang_INCLUDE_DIR}"
  "dependencies/rapidjson/include"
  "dependencies/"
  "dependencies/enkits/src"
  "${LuaJIT_INCLUDE_DIR}"
  "dependencies/sol/single/sol"
  "dependencies/sparsepp"
  "${PhysX_INCLUDE_DIR}"
  "${SDL2_INCLUDE_DIR}"
  "dependencies/spirvcross/"
  "${Vulkan_INCLUDE_DIR}"
  "${BUGSPLAT_INCLUDE_DIR}"
  "dependencies/microprofile"
  "dependencies/tlsf"
)

if(INTR_PLATFORM_WINDOWS)
  add_definitions("-DVK_USE_PLATFORM_WIN32_KHR")
elseif(INTR_PLATFORM_LINUX)
  add_definitions("-DVK_USE_PLATFORM_XLIB_KHR")
endif()

include_directories(${INTR_DEPENDENCIES})

# Intrinsic includes
set(INTR_INCLUDES
  "IntrinsicEd/src"
  "IntrinsicCore/src"
  "IntrinsicRenderer/src"
  "IntrinsicAssetManagement/src"
)
include_directories(${INTR_INCLUDES})

# Libraries
if(INTR_BUILD_INTRINSICED)
  set(INTR_ED_EXTERNAL_LIBS
    Qt5::Widgets
    ${FBX_LIBRARIES}
  )
endif()

set(INTR_EXTERNAL_LIBS
  ${Vulkan_LIBRARIES}
  ${LuaJIT_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${PhysX_LIBRARIES}
  ${BUGSPLAT_LIBRARIES}
  ${GLSLang_LIBRARIES}
)

if(UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  set(INTR_EXTERNAL_LIBS "${INTR_EXTERNAL_LIBS}" 
    Threads::Threads)
endif()

# Link libraries
if (INTR_PLATFORM_WINDOWS)
  target_link_libraries(IntrinsicCore wsock32 ws2_32)
  target_link_libraries(IntrinsicCore Dbghelp)
endif()

target_link_libraries(IntrinsicCore ${INTR_EXTERNAL_LIBS})

if (INTR_BUILD_STANDALONE_APP)
  target_link_libraries(Intrinsic IntrinsicCore)
endif()

if (INTR_BUILD_INTRINSICED)
  target_link_libraries(IntrinsicEd IntrinsicCore)
  target_link_libraries(IntrinsicEd IntrinsicAssetManagement)
  target_link_libraries(IntrinsicEd ${INTR_ED_EXTERNAL_LIBS})
endif()

if (INTR_BUILD_INTRINSICED)
  set_target_properties(IntrinsicEd PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${COTIRE_CXX_PREFIX_HEADER_INIT} "IntrinsicEd/src/stdafx_editor.h")
  set_target_properties(IntrinsicAssetManagement PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${COTIRE_CXX_PREFIX_HEADER_INIT} "IntrinsicAssetManagement/src/stdafx_assets.h")
endif()
set_target_properties(IntrinsicCore PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "IntrinsicCore/src/stdafx.h")

cotire(IntrinsicCore)
if (INTR_BUILD_INTRINSICED)
  cotire(IntrinsicAssetManagement)
  cotire(IntrinsicEd)
endif()

if(MSVC)
  set_target_properties(Intrinsic PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ../app)
  set_target_properties(IntrinsicEd PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ../app)
endif()
