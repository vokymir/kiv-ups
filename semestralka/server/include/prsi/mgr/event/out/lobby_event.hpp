#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Rooms_Evout : public I_Evout {};

using Lobby_Evout = std::variant<Rooms_Evout>;

} // namespace prsi::mgr
