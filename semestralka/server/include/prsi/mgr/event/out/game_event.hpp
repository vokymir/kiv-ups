#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Other_Card_Evout : public I_Event {};

using Game_Evout = std::variant<Other_Card_Evout>;

} // namespace prsi::mgr
