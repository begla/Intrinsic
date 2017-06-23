// Copyright 2017 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// Settings
#define _INTR_MAX_PLAYER_COUNT 4u

// Components
#define _INTR_MAX_ENTITY_COUNT 10240u
#define _INTR_MAX_NODE_COMPONENT_COUNT 10240u
#define _INTR_MAX_MESH_COMPONENT_COUNT 10240u
#define _INTR_MAX_CAMERA_COMPONENT_COUNT 1024u
#define _INTR_MAX_SCRIPT_COMPONENT_COUNT 1024u
#define _INTR_MAX_LIGHT_COMPONENT_COUNT 10240u
#define _INTR_MAX_RIGID_BODY_COMPONENT_COUNT 10240u
#define _INTR_MAX_IRRADIANCE_PROBE_COMPONENT_COUNT 1024u
#define _INTR_MAX_DECAL_COMPONENT_COUNT 10240u
#define _INTR_MAX_CAMERA_CONTROLLER_COMPONENT_COUNT 1024u
#define _INTR_MAX_CHARACTER_CONTROLLER_COMPONENT_COUNT 1024u
#define _INTR_MAX_SWARM_COMPONENT_COUNT 1024u
#define _INTR_MAX_PLAYER_COMPONENT_COUNT 8u
#define _INTR_MAX_POST_EFFECT_CONTROLLER_COMPONENT_COUNT 1024u

// General objects
#define _INTR_MAX_FRUSTUM_COUNT 1024u
#define _INTR_MAX_GPU_PROGRAM_COUNT 1024u
#define _INTR_MAX_PIPELINE_COUNT 1024u
#define _INTR_MAX_RENDER_PASS_COUNT 1024u
#define _INTR_MAX_MATERIAL_COUNT 1024u
#define _INTR_MAX_VERTEX_LAYOUT_COUNT 8u
#define _INTR_MAX_PIPELINE_LAYOUT_COUNT 1024u
#define _INTR_MAX_BUFFER_COUNT 1024u
#define _INTR_MAX_DRAW_CALL_COUNT 10240u
#define _INTR_MAX_COMPUTE_CALL_COUNT 1024u
#define _INTR_MAX_FRAMEBUFFER_COUNT 1024u
#define _INTR_MAX_IMAGE_COUNT 1024u
#define _INTR_MAX_MESH_COUNT 1024u
#define _INTR_MAX_POST_EFFECT_COUNT 1024u
#define _INTR_MAX_SCRIPT_COUNT 1024u
#define _INTR_MAX_EVENT_COUNT 1024u
#define _INTR_MAX_EVENT_LISTENER_COUNT 1024u
#define _INTR_MAX_MATERIAL_PASS_COUNT 256u

// Various
#define _INTR_CONCAT_(x, y) x##y
#define _INTR_CONCAT(x, y) _INTR_CONCAT_(x, y)

