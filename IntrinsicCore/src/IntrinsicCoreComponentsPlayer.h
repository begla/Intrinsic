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
namespace Components
{
typedef Dod::Ref PlayerRef;
typedef _INTR_ARRAY(PlayerRef) PlayerRefArray;

struct PlayerData : Dod::Components::ComponentDataBase
{
  PlayerData()
      : Dod::Components::ComponentDataBase(_INTR_MAX_PLAYER_COMPONENT_COUNT)
  {
    descPlayerId.resize(_INTR_MAX_PLAYER_COMPONENT_COUNT);
  }

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
                                             rapidjson::Value& p_Properties,
                                             rapidjson::Document& p_Document)
  {
    p_Properties.AddMember(
        "playerId", _INTR_CREATE_PROP(p_Document, _N(Player), _N(Enum),
                                      _descPlayerId(p_Ref), false, false),
        p_Document.GetAllocator());
  }

  // <-

  _INTR_INLINE static void initFromDescriptor(PlayerRef p_Ref,
                                              rapidjson::Value& p_Properties)
  {
    if (p_Properties.HasMember("playerId"))
      _descPlayerId(p_Ref) =
          JsonHelper::readPropertyUint(p_Properties["playerId"]);
  }

  // <-

  // Getter/Setter
  // Intrinsic

  // Description
  _INTR_INLINE static uint32_t& _descPlayerId(PlayerRef p_Ref)
  {
    return _data.descPlayerId[p_Ref._id];
  }
};
}
}
}
