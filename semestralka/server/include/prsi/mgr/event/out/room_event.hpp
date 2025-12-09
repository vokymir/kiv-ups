#pragma once

#include <variant>

namespace prsi::mgr {

struct Players_Evout {};

using Room_Evout = std::variant<Players_Evout>;

} // namespace prsi::mgr
