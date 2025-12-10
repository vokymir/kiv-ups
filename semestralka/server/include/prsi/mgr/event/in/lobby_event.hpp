#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Join_Room_Evin : public I_Evin {};

using Lobby_Evin = std::variant<Join_Room_Evin>;

} // namespace prsi::mgr
