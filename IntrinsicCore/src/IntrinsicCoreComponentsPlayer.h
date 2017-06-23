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
namespace Components
{
// Typedefs
typedef Dod::Ref PlayerRef;
typedef _INTR_ARRAY(PlayerRef) PlayerRefArray;

struct PlayerData : Dod::Components::ComponentDataBase
{
  PlayerData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_PLAYER_COMPONENT_COUNT)
  {
    descPlayerId.resize(_INTR_MAX_PLAYER_COMPONENT_COUNT);
  }

  // Description
  _INTR_ARRAY(uint32_t) descPlayerId;
};

struct PlayerManager
    : Dod::Components::ComponentManagerBase<PlayerData,
                                            _INTR_MAX_PLAYER_COMPONENT_COUNT>
{
  static void init();

  // <-

  _INTR_INLINE static PlayerRef createPlayer(Entity::EntityRef p_ParentEntity)
  {
    PlayerRef ref = Dod::Components::ComponentManagerBase<
        PlayerData,
        _INTR_MAX_PLAYER_COMPONENT_COUNT>::_createComponent(p_ParentEntity);
    return ref;
  }

  // <-

  _INTR_INLINE static void resetToDefault(MeshRef p_Ref)
  {
    _descPlayerId(p_Ref) = 0u;
  }

  // <-

  _INTR_INLINE static void destroyPlayer(PlayerRef p_Player)
  {
    Dod::Components::ComponentManagerBase<
        PlayerData,
        _INTR_MAX_PLAYER_COMPONENT_COUNT>::_destroyComponent(p_Player);
  }

  // <-

  _INTR_INLINE static void compileDescriptor(PlayerRef p_Ref,
                                             bool p_GenerateDesc,
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "playerId",
        _INTR_CREATE_PROP(p_Document, p_GenerateDesc, _N(Player), _N(uint),
                          _descPlayerId(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(PlayerRef p_Ref,
                                              bool p_GenerateDesc,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("playerId"))
      _descPlayerId(p_Ref) =
          JsonHelper::readPropertyUint(p_Properties["playerId"]);
  }

  // Description
  _INTR_INLINE static uint32_t& _descPlayerId(PlayerRef p_Ref)
  {
    return _data.descPlayerId[p_Ref._id];
  }
};
}
}
}