// Logging
#if defined(_INTR_LOGGING_ENABLED)
#define _INTR_LOG_INFO(x, ...)                                                 \
  Intrinsic::Core::Log::Manager::log(Intrinsic::Core::Log::LogLevel::kInfo, x, \
                                     ##__VA_ARGS__)
#define _INTR_LOG_WARNING(x, ...)                                              \
  Intrinsic::Core::Log::Manager::log(Intrinsic::Core::Log::LogLevel::kWarning, \
                                     x, ##__VA_ARGS__)
#define _INTR_LOG_ERROR(x, ...)                                                \
  Intrinsic::Core::Log::Manager::log(Intrinsic::Core::Log::LogLevel::kError,   \
                                     x, ##__VA_ARGS__)
#define _INTR_LOG_PUSH() Intrinsic::Core::Log::Manager::indent()
#define _INTR_LOG_POP() Intrinsic::Core::Log::Manager::unindent()
#else
#define _INTR_LOG_INFO(x, ...)
#define _INTR_LOG_WARNING(x, ...)
#define _INTR_LOG_ERROR(x, ...)
#define _INTR_LOG_PUSH()
#define _INTR_LOG_POP()
#endif // _INTR_LOGGING_ENABLED

// Names
#define _N(x) Name(#x)

// Memory management
#define _INTR_NEW(x, y) y = new (Tlsf::MainAllocator::allocate(sizeof(x))) x
#define _INTR_DELETE(x, y)                                                     \
  if (y != nullptr)                                                            \
  {                                                                            \
    y->~x();                                                                   \
    Tlsf::MainAllocator::free(y);                                              \
  }
#define _INTR_DELETE_SAFE(x, y)                                                \
  if (y != nullptr)                                                            \
  {                                                                            \
    y->~x();                                                                   \
    Tlsf::MainAllocator::free(y);                                              \
    y = nullptr;                                                               \
  }

// Data structures
#define _INTR_STRING                                                           \
  std::basic_string<char, std::char_traits<char>,                              \
                    Intrinsic::Core::StlAllocator<char>>
#define _INTR_STRING_STREAM                                                    \
  std::basic_stringstream<char, std::char_traits<char>,                        \
                          Intrinsic::Core::StlAllocator<char>>
#define _INTR_ARRAY(a) std::vector<a, Intrinsic::Core::StlAllocator<a>>
#define _INTR_STACK_ARRAY(a, b) std::array<a, b>
#define _INTR_HASH_MAP(a, b)                                                   \
  spp::sparse_hash_map<a, b, spp::spp_hash<a>, std::equal_to<a>,               \
                       Intrinsic::Core::StlAllocator<std::pair<const a, b>>>
#define _INTR_FSTREAM std::fstream
#define _INTR_IFSTREAM std::ifstream
#define _INTR_OFSTREAM std::ofstream
#define _INTR_OSTRINGSTREAM                                                    \
  std::basic_ostringstream<char, std::char_traits<char>,                       \
                           Intrinsic::Core::StlAllocator<char>>

// Timing macros
#if defined(_INTR_PROFILING_ENABLED)
#define _INTR_PROFILE_START() TimingHelper::timerStart()
#define _INTR_PROFILE_END(x)                                                   \
  {                                                                            \
    const uint32_t msPassed = Intrinsic::Core::TimingHelper::timerEnd();       \
    _INTR_LOG_INFO("%s finished in %ums", x, msPassed);                        \
  }

#define _INTR_PROFILE_AUTO(x)                                                  \
  TimingHelper::AutoTimer _INTR_CONCAT(timer, __COUNTER__) =                   \
      TimingHelper::AutoTimer(x)
#else
#define _INTR_PROFILE_START()
#define _INTR_PROFILE_END(x)
#define _INTR_PROFILE_AUTO(x)
#endif // _INTR_PROFILING_ENABLED

// Sorting
#define _INTR_SORT(a, b, c) std::sort(a, b, c)

// Assert related macros
#if defined(_INTR_ASSERTS_ENABLED)
#if defined(_WIN32)
#define _INTR_IS_DEBUGGER_ATTACHED() IsDebuggerPresent()
#else
#define _INTR_IS_DEBUGGER_ATTACHED() false
#endif // _WIN32
#if defined(_WIN32)
#define _INTR_ERROR_DIALOG(_msg)                                               \
  {                                                                            \
    const int result =                                                         \
        MessageBox(NULL, _msg, "Error", MB_ICONERROR | MB_ABORTRETRYIGNORE);   \
    \
switch(result)                                                                 \
    \
{                                                                       \
      \
case IDABORT : exit(0);                                                        \
      break;                                                                   \
      \
case IDRETRY : _INTR_DBG_BREAK();                                              \
      break;                                                                   \
      \
case IDCONTINUE : break;                                                       \
    \
}                                                                       \
  }
#define _INTR_ERROR_DIALOG_SIMPLE(_msg)                                        \
  {                                                                            \
    MessageBox(NULL, _msg, "Error", MB_ICONERROR | MB_OK);                     \
  }
#define _INTR_DBG_BREAK() __debugbreak()
#define _INTR_ASSERT(_expr)                                                    \
  do                                                                           \
  {                                                                            \
    if (!(_expr))                                                              \
    {                                                                          \
      _INTR_LOG_ERROR("Assert: \"%s\" Line: %d File: \"%s\"", #_expr,          \
                      __LINE__, __FILE__);                                     \
    }                                                                          \
  } while (0)
#else
#define _INTR_ERROR_DIALOG(_msg)
#define _INTR_ERROR_DIALOG_SIMPLE(_msg)
#define _INTR_ASSERT(_expr) assert(_expr)
#define _INTR_DBG_BREAK()
#endif // _WIN32
#else
#define _INTR_ERROR_DIALOG(_msg)
#define _INTR_ERROR_DIALOG_SIMPLE(_msg)
#define _INTR_IS_DEBUGGER_ATTACHED()
#define _INTR_ASSERT(_expr)
#define _INTR_DBG_BREAK()
#endif // _INTR_ASSERTS_ENABLED

// Allocation related macros
#define _INTR_MALLOC(a) malloc(a)
#define _INTR_FREE(a) free(a)
#define _INTR_FREE_SAFE(a)                                                     \
  free(a);                                                                     \
  a = nullptr

// Properties
#define _INTR_CREATE_PROP(_doc, _gd, _cat, _ed, _val, _ro, _int)               \
  Intrinsic::Core::JsonHelper::createProperty(_doc, _gd, _cat, _ed, _val, _ro, \
                                              _int)
#define _INTR_CREATE_PROP_ENUM(_doc, _gd, _cat, _ed, _val, _enum, _ro, _int)   \
  Intrinsic::Core::JsonHelper::createPropertyEnum(_doc, _gd, _cat, _ed, _val,  \
                                                  _enum, _ro, _int)
#define _INTR_CREATE_PROP_FLAGS(_doc, _gd, _cat, _ed, _val, _flags, _ro, _int) \
  Intrinsic::Core::JsonHelper::createPropertyFlags(_doc, _gd, _cat, _ed, _val, \
                                                   _flags, _ro, _int)

// Math
#define _INTR_EPSILON 1.0e-6f

// Atomics
#define _INTR_ATOMIC volatile uint64_t
#define _INTR_INTERL_COMPARE_SWAP(_dest, _exchange, _comp)                     \
  InterlockedCompareExchange(_dest, _exchange, _comp)

// Profiling
#if defined(_INTR_PROFILING_ENABLED)
#define _INTR_PROFILE_CPU(_group, _name)                                       \
  MICROPROFILE_SCOPEI(_group, _name, 0xff00ff)
#define _INTR_PROFILE_CPU_CUSTOM(_var) MICROPROFILE_SCOPE(_var)
#define _INTR_PROFILE_CPU_DEFINE(_var, _group, _name)                          \
  MICROPROFILE_DEFINE(_var, _group, _name, 0xff00ff)
#define _INTR_PROFILE_GPU(_name)                                               \
  MICROPROFILE_SCOPEGPUI(_name, 0xff00ff);                                     \
  _INTR_PROFILE_GPU_MARKER_REGION(_name)
#define _INTR_PROFILE_GPU_CUSTOM(_var, _name)                                  \
  MICROPROFILE_SCOPEGPU(_var);                                                 \
  _INTR_PROFILE_GPU_MARKER_REGION(_name)
#define _INTR_PROFILE_GPU_DEFINE(_var, _name)                                  \
  MICROPROFILE_DEFINE_GPU(_var, _name, 0xff00ff);
#define _INTR_PROFILE_COUNTER_ADD(_name, _count)                               \
  MICROPROFILE_COUNTER_ADD(_name, _count)
#define _INTR_PROFILE_COUNTER_SET(_name, _count)                               \
  MICROPROFILE_COUNTER_SET(_name, _count)
#define _INTR_PROFILE_COUNTER_SUB(_name, _count)                               \
  MICROPROFILE_COUNTER_SUB(_name, _count)

#define _INTR_PROFILE_DEFINE_LOCAL_COUNTER(_var, _name)                        \
  MICROPROFILE_DEFINE_LOCAL_COUNTER(_var, _name)
#define _INTR_PROFILE_COUNTER_LOCAL_ADD(_var, _count)                          \
  MICROPROFILE_COUNTER_LOCAL_ADD(_var, _count)
#define _INTR_PROFILE_COUNTER_LOCAL_SET(_var, _count)                          \
  MICROPROFILE_COUNTER_LOCAL_SET(_var, _count)
#define _INTR_PROFILE_COUNTER_LOCAL_SUB(_var, _count)                          \
  MICROPROFILE_COUNTER_LOCAL_SUB(_var, _count)
#else
#define _INTR_PROFILE_CPU(_group, _name)
#define _INTR_PROFILE_CPU_CUSTOM(_var)
#define _INTR_PROFILE_CPU_DEFINE(_var, _group, _name)
#define _INTR_PROFILE_GPU(_name)
#define _INTR_PROFILE_GPU_CUSTOM(_var, _name)
#define _INTR_PROFILE_GPU_DEFINE(_var, _name)
#define _INTR_PROFILE_COUNTER_ADD(_name, _count)
#define _INTR_PROFILE_COUNTER_SET(_name, _count)
#define _INTR_PROFILE_COUNTER_SUB(_name, _count)

#define _INTR_PROFILE_DEFINE_LOCAL_COUNTER(_var, _name)
#define _INTR_PROFILE_COUNTER_LOCAL_ADD(_var, _count)
#define _INTR_PROFILE_COUNTER_LOCAL_SET(_var, _count)
#define _INTR_PROFILE_COUNTER_LOCAL_SUB(_var, _count)
#endif // _INTR_PROFILING_ENABLED

// String ops
#if defined(_WIN32)
#define _INTR_STRING_TOK(a, b, c) strtok_s(a, b, c);
#else
#define _INTR_STRING_TOK(a, b, c) strtok_r(a, b, c);
#endif // _WIN32

// Various
#if defined(_WIN32)
#define _INTR_INLINE __forceinline
#else
#define _INTR_INLINE inline
#endif // _WIN32
