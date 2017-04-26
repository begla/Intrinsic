// Copyright 2016 Benjamin Glatzel
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

namespace Intrinsic
{
namespace Core
{
namespace Dod
{
typedef uint8_t GenerationType;
typedef uint32_t IdType;

// Note: The last index is used to mark invalid ids and generation ids - thus -2
static const uint32_t maxGenerationIdValue = 254u; // == 2^8-2
static const uint32_t maxIdValue = 16777214u;      // == 2^24-2

// Global constants
enum
{
  kInvalidId = maxIdValue + 1u,
  kInvalidGenerationId = maxGenerationIdValue + 1u,
};

// Reference to a "any" data oriented object
struct Ref
{
  Ref() : _generation(kInvalidGenerationId), _id(kInvalidId) {}
  Ref(IdType p_Id, GenerationType p_GenerationId)
      : _generation(p_GenerationId), _id(p_Id)
  {
  }

  _INTR_INLINE bool isValid() const
  {
    return _id != kInvalidId && _generation != kInvalidGenerationId;
  }

  _INTR_INLINE bool operator==(const Ref& p_Rhs) const
  {
    return _id == p_Rhs._id && _generation == p_Rhs._generation;
  }

  _INTR_INLINE bool operator!=(const Ref& p_Rhs) const
  {
    return !(*this == p_Rhs);
  }

  uint32_t _generation : 8;
  uint32_t _id : 24;
};

#define _INTR_REF_DEF(_type)                                                   \
  struct _type                                                                 \
  {                                                                            \
    _INTR_INLINE _type() {}                                                    \
    _INTR_INLINE _type(const Dod::Ref& val) { v = val; }                       \
                                                                               \
    _INTR_INLINE bool isValid() const { return v.isValid(); }                  \
    _INTR_INLINE Dod::IdType _id() { return v._id; }                           \
                                                                               \
    Dod::Ref v;                                                                \
  }
#define _INTR_REF_ARRAY_DEF(_refType, _type)                                   \
  struct _type                                                                 \
  {                                                                            \
    _INTR_INLINE _type() {}                                                    \
    _INTR_INLINE _type(const _INTR_ARRAY(Dod::Ref) & val) { v = val; }         \
                                                                               \
    _INTR_INLINE Dod::Ref& operator[](int32_t i) { return v[i]; }              \
    _INTR_INLINE const Dod::Ref& operator[](int32_t i) const { return v[i]; }  \
    _INTR_INLINE uint32_t size() const { return (uint32_t)v.size(); }          \
    _INTR_INLINE void push_back(const _refType& val) { v.push_back(val.v); }   \
                                                                               \
    _INTR_ARRAY(Dod::Ref) v;                                                   \
  }

// Typedefs
typedef _INTR_ARRAY(Ref) RefArray;

// <-

// Function ptr typedefs
typedef void (*ManagerCompileDescriptorFunction)(Ref, bool, rapidjson::Value&,
                                                 rapidjson::Document&);
typedef void (*ManagerInitFromDescriptorFunction)(Ref, rapidjson::Value&);
typedef Ref (*ManagerCreateFunction)(Ref);
typedef void (*ManagerDestroyFunction)(Ref);
typedef bool (*ManagerIsAliveFunction)(Ref);
typedef uint32_t (*ManagerGetActiveResourceCountFunction)();
typedef Ref (*ManagerGetActiveResourceAtIndexFunction)(uint32_t);
typedef void (*ManagerCreateResourcesFunction)(const RefArray&);
typedef void (*ManagerDestroyResourcesFunction)(const RefArray&);
typedef void (*ManagerResetToDefaultFunction)(Ref);
typedef void (*ManagerPropertyUpdateFinishedFunction)(Ref);
typedef void (*ManagerInsertionDeletionFinishedFunction)();

// <-

// Manager interface
struct ManagerEntry
{
  ManagerEntry()
      : createFunction(nullptr), destroyFunction(nullptr),
        createResourcesFunction(nullptr), destroyResourcesFunction(nullptr),
        isAliveFunction(nullptr), getActiveResourceCountFunction(nullptr),
        getActiveResourceAtIndexFunction(nullptr),
        resetToDefaultFunction(nullptr),
        onPropertyUpdateFinishedFunction(nullptr),
        onInsertionDeletionFinishedAction(nullptr)
  {
  }

