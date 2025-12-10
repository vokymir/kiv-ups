#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Pong_Evin : public I_Evin {};

using Control_Evin = std::variant<Pong_Evin>;

} // namespace prsi::mgr
