#pragma once

#include "prsi/mgr/event/in/control_event.hpp"
#include "prsi/mgr/event/in/game_event.hpp"
#include "prsi/mgr/event/in/lobby_event.hpp"
#include "prsi/mgr/event/in/room_event.hpp"
#include <variant>

namespace prsi::mgr {

using In_Event = std::variant<Control_Evin, Lobby_Evin, Room_Evin, Game_Evin>;

}
