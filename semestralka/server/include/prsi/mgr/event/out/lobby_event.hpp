#pragma once

#include <variant>

namespace prsi::mgr {

struct Rooms_Evout {};

using Lobby_Evout = std::variant<Rooms_Evout>;

} // namespace prsi::mgr