  ManagerCreateFunction createFunction;
  ManagerDestroyFunction destroyFunction;
  ManagerCreateResourcesFunction createResourcesFunction;
  ManagerDestroyResourcesFunction destroyResourcesFunction;
  ManagerIsAliveFunction isAliveFunction;
  ManagerGetActiveResourceCountFunction getActiveResourceCountFunction;
  ManagerGetActiveResourceAtIndexFunction getActiveResourceAtIndexFunction;
  ManagerResetToDefaultFunction resetToDefaultFunction;
  ManagerPropertyUpdateFinishedFunction onPropertyUpdateFinishedFunction;
  ManagerInsertionDeletionFinishedFunction onInsertionDeletionFinishedAction;
};

// <-

// Property interface
struct PropertyCompilerEntry
{
  PropertyCompilerEntry() : compileFunction(nullptr), initFunction(nullptr) {}

  ManagerCompileDescriptorFunction compileFunction;
  ManagerInitFromDescriptorFunction initFunction;
  Ref ref;
};

// <-

// Base for all date oriented manager classes
template <uint32_t IdCount, class DataType> struct ManagerBase
{
  _INTR_INLINE static bool isAlive(Ref p_Ref)
  {
    return _generations[p_Ref._id] == p_Ref._generation;
  }

  // <-

  _INTR_INLINE static uint32_t getActiveResourceCount()
  {
    return (uint32_t)_activeRefs.size();
  }
  _INTR_INLINE static Ref getActiveResourceAtIndex(uint32_t p_Idx)
  {
    return _activeRefs[p_Idx];
  }

  static _INTR_ARRAY(Ref) _activeRefs;

protected:
  _INTR_INLINE static void _initManager()
  {
    _freeIds.reserve(IdCount);
    _activeRefs.reserve(IdCount);
    _generations.resize(IdCount);

    for (uint32_t i = 0u; i < IdCount; ++i)
    {
      _freeIds.push_back(IdCount - i - 1u);
    }
  }

  _INTR_INLINE static Ref allocate()
  {
    _INTR_ASSERT(!_freeIds.empty() && "Resource pool exhausted");
    uint32_t id = _freeIds.back();
    _freeIds.pop_back();

    Ref ref;
    ref._id = id;
    ref._generation = _generations[id];

    _activeRefs.push_back(ref);

    return ref;
  }

  _INTR_INLINE static void release(Ref p_Ref)
  {
    _INTR_ASSERT(p_Ref.isValid() && isAlive(p_Ref));

    // Erase an swap
    for (uint32_t i = 0; i < _activeRefs.size(); ++i)
    {
      if (_activeRefs[i] == p_Ref)
      {
        _activeRefs[i] = _activeRefs[_activeRefs.size() - 1u];
        _activeRefs.resize(_activeRefs.size() - 1u);
        break;
      }
    }

    _freeIds.push_back(p_Ref._id);

    const GenerationType currentGenId = _generations[p_Ref._id];
    _generations[p_Ref._id] = (currentGenId + 1u) % (maxGenerationIdValue + 1u);
  }

  static _INTR_ARRAY(IdType) _freeIds;
  static _INTR_ARRAY(GenerationType) _generations;
};

// <-

template <uint32_t IdCount, class DataType>
_INTR_ARRAY(IdType)
ManagerBase<IdCount, DataType>::_freeIds;
template <uint32_t IdCount, class DataType>
_INTR_ARRAY(Ref)
ManagerBase<IdCount, DataType>::_activeRefs;
template <uint32_t IdCount, class DataType>
_INTR_ARRAY(GenerationType)
ManagerBase<IdCount, DataType>::_generations;
}
}
}
