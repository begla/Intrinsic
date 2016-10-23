// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

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
