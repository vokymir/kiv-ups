#pragma once

#include <variant>

namespace prsi::mgr {

struct Other_Card_Evout {};

using Game_Evout = std::variant<Other_Card_Evout>;

} // namespace prsi::mgr
