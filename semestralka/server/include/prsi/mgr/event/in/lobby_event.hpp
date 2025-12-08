#pragma once

#include <variant>

namespace prsi::mgr {

struct Join_Room_Event {};

using Lobby_Event = std::variant<Join_Room_Event>;

} // namespace prsi::mgr
