#pragma once

#include "prsi/mgr/event/out/control_event.hpp"
#include "prsi/mgr/event/out/game_event.hpp"
#include "prsi/mgr/event/out/lobby_event.hpp"
#include "prsi/mgr/event/out/room_event.hpp"
#include <variant>

namespace prsi::mgr {

using Out_Event =
    std::variant<Control_Evout, Lobby_Evout, Room_Evout, Game_Evout>;

}
