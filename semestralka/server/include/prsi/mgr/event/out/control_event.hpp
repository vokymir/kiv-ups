#pragma once

#include <variant>

namespace prsi::mgr {

struct Ping_Evout {};

using Control_Evout = std::variant<Ping_Evout>;

} // namespace prsi::mgr
