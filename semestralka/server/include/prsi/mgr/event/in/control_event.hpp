#pragma once

#include <variant>

namespace prsi::mgr {

struct Pong_Evin {};

using Control_Evin = std::variant<Pong_Evin>;

} // namespace prsi::mgr
