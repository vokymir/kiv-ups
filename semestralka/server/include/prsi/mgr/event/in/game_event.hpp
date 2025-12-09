#pragma once

#include <variant>

namespace prsi::mgr {

struct Card_Evin {};

using Game_Evin = std::variant<Card_Evin>;

} // namespace prsi::mgr
