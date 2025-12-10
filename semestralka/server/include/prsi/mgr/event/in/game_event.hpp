#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Card_Evin : public I_Evin {};

using Game_Evin = std::variant<Card_Evin>;

} // namespace prsi::mgr
