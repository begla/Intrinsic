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

namespace Intrinsic
{
namespace Core
{
namespace Resources
{
// Typedefs
typedef Dod::Ref EventRef;
typedef _INTR_ARRAY(EventRef) EventRefArray;

struct QueuedEventData
{
  union
  {
    struct
    {
      uint8_t key;
    } keyEvent;

    struct
    {
      float pos[2];
      float posViewport[2];
      float posRel[2];
    } mouseEvent;

    struct
    {
      uint8_t axis;
      float value;
    } axisEvent;
  };
};

struct EventData : Dod::Resources::ResourceDataBase
{
  EventData() : Dod::Resources::ResourceDataBase(_INTR_MAX_EVENT_COUNT)
  {
    queuedEventData.resize(_INTR_MAX_EVENT_COUNT);
  }

  // Resources
  _INTR_ARRAY(QueuedEventData) queuedEventData;
};

struct EventManager
    : Dod::Resources::ResourceManagerBase<EventData, _INTR_MAX_EVENT_COUNT>
{
  static void init()
  {
    _INTR_LOG_INFO("Inititializing Event Manager...");

    Dod::Resources::ResourceManagerBase<
        EventData, _INTR_MAX_EVENT_COUNT>::_initResourceManager();
  }

  // <-

  _INTR_INLINE static EventRef queueEventIfNotExisting(const Name& p_EventName)
  {
    for (uint32_t i = 0u; i < _activeRefs.size(); ++i)
    {
      if (_name(_activeRefs[i]) == p_EventName)
      {
        return Dod::Ref();
      }
    }

    return queueEvent(p_EventName);
  }

  _INTR_INLINE static EventRef
  queueEventIfNotExisting(const Name& p_EventName,
                          const QueuedEventData& p_EventData)
  {
    EventRef eventRef = queueEventIfNotExisting(p_EventName);
    _queuedEventData(eventRef) = p_EventData;
    return eventRef;
  }

  // <-

  _INTR_INLINE static EventRef queueEvent(const Name& p_EventName)
  {
    return Dod::Resources::ResourceManagerBase<
        EventData, _INTR_MAX_EVENT_COUNT>::_createResource(p_EventName);
  }

  // <-

  _INTR_INLINE static EventRef queueEvent(const Name& p_EventName,
                                          const QueuedEventData& p_EventData)
  {
    EventRef eventRef = queueEvent(p_EventName);
    _queuedEventData(eventRef) = p_EventData;
    return eventRef;
  }

  // <-

  _INTR_INLINE static void fireEvents()
  {
    // Fire events in order (oldest event gets fired first)
    for (int32_t eventIdx = 0u; eventIdx < (int32_t)_activeRefs.size();
         ++eventIdx)
    {
      EventRef eventRef = _activeRefs[eventIdx];

      EventListenerRefArray& eventListeners =
          EventListenerManager::_eventListenerMapping[_name(eventRef)];

      for (uint32_t eventListIdx = 0u; eventListIdx < eventListeners.size();
           ++eventListIdx)
      {
        EventListenerRef eventListenerRef = eventListeners[eventListIdx];

        if (EventListenerManager::_name(eventListenerRef) == _name(eventRef))
        {
          EventListenerManager::_descEventCallbackFunction(eventListenerRef)(
              eventRef);
        }
      }
    }

    for (int32_t eventIdx = (uint32_t)_activeRefs.size() - 1u; eventIdx >= 0;
         --eventIdx)
    {
      Dod::Resources::ResourceManagerBase<EventData, _INTR_MAX_EVENT_COUNT>::
          _destroyResource(_activeRefs[eventIdx]);
    }
  }

  // <-

  _INTR_INLINE static EventListenerRef
  connect(const Name& p_EventName, EventCallbackFunction p_CallbackFunction)
  {
    EventListenerRef eventListener =
        EventListenerManager::createEventListener(p_EventName);
    EventListenerManager::_descEventCallbackFunction(eventListener) =
        p_CallbackFunction;
    return eventListener;
  }

  // <-

  _INTR_INLINE static void disconnect(EventListenerRef p_EventListener)
  {
    EventListenerManager::destroyEventListener(p_EventListener);
  }

  // Resources
  _INTR_INLINE static QueuedEventData& _queuedEventData(EventRef p_EventRef)
  {
    return _data.queuedEventData[p_EventRef._id];
  }
};
}
}
}
