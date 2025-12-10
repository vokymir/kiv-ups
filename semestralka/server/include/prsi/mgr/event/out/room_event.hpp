#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Players_Evout : I_Evout {};

using Room_Evout = std::variant<Players_Evout>;

} // namespace prsi::mgr
