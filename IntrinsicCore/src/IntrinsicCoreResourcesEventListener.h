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
namespace Resources
{
typedef Dod::Ref EventListenerRef;
typedef _INTR_ARRAY(EventListenerRef) EventListenerRefArray;

typedef std::function<void(Dod::Ref)> EventCallbackFunction;

struct EventListenerData : Dod::Resources::ResourceDataBase
{
  EventListenerData()
      : Dod::Resources::ResourceDataBase(_INTR_MAX_EVENT_LISTENER_COUNT)
  {
    descEventCallbackFunction.resize(_INTR_MAX_EVENT_LISTENER_COUNT);
  }

  // <-

  _INTR_ARRAY(EventCallbackFunction) descEventCallbackFunction;
};

struct EventListenerManager
    : Dod::Resources::ResourceManagerBase<EventListenerData,
                                          _INTR_MAX_EVENT_LISTENER_COUNT>
{
  static void init()
  {
    _INTR_LOG_INFO("Inititializing Event Listener Manager...");

    Dod::Resources::ResourceManagerBase<
        EventListenerData,
        _INTR_MAX_EVENT_LISTENER_COUNT>::_initResourceManager();
  }

  // <-

  _INTR_INLINE static EventListenerRef createEventListener(const Name& p_Name)
  {
    EventListenerRef ref = Dod::Resources::ResourceManagerBase<
        EventListenerData,
        _INTR_MAX_EVENT_LISTENER_COUNT>::_createResource(p_Name);

    _eventListenerMapping[p_Name].push_back(ref);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(EventListenerRef p_Ref) {}

  // <-

  _INTR_INLINE static void destroyEventListener(EventListenerRef p_Ref)
  {
    EventListenerRefArray& eventListeners = _eventListenerMapping[_name(p_Ref)];

    for (auto it = eventListeners.begin(); it != eventListeners.end();)
    {
      if (*it == p_Ref)
      {
        it = eventListeners.erase(it);
      }
      {
        ++it;
      }
    }

    Dod::Resources::ResourceManagerBase<
        EventListenerData,
        _INTR_MAX_EVENT_LISTENER_COUNT>::_destroyResource(p_Ref);
  }

  // Getter/Setter
  // Intrinsic

  _INTR_INLINE static EventCallbackFunction&
  _descEventCallbackFunction(EventListenerRef p_Ref)
  {
    return _data.descEventCallbackFunction[p_Ref._id];
  }

  // Intrinsic

  static _INTR_HASH_MAP(Name, EventListenerRefArray) _eventListenerMapping;
};
}
}
}
